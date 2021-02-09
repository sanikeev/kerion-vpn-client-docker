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

/**
 *	main vnet data structure
 */
struct vnet *gs_vnet = NULL;

/**
 *	vnet_init
 */
#undef __FUNC__
#define __FUNC__ "vnet_init()"
int32_t
vnet_init(
	void
	)
{
	if (IS_PTR_VALID(gs_vnet)) {
		TRACE(LOG_ERR, "%s: vnet object already exists.", __FUNC__);
		return -1;
	}

	//
	//	allocate main vnet object
	//
	gs_vnet = alloc_nonpaged_mem(sizeof(struct vnet));

	if (!IS_PTR_VALID(gs_vnet)) {
		TRACE(LOG_ERR, "%s: can't allocate %d byte(s) of memory for vnet object.", __FUNC__, sizeof(struct vnet));
		return -1;
	}

	memset(gs_vnet, 0, sizeof(struct vnet));

	queue_init(&gs_vnet->net_dev.queue);
	
	TRACE(LOG_INFO, "%s: vnet object created. (%p)", __FUNC__, gs_vnet);

	return 0;
}

/**
 *	vnet_close
 */
#undef __FUNC__
#define __FUNC__ "vnet_close()"
int32_t
vnet_close(
	void
	)
{
	if (IS_PTR_VALID(gs_vnet)) {		

		queue_close(&gs_vnet->net_dev.queue);

		free_nonpaged_mem(gs_vnet, sizeof(struct vnet));
		gs_vnet = NULL;		

		TRACE(LOG_INFO, "%s: vnet object destroyed.", __FUNC__);
	}

	return 0;
}

/**
 *	vnet_dev
 */
#undef __FUNC__
#define __FUNC__ "vnet_dev()"
struct vnet*
vnet_dev(
	void
	)
{
	return gs_vnet;
}
