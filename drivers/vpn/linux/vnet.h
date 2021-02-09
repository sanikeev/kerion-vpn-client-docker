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
#ifndef __VNET_H__
#define __VNET_H__

#pragma pack(1)

//
//	main vnet object
//
struct vnet {	

	//
	//	io_dev - vnet I/O device
	//
	struct {

		//
		//		- 0 removed
		//		- 1 added
		//
		int32_t added;

		//
		//		- 0 closed
		//		- 1 opened
		//
		int32_t opened;

		//
		//		- 0 ready
		//		- 1 canceled
		//
		int32_t canceled;		

		//
		//	place for any OS specific I/O data
		//
		struct os_specific_io_dev_data os_specific;

	} io_dev;

	//
	//	net_dev
	//
	struct {

		//
		//		- 0 removed
		//		- 1 added
		//
		int32_t added;

		//
		//		- 0 disconnected
		//		- 1 connected
		//
		int32_t connected;

		//
		//	statistics for vnet device
		//
		struct {

			u_int32_t rx_packets;			// total packets received
			u_int32_t tx_packets;			// total packets transmitted	

			u_int32_t rx_errors;			// bad packets received		
			u_int32_t tx_errors;			// packet transmit problems	

			u_int32_t rx_dropped;			// no space in linux buffers	
			u_int32_t tx_dropped;			// no space available in linux	

			u_int32_t rx_bytes;				// total bytes received 	
			u_int32_t tx_bytes;				// total bytes transmitted	
	
			u_int32_t rx_fifo_errors;		// recv'r fifo overrun		
			u_int32_t tx_fifo_errors;		// send'r fifo overrun		

		} stats;

		//
		//	packet queue
		//
		struct queue queue;

		//
		//	place for any OS specific I/O data
		//
		struct os_specific_net_dev_data os_specific;
			
	} net_dev;

	//
	//	configuration values for our vnet
	//
	struct {
		u_int8_t mac[ETH_ALEN];
		int32_t max_queue_len;
	} cfg;
};

#pragma pack()

/**
 *	vnet_init
 */
int32_t
vnet_init(
	void
	);

/**
 *	vnet_close
 */
int32_t
vnet_close(
	void
	);

/**
 *	vnet_dev
 */
struct vnet*
vnet_dev(
	void
	);

#endif // __VNET_H__
