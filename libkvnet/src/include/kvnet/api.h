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
#ifndef __KVNET_API_H__
#define __KVNET_API_H__

#include "status.h"

#include "../../../../drivers/vpn/linux/defs.h"
#include "../../../../drivers/vpn/linux/ioctl.h"

#ifdef __cplusplus
extern "C"{
#endif 

/*
 *	kvnet_open
 */
kvnet_status kvnet_open(int use_tap);

/*
 *	kvnet_close
 */
kvnet_status kvnet_close(void);

/*
 *	kvnet_cancel_read
 */
kvnet_status kvnet_cancel_read(void);

/*
 *	kvnet_read
 */
kvnet_status kvnet_read(struct kvnet_header *hdr, unsigned int len);

/*
 *	kvnet_write
 */
kvnet_status kvnet_write(struct kvnet_header *hdr, unsigned int len);

/*
 *	kvnet_get_driver_info
 */
kvnet_status kvnet_get_driver_info(struct kvnet_info *info);

#ifdef __cplusplus
}
#endif

#endif // __KVNET_API_H__
