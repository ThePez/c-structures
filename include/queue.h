/**
 * @file queue.h
 * @brief FIFO queue. (interface)
 *
 * Queues are created with queue_create() and used through the function
 * pointers stored in the struct, e.g. q->enqueue(q, &item). The first
 * argument is always the queue itself.
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>

typedef struct Queue Queue;

/**
 * @brief Public face of a queue: its methods.
 *
 * The remaining fields are private to queue.c. Only queue_create() can make
 * a valid queue, so never declare or copy a Queue by value.
 */
struct Queue {
    /**
     * @brief Gets the number of elements currently in the queue.
     *
     * @param self The queue to query.
     * @return The element count.
     */
    size_t (*size)(const Queue *self);

    /**
     * @brief Checks whether the queue holds no elements.
     *
     * @param self The queue to query.
     * @return 1 if the queue is empty, 0 if it has at least one element.
     */
    int (*empty)(const Queue *self);

    /**
     * @brief Adds a copy of an item to the back of the queue.
     *
     * This is O(1). On failure the queue is left unchanged.
     *
     * @param self The queue to add to.
     * @param item Pointer to the item to copy in, must point to one element
     *             (the element size given to queue_create()).
     * @return 0 on success, EINVAL if item is NULL, or ENOMEM if the element
     *         could not be allocated.
     */
    int (*enqueue)(Queue *self, const void *item);

    /**
     * @brief Removes the front element, copying it into a caller-supplied
     *        buffer.
     *
     * This is O(1). On failure the queue is left unchanged.
     *
     * @param self The queue to remove from.
     * @param dest Buffer to copy the element into, must hold at least one
     *             element.
     * @return 0 on success, EINVAL if dest is NULL, or ENOENT if the queue
     *         is empty.
     */
    int (*dequeue)(Queue *self, void *dest);

    /**
     * @brief Copies the front element into a caller-supplied buffer without
     *        removing it.
     *
     * @param self The queue to read from.
     * @param dest Buffer to copy the element into, must hold at least one
     *             element.
     * @return 0 on success, EINVAL if dest is NULL, or ENOENT if the queue
     *         is empty.
     */
    int (*peek)(const Queue *self, void *dest);

    /**
     * @brief Frees a queue and all of its elements.
     *
     * Only the queue's own memory is freed. If the stored elements contain
     * pointers to other allocations, the caller must free those first. The
     * queue must not be used afterwards.
     *
     * @param self The queue to destroy.
     */
    void (*destroy)(Queue *self);
};

/**
 * @brief Creates an empty queue.
 *
 * Elements are stored by value: enqueued items are copied in, byte for byte.
 * The queue must be released with its destroy() method.
 *
 * @param memberSize Size in bytes of one element, e.g. sizeof(int). Must be
 *                   at least 1.
 * @return The new queue, or NULL if memberSize is 0 or too large, or memory
 *         could not be allocated.
 */
Queue *queue_create(size_t memberSize);

#endif /* QUEUE_H_ */
