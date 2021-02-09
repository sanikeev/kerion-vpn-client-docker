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
#ifndef __QUEUE_H__
#define __QUEUE_H__

//
//	queue
//
struct queue {

	//
	//	output end of the queue. dequeueing takes buffers from here
	//
	void *head;
	
	//
	//	input end. new buffers are appended here.
	//
	void *tail;

	//
	//	size of queue
	//
	int size;

	//
	//	lock for this queue
	//
	K_LOCK lock;

	//
	//	is queueing enabled ?
	//
	int enabled;
};

/**
 *	queue_init
 */
int
queue_init(
	struct queue *q
	);

/**
 *	queue_close
 */
int
queue_close(
	struct queue *q
	);

/**
 *	__is_queue_enabled
 */
int
__is_queue_enabled(
	struct queue *q	
	);

/**
 *	is_queue_enabled
 */
int
is_queue_enabled(
	struct queue *q	
	);

/**
 *	__queue_enable
 */
void
__queue_enable(
	struct queue *q,
	int enable
	);

/**
 *	queue_enable
 */
void
queue_enable(
	struct queue *q,
	int enable
	);

/**
 *	__queue_is_empty
 */
int
__queue_is_empty(
	struct queue *q
	);

/**
 *	queue_is_empty
 */
int
queue_is_empty(
	struct queue *q
	);

/**
 *	__queue_len
 */
int
__queue_len(
	struct queue *q
	);

/**
 *	queue_len
 */
int
queue_len(
	struct queue *q
	);

/**
 *	__queue_insert
 */
void
__queue_insert(
	struct queue *q,
	void *buf
	);

/**
 *	queue_insert
 */
void
queue_insert(
	struct queue *q,
	void *buf
	);

/**
 *	__queue_remove
 */
void*
__queue_remove(
	struct queue *q
	);

/**
 *	queue_remove
 */
void*
queue_remove(
	struct queue *q
	);

#endif // __QUEUE_H__
