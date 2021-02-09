/*
 * Kerio virtual network interface library for userspace
 * Copyright (C) Kerio Technologies s.r.o.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <kvnet/api.h>

/* Bug 49123 - Packets bigger than 1400 B do not pass through vpn for some customers with DSL modems */
#define KVNET_MAX_MTU	1400	//1446

// only for TAP version
#define MAJOR_PRODUCT_VERSION       6
#define MINOR_PRODUCT_VERSION       7
#define REVISION_NUMBER             0
#define BUILD_NUMBER                666

#ifndef MIN
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#endif

/* Globals */
static int fdesc = -1;
static int canceled = 0;
static int use_tap_dev = 0;

/**
 *	kvnet_get_sys_error
 */
unsigned int kvnet_get_sys_error(void)
{
	return errno;
}

/*
 *	kvnet_open
 */
kvnet_status kvnet_open(int use_tap)
{
	//
	//	check if pointers is 4bytes long (32 bit application)
	//

	/*	Already opened ? */	
	if (fdesc != -1) {
		return KVNET_MAKE_STATUS(KVNET_ERROR_ALREADY_OPENED);
	}

	if (use_tap) {
	      	struct ifreq ifr;
			int skfd = -1;

      		if ((fdesc = open("/dev/net/tun", O_RDWR | O_CLOEXEC)) == -1) {
				return KVNET_MAKE_STATUS(KVNET_OPEN_DEVICE_ERROR);
			}

      		memset(&ifr, 0, sizeof(ifr));

	      	/* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
       		 *        IFF_TAP   - TAP device  
       		 *        IFF_NO_PI - Do not provide packet information  
	       	 */ 
			ifr.ifr_flags = IFF_TAP | IFF_NO_PI; 
			strncpy(ifr.ifr_name, "kvnet", IFNAMSIZ);

      		if (ioctl(fdesc, TUNSETIFF, (void *) &ifr) == -1) {
				close(fdesc);
				fdesc = -1;
				return KVNET_MAKE_STATUS(KVNET_IOCTL_ERROR);
      		}

			/* Bug 49123 - Packets bigger than 1400 B do not pass through vpn for some customers with DSL modems */			
			if ((skfd = socket(PF_INET, SOCK_DGRAM, 0)) != -1) {
				memset(&ifr, 0, sizeof(ifr));

				ifr.ifr_mtu = KVNET_MAX_MTU;
				strncpy(ifr.ifr_name, "kvnet", IFNAMSIZ);

      			ioctl(skfd, SIOCSIFMTU, (void *) &ifr);
				close(skfd);      							
			}

	} else {
		if ((fdesc = open(KVNET_DEVICE, O_RDWR | O_CLOEXEC | O_NONBLOCK)) == -1) {
			return KVNET_MAKE_STATUS(KVNET_OPEN_DEVICE_ERROR);
		}
	}

	canceled = 0;
	use_tap_dev = use_tap;

	return KVNET_MAKE_STATUS(KVNET_NO_ERROR);
}

/*
 *	kvnet_close
 */
kvnet_status kvnet_close(void)
{
	if (fdesc == -1) {
		return KVNET_MAKE_STATUS(KVNET_NOT_OPENED);
	}	

	close(fdesc);
	fdesc = -1;

	return KVNET_MAKE_STATUS(KVNET_NO_ERROR);
}

/*
 *	kvnet_cancel_read
 */
kvnet_status kvnet_cancel_read(void)
{
	if (use_tap_dev) {
		canceled = 1;	
	} else {
		int ret = 0;

		if (fdesc == -1) {
			return KVNET_MAKE_STATUS(KVNET_INVALID_PARAMETER);
		}

		if ((ret = ioctl(fdesc, KVNET_IOCTL_CANCEL_PENDING_READ, NULL)) == -1) {	
			return KVNET_MAKE_STATUS(KVNET_IOCTL_ERROR);
		}
	}

	return KVNET_MAKE_STATUS(KVNET_NO_ERROR);	
}

/*
 *	kvnet_read
 */
