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

//
//	NO_ARP
//
//#define NO_ARP

/* Bug 49123 - Packets bigger than 1400 B do not pass through vpn for some customers with DSL modems */
#define KVNET_MAX_MTU	1400	//1446

/**
 *	net_dev_flush_tx_queue
 */
#undef __FUNC__
#define __FUNC__ "net_dev_flush_tx_queue()"
void
net_dev_flush_tx_queue(
	void
	)
{
	int flushed = 0;
	struct sk_buff *skb = NULL;
	struct vnet *vnet = vnet_dev();

	acquire_lock(&vnet->net_dev.queue.lock);

	while ((skb = __queue_remove(&vnet->net_dev.queue))) {		
		kfree_skb(skb);
		flushed++;
	}

	release_lock(&vnet->net_dev.queue.lock);

	vnet->net_dev.stats.tx_dropped += flushed;

	TRACE(LOG_INFO, "%s: flushed=%d", __FUNC__, flushed);
}

/**
 *	net_dev_open
 */
#undef __FUNC__
#define __FUNC__ "net_dev_open()"
static int 
net_dev_open(
	struct net_device *dev
	)
{			
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	//
	//	enable packet queueing
	//
	queue_enable(&vnet->net_dev.queue, 1);

	netif_start_queue(dev);

	vnet->net_dev.connected = 1;

	TRACE(LOG_INFO, "%s: net device \"kvnet\" opened.", __FUNC__);

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return 0;
}

/**
 *	net_dev_close
 */
#undef __FUNC__
#define __FUNC__ "net_dev_close()"
static int 
net_dev_close(
	struct net_device *dev
	)
{			
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	//
	//	disable packet queueing
	//
	queue_enable(&vnet->net_dev.queue, 0);

	net_dev_flush_tx_queue();

	netif_stop_queue(dev);

	vnet->net_dev.connected = 0;

	TRACE(LOG_INFO, "%s: net device \"kvnet\" closed.", __FUNC__);

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return 0;
}

/**
 *	net_dev_send
 */
#undef __FUNC__
#define __FUNC__ "net_dev_send()"
static int 
net_dev_send(
	struct sk_buff *skb,
	struct net_device *dev
	)
{		
	struct vnet *vnet = vnet_dev();

	if (IS_DEBUG_FLAG_SET()) {

		TRACE(LOG_DBG, "%s: skb=%p, len=%d, head_len=%d, dev='%s', frag_list=%p, nr_frags=%d, ip_summed=%d, mac=%s, "
			"mac_hdr=%p, mac_len=%d, data_len=%d, ip=%p, head=%p, data=%p, tail=%p, end=%p, frag_list=%p, next=%p, prev=%p",
			__FUNC__, skb, skb->len, skb_headlen(skb), ((skb->dev) ? (skb->dev->name) : ("???")), skb_shinfo(skb)->frag_list,
			skb_shinfo(skb)->nr_frags, skb->ip_summed, ((skb_mac_header_was_set(skb)) ? ("yes") : ("no")), skb_mac_header(skb),
			skb->mac_len, skb->data_len, ip_hdr(skb), skb->head, skb->data, skb->tail, skb->end, skb_shinfo(skb)->frag_list,
			skb->next, skb->prev);

		dump_ip_packet((u_int8_t*) ip_hdr(skb), skb_headlen(skb));	
	}

	acquire_lock(&vnet->net_dev.queue.lock);

	//	
	//	Packet dropping
	//
	if (!__is_queue_enabled(&vnet->net_dev.queue) || __queue_len(&vnet->net_dev.queue) >= dev->tx_queue_len) {			

		vnet->net_dev.stats.tx_dropped++;

		kfree_skb(skb);

		release_lock(&vnet->net_dev.queue.lock);

		return 0;
	}	
	
	__queue_insert(&vnet->net_dev.queue, skb);				

	release_lock(&vnet->net_dev.queue.lock);	

	dev->trans_start = jiffies;

	io_dev_wakeup();

	return 0;
}

