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
 *	io_read_fill_network_packet
 */
#undef __FUNC__
#define __FUNC__ "io_read_fill_network_packet()"
static int
io_read_fill_network_packet(	
	struct kvnet_packet* pkt,	// user space address
	struct sk_buff *skb
	)
{
	u_int32_t len = 0;
	u_int32_t data_len = 0;
	struct vnet *vnet = vnet_dev();

	if (get_user(data_len, (u_int32_t __user *) &pkt->data_len)) {
		return -EFAULT;
	}

	//
	//	copy skb buffer to user space buffer
	//
	len = min_t(u_int32_t, skb->len, data_len);
	
	if (copy_to_user(pkt->data, skb->data, len)) {
		return -EFAULT;
	}

	vnet->net_dev.stats.tx_packets++;
	vnet->net_dev.stats.tx_bytes += len;

	if (put_user(len, &pkt->data_len)) {
		return -EFAULT;
	}

	return 0;
}

/**
 *	io_read_to_iovec
 */
#undef __FUNC__
#define __FUNC__ "io_read_to_iovec()"
ssize_t
io_read_to_iovec(				
	struct iovec *iv,
	ssize_t len
	)
{	
#if defined (USE_BLOCKING_READ)
	DECLARE_WAITQUEUE(wait, current);
#endif
	struct kvnet_header *hdr = NULL;
	struct sk_buff *skb = NULL;	
	ssize_t ret = 0;	
	u_int32_t i = 0;			
	struct vnet *vnet = vnet_dev();

	if (len < sizeof(struct kvnet_header)) {
		TRACE(LOG_WARN, "%s: invalid len. (%d)", __FUNC__, len);
		return -EINVAL;
	}

#if defined (USE_BLOCKING_READ)
	add_wait_queue(&vnet->io_dev.os_specific.wait_queue, &wait);
	set_current_state(TASK_INTERRUPTIBLE);
#endif

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

	//
	//	fetch a new mbuf from the queue
	//	
	for (i = 0; i < hdr->count; i++) {

#if defined (USE_BLOCKING_READ)
try_again:			
#endif

		skb = queue_remove(&vnet->net_dev.queue);

		if (!IS_PTR_VALID(skb)) {			
		
			if (ret == 0) {

#if defined (USE_BLOCKING_READ)

				TRACE(LOG_DBG, "%s: schedule(): going to sleep...", __FUNC__);
			
				//
				//	Nothing to read, let's sleep
				//				
				schedule();				

				//
				//	After you slept you should check if a signal occurred: the Unix/Linux way of handling signals
				//	is to temporarily exit the system call with the -ERESTARTSYS error. The system call entry code
				//	will switch back to user context, process the signal handler and then your system call will be
				//	restarted.
				//
				//	http://mail.nl.linux.org/kernelnewbies/2003-02/msg00305.html
				//
				if (signal_pending(current)) {
					TRACE(LOG_WARN, "%s: some system signal has occured.", __FUNC__);
					ret = -ERESTARTSYS;
					//ret = -EINTR;
					break;
				}

				goto try_again;

#else

				TRACE(LOG_DBG, "%s: no buffer in the queue ???", __FUNC__);
				ret = -EAGAIN;

#endif // USE_BLOCKING_READ

			}

			break;
		}						

		if (io_read_fill_network_packet((struct kvnet_packet*)hdr->pkt[i], skb)) {
			TRACE(LOG_WARN, "%s: io_read_fill_network_packet(): segmentation fault.", __FUNC__);
			kfree_skb(skb);	
			ret = -EFAULT;
			break;
		}			

		ret++;

		kfree_skb(skb);	

	} // for (i = 0; i < hdr->count; i++) {

	netif_wake_queue(vnet->net_dev.os_specific.device);

#if defined (USE_BLOCKING_READ)
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&vnet->io_dev.os_specific.wait_queue, &wait);	
#endif

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

