/**
 * @file heap.h
 * @brief Binary heap, configurable as a min heap or a max heap. (interface)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef HEAP_H_
#define HEAP_H_

#include "arrayList.h"
#include <stddef.h>

typedef struct Heap Heap;

/**
 * @brief Comparison callback that defines the ordering of the heap's elements.
 *
 * Must return a negative value if the first element is less than the second,
 * zero if they are equal, and a positive value if the first is greater. The
 * arguments point at elements stored inside the heap and must not be modified.
 */
typedef int (*Compare)(const void *, const void *);

/** @brief Which element sits at the top of the heap. */
typedef enum {
    HEAP_MIN, /**< The smallest element is on top. */
    HEAP_MAX  /**< The largest element is on top. */
} HeapType;

/**
 * @brief Creates an empty heap.
 *
 * Elements are stored by value: added items are copied in, byte for byte.
 * The heap must be released with heap_destroy().
 *
 * @param memSize Size in bytes of one element, e.g. sizeof(int). Must be at
 *                least 1.
 * @param type    HEAP_MIN or HEAP_MAX.
 * @param cmp     Comparison function defining the element ordering.
 * @return The new heap, or NULL if memSize is 0, type is not a valid
 *         HeapType, cmp is NULL, or memory could not be allocated.
 */
Heap *heap_create(size_t memSize, HeapType type, Compare cmp);

/**
 * @brief Creates a heap holding a copy of every element of a list.
 *
 * The heap is built in O(n). The source list is not modified and remains
 * owned by the caller. The heap must be released with heap_destroy().
 *
 * @param list    The list to copy elements from.
 * @param memSize Size in bytes of one element, must match the element size
 *                the list was created with.
 * @param type    HEAP_MIN or HEAP_MAX.
 * @param cmp     Comparison function defining the element ordering.
 * @return The new heap, or NULL if list or cmp is NULL, memSize is 0, type is
 *         not a valid HeapType, or memory could not be allocated.
 */
Heap *heap_from_list(const ArrayList *list, size_t memSize, HeapType type, Compare cmp);

/**
 * @brief Frees a heap and its storage.
 *
 * Only the heap's own memory is freed. If the stored elements contain
 * pointers to other allocations, the caller must free those first. Passing
 * NULL is a no-op. The heap must not be used afterwards.
 *
 * @param heap The heap to destroy.
 */
void heap_destroy(Heap *heap);

/**
 * @brief Gets the number of elements currently in the heap.
 *
 * @param heap The heap to query.
 * @return The element count, or 0 if heap is NULL.
 */
size_t heap_size(const Heap *heap);

/**
 * @brief Adds a copy of an item to the heap.
 *
 * O(log n) amortised. On failure the heap is left unchanged.
 *
 * @param heap The heap to add to.
 * @param item Pointer to the item to copy in, must point to one element.
 * @return 0 on success, EINVAL if heap or item is NULL, or ENOMEM if the heap
 *         could not grow.
 */
int heap_add(Heap *heap, const void *item);

/**
 * @brief Copies the top element into a caller-supplied buffer without
 *        removing it.
 *
 * @param heap The heap to read from.
 * @param dest Buffer to copy into, must hold at least one element.
 * @return 0 on success, EINVAL if heap or dest is NULL, or ENOENT if the heap
 *         is empty.
 */
int heap_peek(const Heap *heap, void *dest);

/**
 * @brief Removes the top element and copies it into a caller-supplied buffer.
 *
 * O(log n). On failure the heap is left unchanged.
 *
 * @param heap The heap to remove from.
 * @param dest Buffer to copy into, must hold at least one element.
 * @return 0 on success, EINVAL if heap or dest is NULL, or ENOENT if the heap
 *         is empty.
 */
int heap_pop(Heap *heap, void *dest);

#endif /* HEAP_H_ */
