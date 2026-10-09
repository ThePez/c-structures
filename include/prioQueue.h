/**
 * @file prioQueue.h
 * @brief Priority queue where elements are removed in priority order. (interface)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef PRIO_QUEUE_H_
#define PRIO_QUEUE_H_

#include "heap.h"
#include <stddef.h>

typedef struct PrioQueue PrioQueue;

/**
 * @brief Gets the number of elements currently in the queue.
 *
 * @param queue The queue to query.
 * @return The element count, or 0 if queue is NULL.
 */
size_t prioQueue_size(const PrioQueue *queue);

/**
 * @brief Adds a copy of an item to the queue.
 *
 * This is O(log n) amortised. On failure the queue is left unchanged.
 *
 * @param queue The queue to add to.
 * @param item  Pointer to the item to copy in, must point to one element
 *              (the element size given to prioQueue_create()).
 * @return 0 on success, EINVAL if queue or item is NULL, or ENOMEM if the
 *         queue could not grow.
 */
int prioQueue_enqueue(PrioQueue *queue, const void *item);

/**
 * @brief Removes the highest-priority element, copying it into a
 *        caller-supplied buffer.
 *
 * This is O(log n). On failure the queue is left unchanged.
 *
 * @param queue The queue to remove from.
 * @param dest  Buffer to copy the element into, must hold at least one
 *              element.
 * @return 0 on success, EINVAL if queue or dest is NULL, or ENOENT if
 *         the queue is empty.
 */
int prioQueue_dequeue(PrioQueue *queue, void *dest);

/**
 * @brief Copies the highest-priority element into a caller-supplied buffer
 *        without removing it.
 *
 * @param queue The queue to read from.
 * @param dest  Buffer to copy the element into, must hold at least one
 *              element.
 * @return 0 on success, EINVAL if queue or dest is NULL, or ENOENT if
 *         the queue is empty.
 */
int prioQueue_peek(const PrioQueue *queue, void *dest);

/**
 * @brief Creates an empty priority queue.
 *
 * Elements are stored by value: enqueued items are copied in, byte for byte.
 * The queue must be released with prioQueue_destroy().
 *
 * @param memberSize Size in bytes of one element, e.g. sizeof(int). Must be
 *                   at least 1.
 * @param type       HEAP_MIN to remove the smallest element first, or
 *                   HEAP_MAX to remove the largest first.
 * @param cmp        Comparison function defining the element ordering.
 * @return The new queue, or NULL if memberSize is 0, type is not a valid
 *         HeapType, cmp is NULL, or memory could not be allocated.
 */
PrioQueue *prioQueue_create(size_t memberSize, HeapType type, Compare cmp);

/**
 * @brief Frees a queue and all of its elements.
 *
 * Only the queue's own memory is freed. If the stored elements contain
 * pointers to other allocations, the caller must free those first. Passing
 * NULL is a no-op. The queue must not be used afterwards.
 *
 * @param queue The queue to destroy.
 */
void prioQueue_destroy(PrioQueue *queue);

#endif /* PRIO_QUEUE_H_ */
