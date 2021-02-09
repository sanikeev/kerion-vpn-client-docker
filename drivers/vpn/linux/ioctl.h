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
#ifndef __IOCTL_H__
#define __IOCTL_H__

/* Kerio virtual netfork interface IO device */
#define KVNET_DEVICE						"/dev/kvnet"

/*
 *	ioctl's commands
 */
#define KVNET_IOCTL_CANCEL_PENDING_READ	_IO('O', 1)
#define KVNET_IOCTL_GET_INFO			_IOR('O', 2, struct kvnet_info)

#endif // __IOCTL_H__
