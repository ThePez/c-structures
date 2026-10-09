/**
 * @file queue.c
 * @brief FIFO queue. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "queue.h"
#include "linkedList.h"

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

struct Queue {
    LinkedList *data;
};

size_t queue_size(const Queue *queue)
{
    if (!queue) {
        return 0;
    }

    return linkedList_size(queue->data);
}

int queue_enqueue(Queue *queue, const void *item)
{
    if (!queue) {
        return EINVAL;
    }

    return linkedList_append(queue->data, item);
}

int queue_dequeue(Queue *queue, void *dest)
{
    if (!queue) {
        return EINVAL;
    }

    int error = linkedList_get_first_cpy(queue->data, dest);
    if (error) {
        return error;
    }

    return linkedList_delete_first(queue->data);
}

int queue_peek(const Queue *queue, void *dest)
{
    if (!queue) {
        return EINVAL;
    }

    return linkedList_get_first_cpy(queue->data, dest);
}

void queue_destroy(Queue *queue)
{
    if (!queue) {
        return;
    }

    linkedList_destroy(queue->data);
    free(queue);
}

Queue *queue_create(size_t memberSize)
{

    LinkedList *data = linkedList_create(memberSize);
    if (!data) {
        return NULL;
    }

    Queue *queue = malloc(sizeof(Queue));
    if (queue == NULL) {
        linkedList_destroy(data);
        return NULL;
    }

    queue->data = data;
    return queue;
}
