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
 *	queue_init
 */
#undef __FUNC__
#define __FUNC__ "queue_init"
int
queue_init(
	struct queue *q
	)
{
	//
	//	Allocate lock
	//
	alloc_lock(&q->lock);

	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
	q->enabled = 0;

	return 0;
}

/**
 *	queue_close
 */
#undef __FUNC__
#define __FUNC__ "queue_close"
int
queue_close(
	struct queue *q
	)
{
	//
	//	Some packets still in the queue ?
	//
	if (IS_PTR_VALID(q->head)) {
		TRACE(LOG_WARN, "%s: %d packet(s) still left in the queue.", __FUNC__, q->size);
	}

	//
	//	Free lock
	//
	free_lock(&q->lock);		

	return 0;
}

/**
 *	__is_queue_enabled
 */
int
__is_queue_enabled(
	struct queue *q	
	)
{
	return q->enabled;
}

/**
 *	is_queue_enabled
 */
int
is_queue_enabled(
	struct queue *q	
	)
{
	int enabled = 0;

	acquire_lock(&q->lock);
	enabled = q->enabled;
	release_lock(&q->lock);

	return enabled;
}

/**
 *	__queue_enable
 */
void
__queue_enable(
	struct queue *q,
	int enable
	)
{	
	q->enabled = enable;
}

/**
 *	queue_enable
 */
void
queue_enable(
	struct queue *q,
	int enable
	)
{
	acquire_lock(&q->lock);		
	q->enabled = enable;
	release_lock(&q->lock);
}

/**
 *	__queue_is_empty
 */
int
__queue_is_empty(
	struct queue *q
	)
{	
	return ((q->size == 0) ? (1) : (0));
}

/**
 *	queue_is_empty
 */
int
queue_is_empty(
	struct queue *q
	)
{
	int empty = 0;

	acquire_lock(&q->lock);		
	empty = ((q->size == 0) ? (1) : (0));
	release_lock(&q->lock);		

	return empty;
}

/**
 *	__queue_len
 */
int
__queue_len(
	struct queue *q
	)
{
	return q->size;
}

/**
 *	queue_len
 */
int
queue_len(
	struct queue *q
	)
{
	int len = 0;

	acquire_lock(&q->lock);		
	len = q->size;
	release_lock(&q->lock);		

	return len;
}

/**
 *	__queue_insert
 */
#undef __FUNC__
#define __FUNC__ "__queue_insert"
void
__queue_insert(
	struct queue *q,
	void *buf
	)
{	
	SET_NEXT_PKT(buf, NULL);

	if (q->head == NULL) {

		q->head = q->tail = buf;

	} else {

		SET_NEXT_PKT(q->tail, buf);

		q->tail = buf;
	}

	q->size++;	
}

/**
 *	queue_insert
 */
#undef __FUNC__
#define __FUNC__ "queue_insert"
void
queue_insert(
	struct queue *q,
	void *buf
	)
{	
	acquire_lock(&q->lock);			

	SET_NEXT_PKT(buf, NULL);

	if (q->head == NULL) {

		q->head = q->tail = buf;

	} else {

		SET_NEXT_PKT(q->tail, buf);

		q->tail = buf;
	}

	q->size++;

	release_lock(&q->lock);
}

/**
 *	__queue_remove
 */
#undef __FUNC__
#define __FUNC__ "__queue_remove"
void*
__queue_remove(
	struct queue *q
	)
{
	void *buf = NULL;
	
	//
	//	check whether there is a packet in the queue
	//
	if (IS_PTR_VALID(q->head)) {
					
		//
		//	fetch it
		//
		buf = q->head;
		q->head = GET_NEXT_PKT(q->head);
		SET_NEXT_PKT(buf, NULL);

		q->size--;
	}		

	return buf;
}

/**
 *	queue_remove
 */
#undef __FUNC__
#define __FUNC__ "queue_remove"
void*
queue_remove(
	struct queue *q
	)
{
	void *buf = NULL;

	acquire_lock(&q->lock);		
	
	//
	//	check whether there is a packet in the queue
	//
	if (IS_PTR_VALID(q->head)) {
					
		//
		//	fetch it
		//
		buf = q->head;
		q->head = GET_NEXT_PKT(q->head);
		SET_NEXT_PKT(buf, NULL);

		q->size--;
	}	

	release_lock(&q->lock);		

	return buf;
}

