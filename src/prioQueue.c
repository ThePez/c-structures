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

/**
 * @brief Moves an error from the inner Heap onto the wrapper.
 *
 * Clears the inner error so the next operation starts clean.
 */
static PrioQueue *sync_err(PrioQueue *self, Heap *data)
{
    self->err = data->err;
    data->err = 0;
    return self;
}

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

static PrioQueue *enqueue(PrioQueue *self, const void *item)
{
    if (!self || self->err) {
        return self;
    }

    Heap *data = PRIVATE(self)->data;
    data->add(data, item);
    return sync_err(self, data);
}

static PrioQueue *dequeue(PrioQueue *self, void *dest)
{
    if (!self || self->err) {
        return self;
    }

    Heap *data = PRIVATE(self)->data;
    data->pop(data, dest);
    return sync_err(self, data);
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
