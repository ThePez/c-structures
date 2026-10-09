/**
 * @file queue.h
 * @brief FIFO queue. (interface)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>

typedef struct Queue Queue;

/**
 * @brief Gets the number of elements currently in the queue.
 *
 * @param queue The queue to query.
 * @return The element count, or 0 if queue is NULL.
 */
size_t queue_size(const Queue *queue);

/**
 * @brief Adds a copy of an item to the back of the queue.
 *
 * This is O(1). On failure the queue is left unchanged.
 *
 * @param queue The queue to add to.
 * @param item  Pointer to the item to copy in, must point to one element
 *              (the element size given to queue_create()).
 * @return 0 on success, EINVAL if queue or item is NULL, or ENOMEM if the
 *         element could not be allocated.
 */
int queue_enqueue(Queue *queue, const void *item);

/**
 * @brief Removes the front element, copying it into a caller-supplied buffer.
 *
 * This is O(1). On failure the queue is left unchanged.
 *
 * @param queue The queue to remove from.
 * @param dest  Buffer to copy the element into, must hold at least one
 *              element.
 * @return 0 on success, EINVAL if queue or dest is NULL, or ENOENT if
 *         the queue is empty.
 */
int queue_dequeue(Queue *queue, void *dest);

/**
 * @brief Copies the front element into a caller-supplied buffer without
 *        removing it.
 *
 * @param queue The queue to read from.
 * @param dest  Buffer to copy the element into, must hold at least one
 *              element.
 * @return 0 on success, EINVAL if queue or dest is NULL, or ENOENT if
 *         the queue is empty.
 */
int queue_peek(const Queue *queue, void *dest);

/**
 * @brief Creates an empty queue.
 *
 * Elements are stored by value: pushed items are copied in, byte for byte.
 * The queue must be released with queue_destroy().
 *
 * @param memberSize Size in bytes of one element, e.g. sizeof(int). Must be
 *                   at least 1.
 * @return The new queue, or NULL if memberSize is 0 or too large, or memory
 *         could not be allocated.
 */
Queue *queue_create(size_t memberSize);

/**
 * @brief Frees a queue and all of its elements.
 *
 * Only the queue's own memory is freed. If the stored elements contain
 * pointers to other allocations, the caller must free those first. Passing
 * NULL is a no-op. The queue must not be used afterwards.
 *
 * @param queue The queue to destroy.
 */
void queue_destroy(Queue *queue);

#endif /* QUEUE_H_ */
