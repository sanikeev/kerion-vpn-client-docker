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
#ifndef __LIN_SPECIFIC_H__
#define __LIN_SPECIFIC_H__

#pragma pack(1)

//
//	os_specific_io_dev_data
//
struct os_specific_io_dev_data {		

	wait_queue_head_t wait_queue;

	int major_number;

	int minor_number;

	struct cdev cdev;

	struct class *class;

	struct device *device;
};

//
//	os_specific_net_dev_data
//
struct os_specific_net_dev_data {
	struct net_device *device;
};

//
//	SET_NEXT_PKT
//
#define SET_NEXT_PKT(pkt, nextpkt) ((struct sk_buff*)pkt)->next = nextpkt

//
//	GET_NEXT_PKT
//
#define GET_NEXT_PKT(pkt) ((struct sk_buff*)pkt)->next

#endif // __LIN_SPECIFIC_H__
