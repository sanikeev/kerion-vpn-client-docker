/*
 * Kerio virtual network interface
 * Copyright (C) Kerio Technologies s.r.o.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hdrs.h"

/**
 *	io_write_from_iovec
 */
#undef __FUNC__
#define __FUNC__ "io_write_to_iovec()"
ssize_t
io_write_from_iovec(				
	struct iovec *iv,
	ssize_t len
	)
{	
	struct sk_buff *skb = NULL;		
	struct kvnet_header *hdr = NULL;
	struct kvnet_packet *pkt = NULL;		
	int i = 0;
	u_int32_t data_len = 0;
	ssize_t ret = 0;
	struct vnet *vnet = vnet_dev();

	if (len < sizeof(struct kvnet_header)) {
		TRACE(LOG_WARN, "%s: invalid len. (%d)", __FUNC__, len);
		return -EINVAL;
	}

	hdr = alloc_paged_mem(len);

	if (!IS_PTR_VALID(hdr)) {
		TRACE(LOG_WARN, "%s: alloc_paged_mem(): no memory to allocate %d byte(s).", __FUNC__, len);
		return -ENOMEM;
	}				

	//
	//	memcpy_fromiovecend do not modifies the original iovec
	//
	if (memcpy_fromiovecend((unsigned char*) hdr, iv, 0, len)) {
		TRACE(LOG_WARN, "%s: memcpy_fromiovecend(): segmentation fault.", __FUNC__);
		free_paged_mem(hdr, len);
		return -EFAULT;
	}

	if (!hdr->count) {
		TRACE(LOG_WARN, "%s: invalid count of packets. (%d)", __FUNC__, hdr->count);
		free_paged_mem(hdr, len);
		return -EINVAL;		
	}

	if (len < sizeof(struct kvnet_header) + ((hdr->count - 1/* one pointer is already included in the header*/) * sizeof(KVNET_PTR))) {
		TRACE(LOG_WARN, "%s: invalid length of header. (%d)", __FUNC__, len);
		free_paged_mem(hdr, len);
		return -EINVAL;		
	}

	for (i = 0; i < hdr->count; i++) {

		pkt = (struct kvnet_packet*)hdr->pkt[i];	// user space buffer

		if (get_user(data_len, (u_int32_t __user *) &pkt->data_len)) {
			TRACE(LOG_WARN, "%s: get_user(): segmentation fault.", __FUNC__);
			ret = -EFAULT;
			break;
		}

		skb = dev_alloc_skb(data_len + NET_IP_ALIGN);

		if (!IS_PTR_VALID(skb)) {
			TRACE(LOG_WARN, "%s: dev_alloc_skb(): no memory to allocate %d byte(s).", __FUNC__, data_len + NET_IP_ALIGN);
			ret = -ENOMEM;					
			break;
		}
		
		skb_reserve(skb, NET_IP_ALIGN);		

		if (copy_from_user(skb_put(skb, data_len), pkt->data, data_len)) {
			TRACE(LOG_WARN, "%s: copy_from_user(): segmentation fault.", __FUNC__);
			ret = -EFAULT;
			kfree_skb(skb);
			break;
		}

		skb->protocol = eth_type_trans(skb, vnet->net_dev.os_specific.device);
		skb->mac_len = sizeof(struct ethhdr);
		skb->dev = vnet->net_dev.os_specific.device;

		//skb->ip_summed = CHECKSUM_UNNECESSARY;

		//
		//	Receiving packets in non-interrupt mode:
		//	should use netif_rx_ni (it will call preempt_disable > netif_rx -> preempt_enable
		//
		netif_rx_ni(skb);

		vnet->net_dev.os_specific.device->last_rx = jiffies;

		vnet->net_dev.stats.rx_packets++;
		vnet->net_dev.stats.rx_bytes += data_len;

		ret++;

	}	// for (i = 0; i < hdr->count; i++) {
	
	if (ret >= 0) {

		hdr->count = ret;	

		if (memcpy_toiovec(iv, (unsigned char*) hdr, sizeof(hdr->count))) {
			TRACE(LOG_WARN, "%s: memcpy_toiovec(): segmentation fault.", __FUNC__);
			ret = -EFAULT;
		} else {
			ret = sizeof(struct kvnet_header);
			ret += ((hdr->count) ? ((hdr->count - 1) * sizeof(KVNET_PTR)) : (hdr->count));
		}
	}

	free_paged_mem(hdr, len);

	return ret;
}

