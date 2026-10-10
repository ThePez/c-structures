/**
 * @file arrayList.c
 * @brief Dynamic array list that grows as elements are added. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "arrayList.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <asm-generic/errno-base.h>

typedef struct ArrayListPriv {
    ArrayList pub; // must stay first: ArrayList * and ArrayListPriv * are interchangeable
    size_t capacity, size, elementSize;
    char *data;
} ArrayListPriv;

#define PRIVATE(l)       ((ArrayListPriv *)(l))
#define CONST_PRIVATE(l) ((const ArrayListPriv *)(l))

/**
 * @brief Doubles the capacity of a list.
 *
 * On failure the list is left unchanged.
 *
 * @param list The list to grow.
 * @return 0 on success, or ENOMEM if the new size overflows or the allocation
 *         fails.
 */
static int resize(ArrayListPriv *list)
{
    // Check for overflow
    if (list->capacity > SIZE_MAX / 2) {
        return ENOMEM;
    }

    size_t newLength = list->capacity * 2;
    // Check for overflow
    if (newLength > SIZE_MAX / list->elementSize) {
        return ENOMEM;
    }

    // Get a new block of memory 2 times the old one
    void *tmp = realloc(list->data, list->elementSize * newLength);
    if (!tmp) {
        return ENOMEM;
    }

    // Update struct
    list->data = tmp;
    list->capacity = newLength;
    return 0;
}

static size_t size(const ArrayList *self)
{
    assert(self != NULL);
    return CONST_PRIVATE(self)->size;
}

static int empty(const ArrayList *self)
{
    assert(self != NULL);
    return CONST_PRIVATE(self)->size > 0 ? 0 : 1;
}

static int swap(ArrayList *self, size_t i, size_t j)
{
    ArrayListPriv *list = PRIVATE(self);
    if (!list) {
        return EINVAL;
    }

    size_t size = list->size;
    if (i >= size || j >= size) {
        return EINVAL;
    }

    if (i == j) {
        return 0;
    }

    // Element base addresses
    size_t bytes = list->elementSize;
    char *base_i = list->data + (i * bytes);
    char *base_j = list->data + (j * bytes);

    while (bytes--) {
        char tmp = *base_i;
        *base_i++ = *base_j;
        *base_j++ = tmp;
    }

    return 0;
}

static const void *get(const ArrayList *self, size_t idx)
{
    assert(self != NULL);
    const ArrayListPriv *list = CONST_PRIVATE(self);
    if (idx >= list->size) {
        return NULL;
    }

    return list->data + (list->elementSize * idx);
}

static const void *get_first(const ArrayList *self)
{
    return get(self, 0);
}

static const void *get_last(const ArrayList *self)
{
    assert(self != NULL);
    if (empty(self)) {
        return NULL;
    }

    return get(self, CONST_PRIVATE(self)->size - 1);
}

static int get_cpy(const ArrayList *self, size_t idx, void *dest)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    const ArrayListPriv *list = CONST_PRIVATE(self);
    if (idx >= list->size || !dest) {
        return EINVAL;
    }

    size_t offset = list->elementSize * idx;
    memcpy(dest, list->data + offset, list->elementSize);
    return 0;
}

static int get_first_cpy(const ArrayList *self, void *dest)
{
    assert(self != NULL);
    if (!dest) {
        return EINVAL;
    }

    return get_cpy(self, 0, dest);
}

static int get_last_cpy(const ArrayList *self, void *dest)
{
    assert(self != NULL);
    if (!dest) {
        return EINVAL;
    }

    if (empty(self)) {
        return ENOENT;
    }

    return get_cpy(self, CONST_PRIVATE(self)->size - 1, dest);
}

static int delete(ArrayList *self, size_t idx)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    ArrayListPriv *list = PRIVATE(self);
    if (idx >= list->size) {
        return EINVAL;
    }

    size_t offset = list->elementSize;
    char *base = list->data;

    // memmove allows overlapping reigions
    memmove(base + (idx * offset), base + ((idx + 1) * offset), (list->size - idx - 1) * offset);
    list->size--;
    return 0;
}

static int delete_first(ArrayList *self)
{
    assert(self != NULL);
    return delete (self, 0);
}

static int delete_last(ArrayList *self)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    PRIVATE(self)->size--;
    return 0;
}

static int set(ArrayList *self, size_t idx, const void *item)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    ArrayListPriv *list = PRIVATE(self);
    if (!item || idx >= list->size) {
        return EINVAL;
    }

    size_t slot = idx * list->elementSize;
    memcpy(list->data + slot, item, list->elementSize);
    return 0;
}

static int insert(ArrayList *self, size_t idx, const void *item)
{
    assert(self != NULL);
    ArrayListPriv *list = PRIVATE(self);
    if (!item || idx > list->size) {
        return EINVAL;
    }

    int error = 0;
    if (list->size == list->capacity) {
        error = resize(list);
        if (error) {
            return error;
        }
    }

    size_t offset = list->elementSize;
    size_t slot = idx * list->elementSize;

    char *newDataSlot = list->data + slot;
    char *movedDataSlot = newDataSlot + offset;

    memmove(movedDataSlot, newDataSlot, (list->size - idx) * list->elementSize);
    memcpy(newDataSlot, item, list->elementSize);
    list->size++;
    return 0;
}

static int append(ArrayList *self, const void *item)
{
    assert(self != NULL);
    return insert(self, size(self), item);
}

static void destroy(ArrayList *self)
{
    ArrayListPriv *list = PRIVATE(self);
    if (!list) {
        return;
    }

    free(list->data);
    free(list);
}

ArrayList *arrayList_create(size_t memSize, size_t length)
{
    if (length < 1 || memSize < 1) {
        return NULL;
    }

    ArrayListPriv *dynamicArray = malloc(sizeof(ArrayListPriv));
    if (dynamicArray == NULL) {
        return NULL;
    }

    if (length > SIZE_MAX / memSize) {
        free(dynamicArray);
        return NULL;
    }

    dynamicArray->data = malloc(memSize * length);
    if (dynamicArray->data == NULL) {
        free(dynamicArray);
        return NULL;
    }

    dynamicArray->pub = (const ArrayList){
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
        .swap = swap,
        .destroy = destroy,
    };

    dynamicArray->capacity = length;
    dynamicArray->size = 0;
    dynamicArray->elementSize = memSize;

    return &dynamicArray->pub;
}
