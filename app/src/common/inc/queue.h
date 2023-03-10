/**
  @file src/queue.h

  Implements a simple LIFO structure used for queueing OMX buffers.

  Copyright (C) 2007-2008 STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

  $Date$
  Revision $Rev$
  Author $Author$
*/

#ifndef __TQUEUE_H__
#define __TQUEUE_H__

#include <pthread.h>
/** Maximum number of elements in a queue
 */
#define MAX_QUEUE_ELEMENTS 10
/** Output port queue element. Contains an OMX buffer header type
 */
typedef struct qelem_t qelem_t;
struct qelem_t{
  qelem_t* q_forw;
  void* data;
};

/** This structure contains the queue
 */
typedef struct queue_t{
  qelem_t* first; /**< Output buffer queue head */
  qelem_t* last; /**< Output buffer queue tail */
  unsigned int nelem; /**< Number of elements in the queue */
  pthread_mutex_t mutex;
} queue_t;

/** Initialize a queue descriptor
 *
 * @param queue The queue descriptor to initialize.
 * The user needs to allocate the queue
 */
void queue_init(queue_t* queue);

/** Deinitialize a queue descriptor
 * flushing all of its internal data
 *
 * @param queue the queue descriptor to dump
 */
void queue_deinit(queue_t* queue);

/** Enqueue an element to the given queue descriptor
 *
 * @param queue the queue descritpor where to queue data
 *
 * @param data the data to be enqueued
 */
void queue(queue_t* queue, void* data);

/** Dequeue an element from the given queue descriptor
 *
 * @param queue the queue descriptor from which to dequeue the element
 *
 * @return the element that has bee dequeued. If the queue is empty
 *  a NULL value is returned
 */
void* dequeue(queue_t* queue);

/** Returns the number of elements hold in the queue
 *
 * @param queue the requested queue
 *
 * @return the number of elements in the queue
 */
int getquenelem(queue_t* queue);

#endif
