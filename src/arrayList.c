/**
 * @file arrayList.c
 * @brief Dynamic array list that grows as elements are added. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "arrayList.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct ArrayList {
    size_t capacity, size, elementSize;
    char *data;
};

/**
 * @brief Doubles the capacity of a list.
 *
 * On failure the list is left unchanged.
 *
 * @param list The list to grow.
 * @return 0 on success, or ENOMEM if the new size overflows or the allocation
 *         fails.
 */
static int arrayList_resize(ArrayList *list)
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

size_t arrayList_size(const ArrayList *list)
{
    if (!list) {
        return 0;
    }

    return list->size;
}

const void *arrayList_get(const ArrayList *list, size_t idx)
{
    if (!list || idx >= list->size) {
        return NULL;
    }

    return list->data + (list->elementSize * idx);
}

const void *arrayList_get_first(const ArrayList *list)
{
    return arrayList_get(list, 0);
}

const void *arrayList_get_last(const ArrayList *list)
{
    if (!list || !list->size) {
        return NULL;
    }

    return arrayList_get(list, list->size - 1);
}

int arrayList_get_cpy(const ArrayList *list, size_t idx, void *dest)
{
    if (!list || idx >= list->size || !dest) {
        return EINVAL;
    }

    size_t offset = list->elementSize * idx;
    memcpy(dest, list->data + offset, list->elementSize);
    return 0;
}

int arrayList_get_first_cpy(const ArrayList *list, void *dest)
{
    if (!list || !dest) {
        return EINVAL;
    }

    if (!list->size) {
        return ENOENT;
    }

    return arrayList_get_cpy(list, 0, dest);
}

int arrayList_get_last_cpy(const ArrayList *list, void *dest)
{
    if (!list || !dest) {
        return EINVAL;
    }

    if (!list->size) {
        return ENOENT;
    }

    return arrayList_get_cpy(list, list->size - 1, dest);
}

int arrayList_delete(ArrayList *list, size_t idx)
{
    if (!list || idx >= list->size) {
        return EINVAL;
    }

    size_t offset = list->elementSize;
    char *base = list->data;

    // memmove allows overlapping reigions
    memmove(base + (idx * offset), base + ((idx + 1) * offset), (list->size - idx - 1) * offset);
    list->size--;
    return 0;
}

int arrayList_delete_first(ArrayList *list)
{
    if (!list) {
        return EINVAL;
    }

    if (!list->size) {
        return ENOENT;
    }

    return arrayList_delete(list, 0);
}

int arrayList_delete_last(ArrayList *list)
{
    if (!list) {
        return EINVAL;
    }

    if (!list->size) {
        return ENOENT;
    }

    list->size--;
    return 0;
}

int arrayList_set(ArrayList *list, size_t idx, const void *item)
{
    if (!item || !list || idx >= list->size) {
        return EINVAL;
    }

    size_t slot = idx * list->elementSize;
    memcpy(list->data + slot, item, list->elementSize);
    return 0;
}

int arrayList_insert(ArrayList *list, size_t idx, const void *item)
{
    if (!item || !list || idx > list->size) {
        return EINVAL;
    }

    int error = 0;
    if (list->size == list->capacity) {
        error = arrayList_resize(list);
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

int arrayList_append(ArrayList *list, const void *item)
{
    if (!list) {
        return EINVAL;
    }

    return arrayList_insert(list, list->size, item);
}

ArrayList *arrayList_create(size_t memSize, size_t length)
{
    if (length < 1 || memSize < 1) {
        return NULL;
    }

    ArrayList *dynamicArray = malloc(sizeof(ArrayList));
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

    dynamicArray->capacity = length;
    dynamicArray->size = 0;
    dynamicArray->elementSize = memSize;

    return dynamicArray;
}

void arrayList_destroy(ArrayList *list)
{
    if (!list) {
        return;
    }

    free(list->data);
    free(list);
}
