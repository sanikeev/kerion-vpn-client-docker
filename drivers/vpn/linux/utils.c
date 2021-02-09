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

/* arphdr_eth */
struct arphdr_eth
{
	 /*
	  *	 Ethernet looks like this : This bit is variable sized however...
	  */
	unsigned char		ar_sha[ETH_ALEN];	/* sender hardware address	*/
	unsigned char		ar_sip[4];		/* sender IP address		*/
	unsigned char		ar_tha[ETH_ALEN];	/* target hardware address	*/
	unsigned char		ar_tip[4];		/* target IP address		*/
};

/**
 *	alloc_lock
 */
inline int
alloc_lock(
	K_LOCK *lock
	)
{
	if (!IS_PTR_VALID(lock)) {
		DbgBrk();
		return -EINVAL;
	}

	spin_lock_init(&lock->spinlock);
	return 0;
}

/**
 *	free_lock
 */
inline void
free_lock(
	K_LOCK *lock
	)
{
	if (!IS_PTR_VALID(lock)) {
		DbgBrk();
		return;
	}		
}

/**
 *	acquire_lock
 */
inline void
acquire_lock(
	K_LOCK *lock
	)
{
	if (!IS_PTR_VALID(lock)) {
		DbgBrk();
		return;
	}

	spin_lock_bh(&lock->spinlock);
}

/**
 *	release_lock
 */
inline void
release_lock(
	K_LOCK *lock
	)
{
	if (!IS_PTR_VALID(lock)) {
		DbgBrk();
		return;
	}

	spin_unlock_bh(&lock->spinlock);
}

/**
 *	alloc_paged_mem
 */
inline void*
alloc_paged_mem(
	int size
	)
{
	void *ptr = NULL;	

	ptr = kmalloc(size, GFP_KERNEL);

	return ptr;
}

/**
 *	alloc_nonpaged_mem
 */
inline void*
alloc_nonpaged_mem(
	int size
	)
{
	void *ptr = NULL;

	ptr = kmalloc(size, GFP_KERNEL);

	return ptr;
}

/**
 *	free_paged_mem
 */
inline void
free_paged_mem(
	void *buf,
	int size
	)
{
	if (!IS_PTR_VALID(buf)) {
		DbgBrk();
		return;
	}		

	kfree(buf);
}

/**
 *	free_nonpaged_mem
 */
inline void
free_nonpaged_mem(
	void *buf,
	int size
	)
{
	if (!IS_PTR_VALID(buf)) {
		DbgBrk();
		return;
	}
		
	kfree(buf);
}

/**
 *	trace
 */
void
trace(
	int flag,
	char *fmt,
	...
	)
{
    va_list arg;    
	char dbgbuf1[256];
    char dbgbuf2[256];

    va_start(arg, fmt);
	
	vsnprintf(
		dbgbuf1,
		sizeof(dbgbuf1) - 1,
		fmt,
		arg
		);      			

	va_end(arg);	

	if (flag & LOG_RAW) {
		printk(KERN_INFO "%s", dbgbuf1);
		return;
	}

	sprintf(
		dbgbuf2,
		"%s!%s\n",
		MODULE_NAME,
		dbgbuf1
		);    
	
	printk(KERN_INFO "%s", dbgbuf2);
}

/**
 *	dump_skb
 */
#undef __FUNC__
#define __FUNC__ "dump_skb"
void
dump_skb(
	struct sk_buff *skb
	)
{
	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	/*

		TRACE(LOG_INFO, "%s: head skb=%p, head_len=%d, len=%d, data_len=%d, frag_list=%p, nr_frags=%d, data=%p, next=%p, prev=%p, nonlinear=%d",
			__FUNC__, (*pskb), skb_headlen(*pskb), (*pskb)->len, (*pskb)->data_len,	skb_shinfo(*pskb)->frag_list, skb_shinfo(*pskb)->nr_frags,
			(*pskb)->data, (*pskb)->next, (*pskb)->prev, skb_is_nonlinear(*pskb));	

		dump_ip_packet((u_int8_t*) ip_hdr(frag_skb), frag_skb->len);

		frag_skb = skb_shinfo(*pskb)->frag_list;

		while (frag_skb) {

			TRACE(LOG_INFO, "%s: frag skb=%p, head_len=%d, len=%d, data_len=%d, frag_list=%p, nr_frags=%d, data=%p, next=%p, prev=%p, nonlinear=%d",
				__FUNC__, frag_skb, skb_headlen(frag_skb), frag_skb->len, frag_skb->data_len,	skb_shinfo(frag_skb)->frag_list, skb_shinfo(frag_skb)->nr_frags,
				frag_skb->data, frag_skb->next, frag_skb->prev, skb_is_nonlinear(frag_skb));

			dump_ip_packet((u_int8_t*) ip_hdr(frag_skb), frag_skb->len);

			frag_skb = frag_skb->next;
		}	
#if 1
			dump_eth_frame(
				mbuf_data(pkt),
				mbuf_len(pkt)
				);				
#else
			dump_data(
				mbuf_data(pkt),
				mbuf_len(pkt)
				);
#endif
	*/

	TRACE(LOG_INFO, "%s: <==", __FUNC__);
}

