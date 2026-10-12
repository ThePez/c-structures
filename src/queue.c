/**
 * @file queue.c
 * @brief FIFO queue. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "queue.h"
#include "linkedList.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

typedef struct QueuePriv {
    Queue pub; // must stay first
    LinkedList *data;
} QueuePriv;

#define PRIVATE(q)       ((QueuePriv *)(q))
#define CONST_PRIVATE(q) ((const QueuePriv *)(q))

/**
 * @brief Moves an error from the inner LinkedList onto the wrapper.
 *
 * Clears the inner error so the next operation starts clean.
 */
static Queue *sync_err(Queue *self, LinkedList *data)
{
    self->err = data->err;
    data->err = 0;
    return self;
}

static size_t size(const Queue *self)
{
    assert(self != NULL);
    const LinkedList *data = CONST_PRIVATE(self)->data;
    return data->size(data);
}

static int empty(const Queue *self)
{
    assert(self != NULL);
    const LinkedList *data = CONST_PRIVATE(self)->data;
    return data->empty(data);
}

static Queue *enqueue(Queue *self, const void *item)
{
    if (!self || self->err) {
        return self;
    }

    LinkedList *data = PRIVATE(self)->data;
    data->append(data, item);
    return sync_err(self, data);
}

static Queue *dequeue(Queue *self, void *dest)
{
    if (!self || self->err) {
        return self;
    }

    LinkedList *data = PRIVATE(self)->data;

    int error = data->get_first_cpy(data, dest);
    if (error) {
        self->err = error;
        return self;
    }

    data->erase_first(data);
    return sync_err(self, data);
}

static int peek(const Queue *self, void *dest)
{
    assert(self != NULL);
    const LinkedList *data = CONST_PRIVATE(self)->data;
    return data->get_first_cpy(data, dest);
}

static void destroy(Queue *self)
{
    if (!self) {
        return;
    }

    LinkedList *data = PRIVATE(self)->data;
    data->destroy(data);
    free(self);
}

Queue *queue_create(size_t memberSize)
{
    LinkedList *data = linkedList_create(memberSize);
    if (!data) {
        return NULL;
    }

    QueuePriv *self = malloc(sizeof(QueuePriv));
    if (self == NULL) {
        data->destroy(data);
        return NULL;
    }

    self->data = data;

    self->pub = (const Queue){
        .size = size,
        .empty = empty,
        .enqueue = enqueue,
        .dequeue = dequeue,
        .peek = peek,
        .destroy = destroy,
    };

    return &self->pub;
}
