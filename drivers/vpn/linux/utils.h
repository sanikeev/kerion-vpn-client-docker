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
#ifndef __UTILS_H__
#define __UTILS_H__

/**
 *	IP_ADDRESS_STRING
 */
#define IP_ADDRESS_STRING(x) (((u_int8_t*)&x)[0])&255,(((u_int8_t*)&x)[1])&255,(((u_int8_t*)&x)[2])&255,(((u_int8_t*)&x)[3])&255

/**
 *	DbgBrk
 */
#if DBG
#define DbgBrk() asm volatile("int $0x3;")
#else
#define DbgBrk()
#endif	//	DBG

/**
 *	DbgPrintStack
 */
#if DBG
#define DbgPrintStack() dump_stack()
#else
#define DbgPrintStack()
#endif	//	DBG

/**
 *	Log flags
 */
#define LOG_DBG		0x00000002
#define LOG_INFO	0x00000004
#define LOG_WARN	0x00000008
#define LOG_ERR		0x00000016
#define LOG_RAW		0x10000000

/*
 *	performance optimization, do not evaluate arguments if event disabled 
 */
#define TRACE(flag, format, ...)										\
	if (!(flag & LOG_DBG) || IS_DEBUG_FLAG_SET())		\
		trace(flag, format, __VA_ARGS__);


/**
 *	IS_DEBUG_FLAG_SET
 */
#define IS_DEBUG_FLAG_SET() (g_utils_debug_enabled)

/**
 *	IS_PTR_VALID
 */
#ifndef IS_PTR_VALID
#define IS_PTR_VALID(x)	((x != NULL) ? (1) : (0))
#endif

/**
 *	ARRAY_CNT
 */
#ifndef ARRAY_CNT
#define ARRAY_CNT(a) (sizeof(a)/sizeof((a)[0]))
#endif

/**
 *	MIN
 */
#ifndef MIN
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#endif

/**
 *	MAX
 */
#ifndef MAX
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif

/**
 *	K_LOCK
 */
typedef struct {
	spinlock_t spinlock;
} K_LOCK, *PK_LOCK;

/**
 *	OSVER
 */
typedef struct {	
	int major_version;
	int minor_version;
	int build_number;		
	int sp_major_version;
	int sp_minor_version;
	int free_build;
	int x86;
} OSVER, *POSVER;

/**
 *	alloc_lock
 */
int
alloc_lock(
	K_LOCK *lock
	);

/**
 *	free_lock
 */
void
free_lock(
	K_LOCK *lock
	);

/**
 *	acquire_lock
 */
void
acquire_lock(
	K_LOCK *lock
	);

/**
 *	release_lock
 */
void
release_lock(
	K_LOCK *lock
	);

/**
 *	alloc_paged_mem
 */
void*
alloc_paged_mem(
	int size
	);

/**
 *	alloc_nonpaged_mem
 */
void*
alloc_nonpaged_mem(
	int size
	);

/**
 *	free_paged_mem
 */
void
free_paged_mem(
	void *buf,
	int size
	);

/**
 *	free_nonpaged_mem
 */
void
free_nonpaged_mem(
	void *buf,
	int size
	);

/**
 *	get_os_ver
 */
void
get_os_ver(
	OSVER *ver
	);

/**
 *	trace
 */
void
trace(
	int flag,
	char *fmt,
	...
	);


/**
 *	dump_data
 */
void
dump_data(
	unsigned char *data,
	int len
	);

/**
 *	dump_eth_frame
 */
void
dump_eth_frame(
	unsigned char *data,
	int len
	);

/**
 *	dump_ip_packet
 */
void
dump_ip_packet(
	unsigned char *data,
	int len
	);

/**
 *	adjust_checksum
 */
void
adjust_checksum(
	unsigned short *sum,
	void *odata,
	void *ndata, 
	int len
	);

/**
 *	checksum
 */
unsigned short
checksum(
	unsigned char *buff,
	unsigned int len
	);

/**
 *	dump_skb
 */
void
dump_skb(
	struct sk_buff *skb
	);

//
//	g_utils_debug_enabled - used in trace function
//
extern int g_utils_debug_enabled;

#endif // __UTILS_H__
