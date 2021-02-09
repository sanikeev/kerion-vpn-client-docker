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
 *	g_utils_debug_enabled - used in trace function
 */
int g_utils_debug_enabled = 0;

/**
 *	module_stop
 */
#undef __FUNC__
#define __FUNC__ "module_stop"
void
module_stop(
	void
	)
{		
	TRACE(LOG_INFO, "%s: ==>", __FUNC__);

	do {

		//
		//	Remove IO device
		//
		io_dev_remove();				

		//
		//	destroy main vnet object
		//
		vnet_close();

	} while(0);

	TRACE(LOG_INFO, "%s: <==", __FUNC__);    
}

/**
 *	module_start
 */
#undef __FUNC__
#define __FUNC__ "module_start"
int
module_start(
	void
	)
{
	int ret = -EINVAL;

	TRACE(LOG_INFO, "%s: ==>", __FUNC__);	
	
	do {			
		struct vnet *vnet = NULL;

		//
		//	create main vnet object
		//
		if (vnet_init() != 0) {			
			break;
		}		

		vnet = vnet_dev();

		//
		//	load cfg ...
		//
		vnet->cfg.max_queue_len = 256;

		vnet->cfg.mac[0] = 0x44;
		vnet->cfg.mac[1] = 0x45;
		vnet->cfg.mac[2] = 0x53;
		vnet->cfg.mac[3] = 0x54;
		vnet->cfg.mac[4] = 0x4f;
		vnet->cfg.mac[5] = 0x53;

		//
		//	add IO device (it have to return 0 on success and -ENOMEM [or something else] on failure)
		//
		if ((ret = io_dev_add())) {		
			break;
		}

	} while(0);

	//
	//	On error perform rollback action
	//
	if (ret) {
		module_stop();
	}

	TRACE(LOG_INFO, "%s: <==", __FUNC__);

    return ret;	
}

module_init(module_start);
module_exit(module_stop);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Copyright (C) Kerio Technologies s.r.o.");
MODULE_DESCRIPTION("Kerio virtual net interface.");

