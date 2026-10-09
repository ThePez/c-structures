/**
 * @file prioQueue.c
 * @brief Priority queue where elements are removed in priority order. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "prioQueue.h"
#include "heap.h"

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

struct PrioQueue {
    Heap *data;
};

size_t prioQueue_size(const PrioQueue *queue)
{
    if (!queue) {
        return 0;
    }

    return heap_size(queue->data);
}

int prioQueue_enqueue(PrioQueue *queue, const void *item)
{
    if (!queue) {
        return EINVAL;
    }

    return heap_add(queue->data, item);
}

int prioQueue_dequeue(PrioQueue *queue, void *dest)
{
    if (!queue) {
        return EINVAL;
    }

    return heap_pop(queue->data, dest);
}

int prioQueue_peek(const PrioQueue *queue, void *dest)
{
    if (!queue) {
        return EINVAL;
    }

    return heap_peek(queue->data, dest);
}

void prioQueue_destroy(PrioQueue *queue)
{
    if (!queue) {
        return;
    }

    heap_destroy(queue->data);
    free(queue);
}

PrioQueue *prioQueue_create(size_t memberSize, HeapType type, Compare cmp)
{
    Heap *data = heap_create(memberSize, type, cmp);
    if (!data) {
        return NULL;
    }

    PrioQueue *queue = malloc(sizeof(PrioQueue));
    if (!queue) {
        heap_destroy(data);
        return NULL;
    }

    queue->data = data;
    return queue;
}