/**
 *	dump_data
 */
#undef __FUNC__
#define __FUNC__ "dump_data"
void
dump_data(
	unsigned char *data,
	int len
	)
{
	int i = 0;

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);	
	TRACE((LOG_INFO | LOG_RAW), "data=0x%p, len=%d byte(s)\n", data, len);

	if (IS_PTR_VALID(data)) {

		TRACE((LOG_INFO | LOG_RAW), "%s", "data=");

		while (len--) {
			TRACE((LOG_INFO | LOG_RAW), "%02X ", data[i++]);
		}

		TRACE((LOG_INFO | LOG_RAW), "%s", "\n");
	}

	TRACE(LOG_INFO, "%s: <==", __FUNC__);
}

/**
 *	dump_eth_frame
 */
#undef __FUNC__
#define __FUNC__ "dump_eth_frame"
void
dump_eth_frame(
	unsigned char *data,
	int len
	)
{
	struct ethhdr *eh = NULL;

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	do {	

		if (!IS_PTR_VALID(data)) {
			TRACE(LOG_WARN, "%s: invalid pointer passed.", __FUNC__);
			break;
		}

		if (len < sizeof(struct ethhdr)) {
			TRACE(LOG_WARN, "%s: can't print ethernet header: invalid length.", __FUNC__);
			break;
		}

		eh = (struct ethhdr*) data;			

		//
		//	ETHERNET
		//
		TRACE((LOG_INFO | LOG_RAW), "ETH: SRC=%02x:%02x:%02x:%02x:%02x:%02x DST=%02x:%02x:%02x:%02x:%02x:%02x TYPE=%04X\n",
			eh->h_source[0],
			eh->h_source[1],
			eh->h_source[2],
			eh->h_source[3],
			eh->h_source[4],
			eh->h_source[5],
			eh->h_dest[0],
			eh->h_dest[1],
			eh->h_dest[2],
			eh->h_dest[3],
			eh->h_dest[4],
			eh->h_dest[5],
			ntohs(eh->h_proto)
			);
		
		//
		//	ARP
		//
		if (ntohs(eh->h_proto) == 0x0806) {
			struct arphdr *ea = (struct arphdr*) ((unsigned char*)eh + sizeof(struct ethhdr));
			struct arphdr_eth *ea_eth = (struct arphdr_eth*) ((unsigned char*)ea + sizeof(struct arphdr));
			unsigned int *ip = NULL;

			if (len < sizeof(struct ethhdr) + sizeof(struct arphdr)) {
				TRACE(LOG_WARN, "%s: can't print ARP header: invalid length.", __FUNC__);
				break;
			}

			TRACE((LOG_INFO | LOG_RAW), "ARP: HRD=%u PRO=0x%04X HLN=%u PLN=%u OP=%u\n",
				ntohs(ea->ar_hrd),
				ntohs(ea->ar_pro),
				ea->ar_hln,
				ea->ar_pln,
				ntohs(ea->ar_op)
				);

			ip = (unsigned int*) &ea_eth->ar_sip;

			TRACE((LOG_INFO | LOG_RAW), "ARP: SHA=%02x:%02x:%02x:%02x:%02x:%02x SIP=%u.%u.%u.%u\n",
				ea_eth->ar_sha[0],
				ea_eth->ar_sha[1],
				ea_eth->ar_sha[2],
				ea_eth->ar_sha[3],
				ea_eth->ar_sha[4],
				ea_eth->ar_sha[5],
				IP_ADDRESS_STRING(*ip)
				);

			ip = (unsigned int*) &ea_eth->ar_tip;

			TRACE((LOG_INFO | LOG_RAW), "ARP: THA=%02x:%02x:%02x:%02x:%02x:%02x TIP=%u.%u.%u.%u\n",
				ea_eth->ar_tha[0],
				ea_eth->ar_tha[1],
				ea_eth->ar_tha[2],
				ea_eth->ar_tha[3],
				ea_eth->ar_tha[4],
				ea_eth->ar_tha[5],
				IP_ADDRESS_STRING(*ip)
				);

			break;
		}

		//
		//	IP
		//
		if (ntohs(eh->h_proto) == 0x0800) {
			struct iphdr *ip = (struct iphdr*) ((unsigned char*) eh + sizeof(struct ethhdr));
			unsigned short offset = 0;
			unsigned short flags = 0;

			if (len < sizeof(struct ethhdr) + sizeof(struct iphdr)) {
				TRACE(LOG_WARN, "%s: can't print IP header: invalid length.", __FUNC__);
				break;
			}

			offset = ntohs(ip->frag_off);
			flags = offset & ~IP_OFFSET;
			offset &= IP_OFFSET;
			offset <<= 3;

			TRACE((LOG_INFO | LOG_RAW), "IP: SRC=%u.%u.%u.%u DST=%u.%u.%u.%u PROTO=%u LEN=%u FLAGS=0x%04X OFFSET=%u\n",				
				IP_ADDRESS_STRING(ip->saddr),
				IP_ADDRESS_STRING(ip->daddr),
				ip->protocol,
				ntohs(ip->tot_len),
				flags,
				offset
				);

			break;
		}

	} while(0);

	TRACE(LOG_INFO, "%s: <==", __FUNC__);
}

