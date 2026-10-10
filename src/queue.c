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

static int enqueue(Queue *self, const void *item)
{
    assert(self != NULL);
    LinkedList *data = PRIVATE(self)->data;
    return data->append(data, item);
}

static int dequeue(Queue *self, void *dest)
{
    assert(self != NULL);
    LinkedList *data = PRIVATE(self)->data;

    int error = data->get_first_cpy(data, dest);
    if (error) {
        return error;
    }

    return data->delete_first(data);
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
