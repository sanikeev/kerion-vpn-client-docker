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
#ifndef __IO_DEV_H__
#define __IO_DEV_H__

/**
 *	io_dev_wakeup
 */
void
io_dev_wakeup(
	void
	);

/**
 *	io_dev_add
 */
int
io_dev_add(
	void
	);

/**
 *	io_dev_remove
 */
int
io_dev_remove(
	void
	);

#endif // __IO_DEV_H__