/**
 *	net_dev_ioctl
 */
#undef __FUNC__
#define __FUNC__ "net_dev_ioctl()"
static int 
net_dev_ioctl(
	struct net_device *dev,
	struct ifreq *ifr,
	int cmd
	)
{	
	TRACE(LOG_DBG, "%s: cmd=0x%08X", __FUNC__, cmd);
	return -ENOSYS;
}

/**
 *	net_dev_get_stats
 */
#undef __FUNC__
#define __FUNC__ "net_dev_get_stats()"
static struct net_device_stats *
net_dev_get_stats(
	struct net_device *dev
	)
{	
	struct net_device_stats *stats = netdev_priv(dev);	
	struct vnet *vnet = vnet_dev();

	stats->rx_packets		=	vnet->net_dev.stats.rx_packets;	
	stats->tx_packets		=	vnet->net_dev.stats.tx_packets;	
	stats->rx_errors		=	vnet->net_dev.stats.rx_errors;	
	stats->tx_errors		=	vnet->net_dev.stats.tx_errors;	
	stats->rx_dropped		=	vnet->net_dev.stats.rx_dropped;	
	stats->tx_dropped		=	vnet->net_dev.stats.tx_dropped;	
	stats->rx_bytes			=	vnet->net_dev.stats.rx_bytes;		
	stats->tx_bytes			=	vnet->net_dev.stats.tx_bytes;		
	stats->rx_fifo_errors	=	vnet->net_dev.stats.rx_fifo_errors;
	stats->tx_fifo_errors	=	vnet->net_dev.stats.tx_fifo_errors;
	
	TRACE(LOG_DBG, "%s: rx_packets=%d, tx_packets=%d, rx_errors=%d, tx_errors=%d, rx_dropped=%d, tx_dropped=%d, "
		"rx_bytes=%d, tx_bytes=%d, rx_fifo_errors=%d, tx_fifo_errors=%d", __FUNC__, stats->rx_packets,
		stats->tx_packets, stats->rx_errors, stats->tx_errors, stats->rx_dropped, stats->tx_dropped, stats->rx_bytes, 
		stats->tx_bytes, stats->rx_fifo_errors, stats->tx_fifo_errors);

	return stats;
}

/**
 *	always_on
 */
#undef __FUNC__
#define __FUNC__ "always_on()"
static u_int32_t 
always_on(
	struct net_device *dev
	)
{
	TRACE(LOG_INFO, "%s: called. ", __FUNC__);
	return 1;
}

static const struct ethtool_ops net_dev_ethtool_ops = {
	.get_link		= always_on,
};

/**
 *	net_dev_init
 */
#undef __FUNC__
#define __FUNC__ "net_dev_init()"
static int32_t
net_dev_init(
	struct net_device *dev
	)
{
	//
	//	here we can allocate our storage 
	//	lstats = alloc_percpu(struct pcpu_lstats);
	//	dev->priv = lstats;
	//
	TRACE(LOG_INFO, "%s: called. ", __FUNC__);
	return 0;
}

/** 
 *      net_dev_uninit 
 */ 
#undef __FUNC__ 
#define __FUNC__ "net_dev_uninit()" 
static void 
net_dev_uninit( 
        struct net_device *dev 
        ) 
{ 
        TRACE(LOG_INFO, "%s: called. ", __FUNC__); 
} 

/**
 *	net_dev_free
 */
#undef __FUNC__
#define __FUNC__ "net_dev_free()"
static void 
net_dev_free(
	struct net_device *dev
	)
{
	//free_percpu(lstats);
	//free_netdev(dev);
	TRACE(LOG_INFO, "%s: called. ", __FUNC__);
}