/**
 *	dump_ip_packet
 */
#undef __FUNC__
#define __FUNC__ "dump_ip_packet"
void
dump_ip_packet(
	unsigned char *data,
	int len
	)
{
	unsigned short offset = 0;
	unsigned short flags = 0;
	struct iphdr *ip = NULL;

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	do {	

		if (!IS_PTR_VALID(data)) {
			TRACE(LOG_WARN, "%s: invalid pointer passed.", __FUNC__);
			break;
		}

		if (len < sizeof(struct iphdr)) {
			TRACE(LOG_WARN, "%s: can't print IP header: invalid length.", __FUNC__);
			break;
		}

		ip = (struct iphdr*) data;

		offset = ntohs(ip->frag_off);
		flags = offset & ~IP_OFFSET;
		offset &= IP_OFFSET;
		offset <<= 3;

		TRACE((LOG_INFO | LOG_RAW), "IP: SRC=%u.%u.%u.%u DST=%u.%u.%u.%u PROTO=%u LEN=%u FLAGS=0x%04X OFFSET=%u\n",				
			IP_ADDRESS_STRING(ip->saddr),
			IP_ADDRESS_STRING(ip->daddr),
			ip->protocol,
			ntohs(ip->tot_len),
			flags,
			offset
			);		

	} while(0);

	TRACE(LOG_INFO, "%s: <==", __FUNC__);
}

/**
 *	adjust_checksum
 */
#undef __FUNC__
#define __FUNC__ "adjust_checksum"
void
adjust_checksum(
	unsigned short *sum,
	void *odata,
	void *ndata, 
	int len
	)
{
	int x = 0;
	int i = 0;
	unsigned char *optr = (unsigned char *) odata;
	unsigned char *nptr = (unsigned char *) ndata;

	x = (unsigned short) ~ntohs(*sum);

	for (i = 0; i < len; i += 2) {

		x -= ((optr[0] << 8) + optr[1]);

		if(x <= 0) {
			x--;
			x &= 0xffff;
		}

		optr+=2;
	}

	for (i = 0; i < len; i += 2) {

		x += ((nptr[0] << 8) + nptr[1]);

		if(x & 0x10000) {
			x++;
			x &= 0xffff;
		}

		nptr+=2;
	}

	*sum = htons((unsigned short)~x);
}

/**
 *	checksum
 */
#undef __FUNC__
#define __FUNC__ "checksum"
unsigned short
checksum(
	unsigned char *buff,
	unsigned int len
	)
{
	unsigned int sum = 0;
	unsigned int nbytes = 0;
	unsigned short answer = 0;
	unsigned short *ptr = NULL;
	unsigned short oddbyte = 0;

	ptr = (unsigned short *) buff;
	nbytes = len;
	sum = 0;

	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}

	if (nbytes == 1) {
		oddbyte = 0;
		*((unsigned char *) &oddbyte) = *(unsigned char *)ptr;
		sum += oddbyte;
	}

	sum  = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = (unsigned short) ~sum;

	return answer;
}
