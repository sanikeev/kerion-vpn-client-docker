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
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <linux/if_ether.h>

#include <kvnet/api.h>

#define MAX_PACKETS 64

//
//	MAX_DATA_SIZE
//
#define MAX_DATA_SIZE	8190

//
//	MAX_PACKET_SIZE
//
#define MAX_PACKET_SIZE	(sizeof(struct kvnet_packet) + MAX_DATA_SIZE)

bool can_run_loop = false;

/** 
 *	pktLoop
 */
#undef __FUNC__
#define __FUNC__ "pktLoop"
static void* pktLoop(void* params)
{
	struct kvnet_packet *pkt[MAX_PACKETS];	
	struct kvnet_header *hdr = NULL;
	kvnet_status status = KVNET_NO_ERROR;	
	struct kvnet_info info;
	unsigned int i = 0;
	unsigned int len = 0;

	//
	//	get driver info
	//
	status = kvnet_get_driver_info(&info);
			
	if (!KVNET_SUCCEEDED(status)) {
		printf("%s(): kvnet_get_driver_info(): status=%d, sysError=%d\n", __FUNC__, KVNET_STATUS(status), KVNET_SYS_ERROR(status));	
		return 0;
	}	

	printf("%s(): API version=%d.%d\n", __FUNC__, info.api_version.major, info.api_version.minor);

	//
	//	allocate memory for header
	//
	len = sizeof(struct kvnet_header) + ((MAX_PACKETS - 1) * sizeof(KVNET_PTR));

	if (!(hdr = (struct kvnet_header*)malloc(len))) {
		return 0;
	}

	//
	//	allocate memory for every packet
	//
	for (i = 0; i < MAX_PACKETS; i++) {
		if (!(pkt[i] = (struct kvnet_packet*) malloc(MAX_PACKET_SIZE))) {					
			break;	
		}					
	}		
		
	if (i != MAX_PACKETS) {
		goto cleanup;
	}		

	//
	//	run until driver is able to run loop 
	//
	while (can_run_loop) {

		//
		//	renew max length of data
		//
		for (i = 0; i < MAX_PACKETS; i++) {
			pkt[i]->data_len = MAX_DATA_SIZE;			
		}

		//
		//	renew pointers to packets
		//
		for (i = 0; i < MAX_PACKETS; i++) {
			hdr->pkt[i] = (KVNET_PTR)pkt[i];
		}

		//
		//	renew max count of packets
		//
		hdr->count = MAX_PACKETS;

		//
		//	read packets from vnet
		//
		status = kvnet_read(hdr, len);

		if (!KVNET_SUCCEEDED(status)) {
			printf("%s(): kvnet_read(): status=%d, sysError=%d\n", __FUNC__, KVNET_STATUS(status), KVNET_SYS_ERROR(status));	
			break;
		}		

		printf("%d packet(s) read.\n", hdr->count);

		for (i = 0; i < hdr->count; i++) {
			printf("%d. packet has %d byte(s).\n", i+1, pkt[i]->data_len);
		}
	}	

cleanup:

	for (i = 0; i < MAX_PACKETS; i++) {
		if (pkt[i]) {			
			free(pkt[i]);			
		}		
	}

	if (hdr) {
		free(hdr);
	}

	return 0;
}

/**
 *	cancel_loop
 */
#undef __FUNC__
#define __FUNC__ "cancel_loop"
static void cancel_loop(void)
{	
	if (!can_run_loop) {
		return;
	}

	can_run_loop = false;

	//
	//	cancel any pending read if any
	//
	kvnet_status status = kvnet_cancel_read();

	if (!KVNET_SUCCEEDED(status)) {
		printf("%s(): kvnet_cancel_read(): status=%d, sysError=%d\n", __FUNC__, KVNET_STATUS(status), KVNET_SYS_ERROR(status));	
	} else {
		printf("%s(): pending read canceled.\n", __FUNC__);	
	}
}

/** 
 *	signal_handler
 */
#undef __FUNC__
#define __FUNC__ "signal_handler"
static void signal_handler(int signum)
{		
	cancel_loop();	
	printf("%s(): Press 'q' for exit.\n", __FUNC__);
}

/** 
 *	main
 */
#undef __FUNC__
#define __FUNC__ "main"
int main(int argc, char *argv[] ) 
{	
	bool use_tap = false;
		
	if (argc > 1) {
		if (strcmp(argv[1], "--usetap") == 0) {
			use_tap = true;
		}
	}

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP, signal_handler);

	kvnet_status status = KVNET_NO_ERROR;	

	printf("%s(): using %s\n", __FUNC__, use_tap ? "TAP" : "KVNET");	

	//
	//	open kvnet 1 - use TAP, 0 - don't use TAP
	//
	status = kvnet_open(use_tap ? 1 : 0);
			
	if (!KVNET_SUCCEEDED(status)) {
		printf("%s(): kvnet_open(): status=%d, sysError=%d\n", __FUNC__, KVNET_STATUS(status), KVNET_SYS_ERROR(status));	
		return 0;
	}		

	printf("%s(): kvnet opened.\n", __FUNC__);

	can_run_loop = true;

	//
	//	create second thread for packet loop
	//
	pthread_t thread = 0;

	if (pthread_create(&thread, NULL, pktLoop, NULL) != 0) {	
		printf("%s(): pthread_create(): %d\n", __FUNC__, errno);
		kvnet_close();
		return 0;
	}				

	//
	//	wait for 'q' which signal quit
	//
	while (1) {
		if (getc(stdin) == 'q') {
			printf("%s(): quiting.\n", __FUNC__);	
			cancel_loop();
			break;
		}
	}
		
	//
	//	wait for packet loop end
	//
	pthread_join(thread, NULL);

	//
	//	close kvnet
	//
	kvnet_close();

	printf("%s(): kvnet closed.\n", __FUNC__);
	
	return 0;			
}