static const struct net_device_ops kvnet_netdev_ops = { 
        .ndo_init               = net_dev_init, 
        .ndo_uninit             = net_dev_uninit, 
        .ndo_open               = net_dev_open, 
        .ndo_stop               = net_dev_close, 
        .ndo_start_xmit         = net_dev_send, 
        .ndo_do_ioctl           = net_dev_ioctl, 
        .ndo_get_stats          = net_dev_get_stats, 
}; 

/**
 *	net_dev_setup
 */
#undef __FUNC__
#define __FUNC__ "net_dev_setup()"
static void 
net_dev_setup(
	struct net_device *dev
	)
{	
	struct net_device_stats *stats = NULL;
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	ether_setup(dev);

#if 0
	dev->open			= net_dev_open;
	dev->stop			= net_dev_close;
	dev->init			= net_dev_init;	
	dev->hard_start_xmit		= net_dev_send;	
	dev->do_ioctl			= net_dev_ioctl;
	dev->get_stats			= net_dev_get_stats;	
#else
	dev->netdev_ops = &kvnet_netdev_ops;
#endif
	dev->destructor			= net_dev_free;
	dev->hard_header_len		= ETH_HLEN;	/* 14	*/
	dev->addr_len			= ETH_ALEN;	/* 6	*/	
	dev->mtu			= KVNET_MAX_MTU;
	dev->tx_queue_len		= vnet->cfg.max_queue_len;
	dev->ethtool_ops		= &net_dev_ethtool_ops;
	dev->flags			&= ~IFF_MULTICAST;			
#if defined (NO_ARP)
	dev->flags			|= IFF_NOARP;	
#endif

	// Set MAC address - since 2.6.32 is pointer not a array! see /net/core/dev.c!dev_addr_init	
	memcpy(dev->dev_addr, vnet->cfg.mac, dev->addr_len);
		
	//
	//	we can use our private area
	//
	stats = netdev_priv(dev);				

	memset(stats, 0, sizeof(struct net_device_stats));

	TRACE(LOG_INFO, "%s: <==", __FUNC__);
}


/**
 *	net_dev_remove
 */
#undef __FUNC__
#define __FUNC__ "net_dev_remove"
int
net_dev_remove(
	void
	)
{
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	if (IS_PTR_VALID(vnet->net_dev.os_specific.device)) {		

		if (vnet->net_dev.added) {
			unregister_netdev(vnet->net_dev.os_specific.device);
		}

		free_netdev(vnet->net_dev.os_specific.device);

		vnet->net_dev.os_specific.device = NULL;

		vnet->net_dev.added = 0;

		TRACE(LOG_INFO, "%s: net device \"kvnet\" removed.", __FUNC__);
	}

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return 0;
}

/**
 *	net_dev_add
 */
#undef __FUNC__
#define __FUNC__ "net_dev_add()"
int
net_dev_add(
	void
	)
{	
	int ret = 0;
	struct vnet *vnet = vnet_dev();

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	do {		

		vnet->net_dev.os_specific.device = alloc_netdev(sizeof(struct net_device_stats), "kvnet", net_dev_setup);

		if (!IS_PTR_VALID(vnet->net_dev.os_specific.device)) {
			ret = -ENOMEM;
			TRACE(LOG_WARN, "%s: alloc_netdev(): %d", __FUNC__, ret);		
			break;
		}			

		//
		//	register_netdev calls rtl_locks for us
		//	register_netdevice not - must be locked otherwise ASSERT_RTNL(); occured
		//
		if ((ret = register_netdev(vnet->net_dev.os_specific.device))) {		
			TRACE(LOG_WARN, "%s: register_netdev(): %d", __FUNC__, ret);		
			break;
		}		

		vnet->net_dev.added = 1;

		TRACE(LOG_INFO, "%s: net device \"kvnet\" added.", __FUNC__);

	} while(0);

	//
	//	On error perform rollback action
	//
	if (ret) {
		net_dev_remove();
	}

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

	return ret;
}

