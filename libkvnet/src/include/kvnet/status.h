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
#ifndef __KVNET_STATUS_H__
#define __KVNET_STATUS_H__

typedef unsigned int kvnet_status;

/* Main macro for creating status code */
#define	KVNET_MAKE_STATUS(status)				((unsigned int)(((unsigned short)((unsigned int)(kvnet_get_sys_error()) & 0xffff)) \
	| ((unsigned int)((unsigned short)((unsigned int)(status) & 0xffff))) << 16))

/* Macro for obtaining system error if any */
#define KVNET_SYS_ERROR(l)						((unsigned short)((unsigned int)(l) & 0xffff))

/* Macro for obtaining kipf error if any */
#define KVNET_STATUS(l)							((unsigned short)((unsigned int)(l) >> 16))

/* Provides a generic test for success on any status value. */
#define KVNET_SUCCEEDED(status)					(KVNET_STATUS(status) == KVNET_NO_ERROR)

/* No error */
#define KVNET_NO_ERROR							0x0000

/* Can't create read event */
#define KVNET_READ_EVENT_ERROR					0x0001

/* Already opened */
#define KVNET_ERROR_ALREADY_OPENED				0x0002

/* Can't open device */
#define KVNET_OPEN_DEVICE_ERROR					0x0003

/* Device is not opened */
#define KVNET_NOT_OPENED							0x0004

/* The parameter is incorrect */
#define KVNET_INVALID_PARAMETER					0x0005

/* Error while reading from device */
#define KVNET_READ_ERROR							0x0006

/* Error read was cancelled */
#define KVNET_READ_CANCELLED						0x0007

/* Error in completing pending read */
#define KVNET_PENDING_READ_ERROR					0x0008

/* Error while writting to device */
#define KVNET_WRITE_ERROR						0x0009

/* Error while performing a IOCTL	*/
#define KVNET_IOCTL_ERROR						0x000a

/* Call not implemented	*/
#define KVNET_NOT_IMPLEMENTED					0x000b

/* Only 32 bit application is supported	*/
#define KVNET_NO_32BIT_APP						0x000c

/* select function failed	*/
#define KVNET_SELECT_ERROR						0x000d

/* no memory */
#define KVNET_NO_MEMORY						0x000e

#endif // __KVNET_STATUS_H__

