/**
 * @file linkedList.c
 * @brief Linked list of dynamically allocated nodes. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "linkedList.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct Node Node;

struct Node {
    Node *next, *prev;
    char *data;
    char *reserved; // Pads Node to a multiple of max_align_t
};

// The payload is stored directly after each Node, so sizeof(Node) must keep it
// suitably aligned for any element type.
_Static_assert(sizeof(Node) % _Alignof(max_align_t) == 0,
               "Node must be padded so the payload after it is max-aligned");

struct LinkedList {
    Node *head, *tail;
    size_t size, elementSize;
};

size_t linkedList_size(const LinkedList *list)
{
    if (!list) {
        return 0;
    }

    return list->size;
}

static Node *get_node(const LinkedList *list, size_t idx)
{
    Node *current = NULL;
    if (idx > list->size / 2) {
        // Walk from tail
        current = list->tail;
        for (size_t steps = list->size - 1 - idx; steps > 0; steps--) {
            current = current->prev;
        }
    } else {
        // Walk from head
        current = list->head;
        for (size_t steps = idx; steps > 0; steps--) {
            current = current->next;
        }
    }

    return current;
}

const void *linkedList_get(const LinkedList *list, size_t idx)
{
    if (!list || idx >= list->size) {
        return NULL;
    }

    return get_node(list, idx)->data;
}

const void *linkedList_get_first(const LinkedList *list)
{
    if (!list || !list->size) {
        return NULL;
    }

    return list->head->data;
}

const void *linkedList_get_last(const LinkedList *list)
{
    if (!list || !list->size) {
        return NULL;
    }

    return list->tail->data;
}

int linkedList_get_cpy(const LinkedList *list, size_t idx, void *dest)
{
    if (!list || !dest || idx >= list->size) {
        return EINVAL;
    }

    Node *node = get_node(list, idx);
    memcpy(dest, node->data, list->elementSize);
    return 0;
}

int linkedList_get_first_cpy(const LinkedList *list, void *dest)
{
    if (!list || !dest || !list->size) {
        return EINVAL;
    }

    memcpy(dest, list->head->data, list->elementSize);
    return 0;
}

int linkedList_get_last_cpy(const LinkedList *list, void *dest)
{
    if (!list || !dest || !list->size) {
        return EINVAL;
    }

    memcpy(dest, list->tail->data, list->elementSize);
    return 0;
}

int linkedList_delete(LinkedList *list, size_t idx)
{
    if (!list || idx >= list->size) {
        return EINVAL;
    }

    if (idx == 0) {
        return linkedList_delete_first(list);
    }

    if (idx == list->size - 1) {
        return linkedList_delete_last(list);
    }

    Node *node = get_node(list, idx);
    Node *before = node->prev;
    Node *after = node->next;

    // Remove from chain
    before->next = after;
    after->prev = before;
    list->size--;

    // Now cleanup
    free(node);
    return 0;
}

int linkedList_delete_first(LinkedList *list)
{
    if (!list || !list->size) {
        return EINVAL;
    }

    Node *next = list->head->next;
    Node *head = list->head;
    list->head = next;
    if (list->head) {
        list->head->prev = NULL;
    }

    free(head);
    list->size--;
    if (!list->size) {
        list->tail = NULL;
    }

    return 0;
}

int linkedList_delete_last(LinkedList *list)
{
    if (!list || !list->size) {
        return EINVAL;
    }

    Node *prev = list->tail->prev;
    Node *tail = list->tail;
    list->tail = prev;
    if (list->tail) {
        list->tail->next = NULL;
    }

    free(tail);
    list->size--;
    if (!list->size) {
        list->head = NULL;
    }

    return 0;
}

int linkedList_insert(LinkedList *list, size_t idx, const void *item)
{
    if (!list || !item || idx > list->size) {
        return EINVAL;
    }

    Node *node = malloc(sizeof(Node) + list->elementSize);
    if (!node) {
        return ENOMEM;
    }

    // Walk along the block to store the actual data
    node->data = (char *)(node + 1);
    memcpy(node->data, item, list->elementSize);

    if (list->size == 0) {
        // Empty list
        node->next = NULL;
        node->prev = NULL;
        list->head = node;
        list->tail = node;
        list->size++;
        return 0;
    }

    if (idx == 0) {
        // New Head
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    } else if (idx == list->size) {
        // New tail
        node->next = NULL;
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    } else {
        // Middle: both neighbours present

        // Now get the old node in idx slot
        Node *oldNode = get_node(list, idx);
        Node *before = oldNode->prev;

        // Update pointers
        before->next = node;
        node->prev = before;
        node->next = oldNode;
        oldNode->prev = node;
    }

    list->size++;
    return 0;
}

int linkedList_append(LinkedList *list, const void *item)
{
    if (!list) {
        return EINVAL;
    }

    return linkedList_insert(list, list->size, item);
}

int linkedList_set(LinkedList *list, size_t idx, const void *item)
{
    if (!list || !item || idx >= list->size) {
        return EINVAL;
    }

    Node *node = get_node(list, idx);
    memcpy(node->data, item, list->elementSize);
    return 0;
}

LinkedList *linkedList_create(size_t memSize)
{
    // Reject zero-size elements and sizes that would overflow node allocation
    if (memSize < 1 || memSize > SIZE_MAX - sizeof(Node)) {
        return NULL;
    }

    LinkedList *list = malloc(sizeof(LinkedList));
    if (list == NULL) {
        return NULL;
    }

    list->elementSize = memSize;
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void linkedList_destroy(LinkedList *list)
{
    if (!list) {
        return;
    }

    Node *current = list->head;
    while (current) {
        Node *tmp = current;
        current = current->next;
        free(tmp);
    }

    free(list);
}
