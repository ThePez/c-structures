/**
 * @file prioQueue.c
 * @brief Priority queue where elements are removed in priority order. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "prioQueue.h"
#include "heap.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

typedef struct PrioQueuePriv {
    PrioQueue pub; // must stay first
    Heap *data;
} PrioQueuePriv;

#define PRIVATE(q)       ((PrioQueuePriv *)(q))
#define CONST_PRIVATE(q) ((const PrioQueuePriv *)(q))

static size_t size(const PrioQueue *self)
{
    assert(self != NULL);
    const Heap *data = CONST_PRIVATE(self)->data;
    return data->size(data);
}

static int empty(const PrioQueue *self)
{
    assert(self != NULL);
    const Heap *data = CONST_PRIVATE(self)->data;
    return data->empty(data);
}

static int enqueue(PrioQueue *self, const void *item)
{
    assert(self != NULL);
    Heap *data = PRIVATE(self)->data;
    return data->add(data, item);
}

static int dequeue(PrioQueue *self, void *dest)
{
    assert(self != NULL);
    Heap *data = PRIVATE(self)->data;
    return data->pop(data, dest);
}

static int peek(const PrioQueue *self, void *dest)
{
    assert(self != NULL);
    const Heap *data = CONST_PRIVATE(self)->data;
    return data->peek(data, dest);
}

static void destroy(PrioQueue *self)
{
    if (!self) {
        return;
    }

    Heap *data = PRIVATE(self)->data;
    data->destroy(data);
    free(self);
}

PrioQueue *prioQueue_create(size_t memberSize, HeapType type, Compare cmp)
{
    Heap *data = heap_create(memberSize, type, cmp);
    if (!data) {
        return NULL;
    }

    PrioQueuePriv *self = malloc(sizeof(PrioQueuePriv));
    if (!self) {
        data->destroy(data);
        return NULL;
    }

    self->data = data;

    self->pub = (const PrioQueue){
        .size = size,
        .empty = empty,
        .enqueue = enqueue,
        .dequeue = dequeue,
        .peek = peek,
        .destroy = destroy,
    };

    return &self->pub;
}
