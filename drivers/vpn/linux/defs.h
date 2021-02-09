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
#ifndef __KVNET_DEFS_H__
#define __KVNET_DEFS_H__

/* Version of this driver */
#define	KVNET_MAJOR_VERSION			2
#define	KVNET_MINOR_VERSION			1

typedef	void* KVNET_PTR;

#pragma pack(1)

/*
 *	kvnet_info
 */
struct kvnet_info {    			
	/*
	 *	version of driver
	 */
	struct {
		unsigned int major;
		unsigned int minor;		
	} api_version;

	unsigned char hw_addr[ETH_ALEN];
}; 

/**
 *	kvnet_header
 */
struct kvnet_header {            
	unsigned int count;	//	count of pointers to packets in pkt
	KVNET_PTR pkt[1];	//	array of pointers to struct Packet
};

/**
 *	kvnet_packet
 */
struct kvnet_packet {            
    unsigned int data_len;	//	on input max accessible length of data, on output length of data in packet
    unsigned char data[1];	//	packet's data
}; 

#pragma pack()

#endif // __KVNET_DEFS_H__