kvnet_status kvnet_read(struct kvnet_header *hdr, unsigned int len)
{
	int ret = 0;
	struct pollfd pfd;

	if (fdesc == -1 || !hdr || len < sizeof(struct kvnet_header)) {
		return KVNET_MAKE_STATUS(KVNET_INVALID_PARAMETER);
	}

try_again:

	pfd.fd = fdesc;
	pfd.events = POLLIN | POLLERR;
	pfd.revents = 0;

	ret = poll(&pfd, 1, (use_tap_dev) ? 1 : -1 /* infinite */);

	if (ret > 0) {

		if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
			return KVNET_MAKE_STATUS(KVNET_READ_CANCELLED);			
		}

		if (pfd.revents & POLLIN) {

			if (use_tap_dev) {
				struct kvnet_packet *pkt = (struct kvnet_packet*)hdr->pkt[0];

				if ((ret = read(fdesc, pkt->data, pkt->data_len)) == -1) {	
					return KVNET_MAKE_STATUS(KVNET_READ_ERROR);
				}

				pkt->data_len = ret;
				hdr->count = 1;
			} else {
				if ((ret = read(fdesc, hdr, len)) == -1) {	
					return KVNET_MAKE_STATUS(KVNET_READ_ERROR);
				}
			}

			return KVNET_MAKE_STATUS(KVNET_NO_ERROR);
		}

	} else if (ret == 0) {

		if (use_tap_dev) {
			if (canceled) {
				return KVNET_MAKE_STATUS(KVNET_READ_CANCELLED);						
			} else {
				goto try_again;
			}
		} else {
			/* timeout ? */
			return KVNET_MAKE_STATUS(KVNET_SELECT_ERROR);						
		}

 	} else if (ret == -1) {
		if (errno == EINTR) {			
			goto try_again;
		}
		return KVNET_MAKE_STATUS(KVNET_SELECT_ERROR);				
	}				

	return KVNET_MAKE_STATUS(KVNET_SELECT_ERROR);		
}

/*
 *	kvnet_write
 */
kvnet_status kvnet_write(struct kvnet_header *hdr, unsigned int len)
{
	int ret = 0;

	if (fdesc == -1 || !hdr || len < sizeof(struct kvnet_header)) {
		return KVNET_MAKE_STATUS(KVNET_INVALID_PARAMETER);
	}

	if (use_tap_dev) {
		int i = 0;
		for (i = 0; i < hdr->count; i++) {
			struct kvnet_packet *pkt = (struct kvnet_packet*)hdr->pkt[i];
			if ((ret = write(fdesc, pkt->data, pkt->data_len)) == -1) {
				return KVNET_MAKE_STATUS(KVNET_WRITE_ERROR);
			}
		}	
	} else {
		if ((ret = write(fdesc, hdr, len)) == -1) {		
			return KVNET_MAKE_STATUS(KVNET_WRITE_ERROR);
		}
	}

	return KVNET_MAKE_STATUS(KVNET_NO_ERROR);
}

/*
 *	kvnet_get_mac
 */
static void kvnet_get_mac(unsigned char* mac, int mac_len)
{
 	int sock = 0; 

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		return;
	}

 	struct ifreq ifr;	

	memset(&ifr, 0, sizeof(struct ifreq));

	strcpy(ifr.ifr_name, "kvnet");
	
	if (ioctl(sock, SIOCGIFHWADDR, &ifr ) == -1) {
		close(sock);
  		return;
	}

	memcpy(mac, ifr.ifr_hwaddr.sa_data, mac_len);

	close(sock);
}

/*
 *	kvnet_get_driver_info
 */
kvnet_status kvnet_get_driver_info(struct kvnet_info *info)
{
	if (use_tap_dev) {
		if (!info) {
			return KVNET_MAKE_STATUS(KVNET_INVALID_PARAMETER);
		}

		info->api_version.major = KVNET_MAJOR_VERSION;
		info->api_version.minor = KVNET_MINOR_VERSION;

		kvnet_get_mac(info->hw_addr, ETH_ALEN);
	} else {
		int ret = 0;

		if (fdesc == -1 || !info) {
			return KVNET_MAKE_STATUS(KVNET_INVALID_PARAMETER);
		}

		if ((ret = ioctl(fdesc, KVNET_IOCTL_GET_INFO, info)) == -1) {	
			return KVNET_MAKE_STATUS(KVNET_IOCTL_ERROR);
		}
	}

	return KVNET_MAKE_STATUS(KVNET_NO_ERROR);		
}

