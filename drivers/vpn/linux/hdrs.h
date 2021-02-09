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
#ifndef __HDRS_H__
#define __HDRS_H__

#include <linux/types.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/random.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/miscdevice.h>
#include <linux/rtnetlink.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/version.h>
#include <net/ip.h>

//
//	Linux kernel prior version 2.6.22 is not supported
//
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
#error "--- Linux kernel prior version 2.6.22 is not supported ---"
#endif

//
//	Linux kernel 2.6.22 - 2.6.26
//
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26))
#define LINUX_KERNEL_2_6_22_TILL_2_6_26
#endif

//
//	Linux kernel 2.6.27
//
#if (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,27))
#define LINUX_KERNEL_2_6_27
#endif

#define MODULE_NAME "kvnet"

//#define USE_BLOCKING_READ

#include "utils.h"
#include "lin_specific.h"
#include "init.h"
#include "io_dev.h"
#include "net_dev.h"
#include "queue.h"
#include "vnet.h"
#include "defs.h"
#include "ioctl.h"

#endif	//	__HDRS_H__
