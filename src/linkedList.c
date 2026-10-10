/**
 * @file linkedList.c
 * @brief Linked list of dynamically allocated nodes. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "linkedList.h"

#include <assert.h>
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

typedef struct LinkedListPriv {
    LinkedList pub; // must stay first
    Node *head, *tail;
    size_t size, elementSize;
} LinkedListPriv;

#define PRIVATE(l)       ((LinkedListPriv *)(l))
#define CONST_PRIVATE(l) ((const LinkedListPriv *)(l))

static size_t size(const LinkedList *self)
{
    assert(self != NULL);
    return CONST_PRIVATE(self)->size;
}

static int empty(const LinkedList *self)
{
    assert(self != NULL);
    return CONST_PRIVATE(self)->size > 0 ? 0 : 1;
}

/**
 * @brief Finds the node at an index, walking from whichever end is closer.
 *
 * @param list The list to search.
 * @param idx  Index of the node, must be less than the list size.
 * @return The node at idx.
 */
static Node *get_node(const LinkedListPriv *list, size_t idx)
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

static const void *get(const LinkedList *self, size_t idx)
{
    assert(self != NULL);
    const LinkedListPriv *list = CONST_PRIVATE(self);
    if (idx >= list->size) {
        return NULL;
    }

    return get_node(list, idx)->data;
}

static const void *get_first(const LinkedList *self)
{
    assert(self != NULL);
    if (empty(self)) {
        return NULL;
    }

    return CONST_PRIVATE(self)->head->data;
}

static const void *get_last(const LinkedList *self)
{
    assert(self != NULL);
    if (empty(self)) {
        return NULL;
    }

    return CONST_PRIVATE(self)->tail->data;
}

static int get_cpy(const LinkedList *self, size_t idx, void *dest)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    const LinkedListPriv *list = CONST_PRIVATE(self);
    if (!dest || idx >= list->size) {
        return EINVAL;
    }

    memcpy(dest, get_node(list, idx)->data, list->elementSize);
    return 0;
}

static int get_first_cpy(const LinkedList *self, void *dest)
{
    assert(self != NULL);
    if (!dest) {
        return EINVAL;
    }

    if (empty(self)) {
        return ENOENT;
    }

    const LinkedListPriv *list = CONST_PRIVATE(self);
    memcpy(dest, list->head->data, list->elementSize);
    return 0;
}

static int get_last_cpy(const LinkedList *self, void *dest)
{
    assert(self != NULL);
    if (!dest) {
        return EINVAL;
    }

    if (empty(self)) {
        return ENOENT;
    }

    const LinkedListPriv *list = CONST_PRIVATE(self);
    memcpy(dest, list->tail->data, list->elementSize);
    return 0;
}

static int delete_first(LinkedList *self)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    LinkedListPriv *list = PRIVATE(self);
    Node *head = list->head;
    list->head = head->next;
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

static int delete_last(LinkedList *self)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    LinkedListPriv *list = PRIVATE(self);
    Node *tail = list->tail;
    list->tail = tail->prev;
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

static int delete(LinkedList *self, size_t idx)
{
    assert(self != NULL);
    LinkedListPriv *list = PRIVATE(self);
    if (idx >= list->size) {
        return EINVAL;
    }

    if (idx == 0) {
        return delete_first(self);
    }

    if (idx == list->size - 1) {
        return delete_last(self);
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

static int insert(LinkedList *self, size_t idx, const void *item)
{
    assert(self != NULL);
    LinkedListPriv *list = PRIVATE(self);
    if (!item || idx > list->size) {
        return EINVAL;
    }

    Node *node = malloc(sizeof(Node) + list->elementSize);
    if (!node) {
        return ENOMEM;
    }

    // Walk along the block to store the actual data
    node->data = (char *)(node + 1);
    memcpy(node->data, item, list->elementSize);

    if (empty(self)) {
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

static int append(LinkedList *self, const void *item)
{
    assert(self != NULL);
    return insert(self, size(self), item);
}

static int set(LinkedList *self, size_t idx, const void *item)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    LinkedListPriv *list = PRIVATE(self);
    if (!item || idx >= list->size) {
        return EINVAL;
    }

    memcpy(get_node(list, idx)->data, item, list->elementSize);
    return 0;
}

static void destroy(LinkedList *self)
{
    LinkedListPriv *list = PRIVATE(self);
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

LinkedList *linkedList_create(size_t memSize)
{
    // Reject zero-size elements and sizes that would overflow node allocation
    if (memSize < 1 || memSize > SIZE_MAX - sizeof(Node)) {
        return NULL;
    }

    LinkedListPriv *list = malloc(sizeof(LinkedListPriv));
    if (list == NULL) {
        return NULL;
    }

    list->pub = (const LinkedList){
        .size = size,
        .empty = empty,
        .get = get,
        .get_first = get_first,
        .get_last = get_last,
        .get_cpy = get_cpy,
        .get_first_cpy = get_first_cpy,
        .get_last_cpy = get_last_cpy,
        .delete = delete,
        .delete_first = delete_first,
        .delete_last = delete_last,
        .insert = insert,
        .append = append,
        .set = set,
        .destroy = destroy,
    };

    list->elementSize = memSize;
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    return &list->pub;
}
