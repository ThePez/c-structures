/**
 * @file heap.c
 * @brief Binary heap, configurable as a min heap or a max heap. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "heap.h"
#include "arrayList.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>

struct Heap {
    ArrayList *data;
    Compare compFunc;
    size_t elementSize;
    HeapType type;
};

#define PARENT(idx)      (((idx) - 1) >> 1) // (idx - 1) / 2
#define LEFT_CHILD(idx)  (((idx) << 1) + 1) // 2 * idx + 1
#define RIGHT_CHILD(idx) (((idx) << 1) + 2) // 2 * idx + 2

/**
 * @brief Whether the element at index a belongs strictly above the element at
 *        index b: greater for a max heap, smaller for a min heap.
 *
 * @param heap The heap, must not be NULL.
 * @param a    Index of the first element, must be a valid index.
 * @param b    Index of the second element, must be a valid index.
 * @return Non-zero if a belongs strictly above b, 0 if b is above a or the two
 *         compare equal.
 */
static int above(const Heap *heap, size_t a, size_t b)
{
    assert(heap != NULL);
    int cmp = heap->compFunc(arrayList_get(heap->data, a), arrayList_get(heap->data, b));
    return heap->type ? cmp > 0 : cmp < 0;
}

/**
 * @brief Picks which child a node should be swapped with when sinking.
 *
 * Chooses the child that belongs higher in the heap (larger for a max heap,
 * smaller for a min heap). On a tie the right child is chosen.
 *
 * @param heap    The heap, must not be NULL.
 * @param idx     Index of the parent node.
 * @param leftIdx Index of the parent's left child, must be less than length.
 * @param length  Number of elements currently considered part of the heap.
 * @return The index of the left or right child.
 */
static size_t getSwapIdx(const Heap *heap, size_t idx, size_t leftIdx, size_t length)
{
    assert(heap != NULL);

    size_t rightIdx = RIGHT_CHILD(idx);

    // The right child wins ties with the left child
    if (rightIdx < length && !above(heap, leftIdx, rightIdx)) {
        return rightIdx;
    }

    return leftIdx;
}

/**
 * @brief Moves the element at an index up until its parent is above it.
 *
 * Restores the heap property after an element is added at the end. Runs in
 * O(log n).
 *
 * @param heap The heap, must not be NULL.
 * @param idx  Index of the element to move, must be a valid index.
 */
static void upHeap(Heap *heap, size_t idx)
{
    assert(heap != NULL);
    if (idx == 0) {
        // Root can't go up
        return;
    }

    if (above(heap, idx, PARENT(idx))) {
        arrayList_swap(heap->data, idx, PARENT(idx));
        upHeap(heap, PARENT(idx));
    }
}

/**
 * @brief Moves the element at an index down until both children are below it.
 *
 * Restores the heap property after the root is replaced, or while building a
 * heap. Only the first length elements are treated as part of the heap. Runs
 * in O(log n).
 *
 * @param heap   The heap, must not be NULL.
 * @param idx    Index of the element to move.
 * @param length Number of elements currently considered part of the heap.
 */
static void downHeap(Heap *heap, size_t idx, size_t length)
{
    assert(heap != NULL);
    size_t leftIdx = LEFT_CHILD(idx);
    if (leftIdx >= length) {
        // No children, so the node is a leaf
        return;
    }

    // Decide which child to swap with
    size_t swapIdx = getSwapIdx(heap, idx, leftIdx, length);

    if (above(heap, swapIdx, idx)) {
        arrayList_swap(heap->data, idx, swapIdx);
        downHeap(heap, swapIdx, length);
    }
}

/**
 * @brief Turns the heap's unordered contents into a valid heap in place.
 *
 * Sinks every internal node, from the last one back to the root. Runs in
 * O(n), which is cheaper than adding the elements one at a time.
 *
 * @param heap The heap, must not be NULL.
 */
static void heap_bottom_up_construct(Heap *heap)
{
    assert(heap != NULL);

    size_t n = arrayList_size(heap->data);

    for (size_t i = n / 2; i-- > 0;) {
        downHeap(heap, i, n);
    }
}

int heap_add(Heap *heap, const void *item)
{
    if (!heap || !item) {
        return EINVAL;
    }

    int error = arrayList_append(heap->data, item);
    if (error) {
        return error;
    }

    upHeap(heap, arrayList_size(heap->data) - 1);
    return 0;
}

int heap_peek(Heap *heap, void *dest)
{
    if (!heap) {
        return EINVAL;
    }

    return arrayList_get_first_cpy(heap->data, dest);
}

int heap_pop(Heap *heap, void *dest)
{
    if (!heap) {
        return EINVAL;
    }

    size_t size = arrayList_size(heap->data);
    if (!size) {
        return ENOENT;
    }

    int error = arrayList_get_first_cpy(heap->data, dest);
    if (error) {
        return error;
    }

    // Rest should just pass without failing...
    error = arrayList_swap(heap->data, 0, size - 1);
    assert(error == 0);
    error = arrayList_delete_last(heap->data);
    assert(error == 0);
    downHeap(heap, 0, size - 1);
    return 0;
}

/**
 * @brief Allocates an empty heap with the given settings.
 *
 * Shared by both public constructors.
 *
 * @param memSize  Size in bytes of one element, must be at least 1.
 * @param capacity Initial capacity in elements, must be at least 1.
 * @param type     HEAP_MIN or HEAP_MAX.
 * @param cmp      Comparison function, must not be NULL.
 * @return The new heap, or NULL if an argument is invalid or memory could not
 *         be allocated.
 */
static Heap *heap_alloc(size_t memSize, size_t capacity, HeapType type, Compare cmp)
{
    if (memSize < 1 || !cmp || (type != HEAP_MIN && type != HEAP_MAX)) {
        return NULL;
    }

    ArrayList *data = arrayList_create(memSize, capacity);
    if (!data) {
        return NULL;
    }

    Heap *heap = malloc(sizeof(Heap));
    if (!heap) {
        arrayList_destroy(data);
        return NULL;
    }

    heap->data = data;
    heap->compFunc = cmp;
    heap->elementSize = memSize;
    heap->type = type;

    return heap;
}

Heap *heap_from_list(const ArrayList *list, size_t memSize, HeapType type, Compare cmp)
{
    if (!list) {
        return NULL;
    }

    size_t size = arrayList_size(list);
    // An ArrayList cannot be created with zero capacity
    Heap *heap = heap_alloc(memSize, size ? size : 1, type, cmp);
    if (!heap) {
        return NULL;
    }

    // Copy the old list data into the new space
    for (size_t i = 0; i < size; i++) {
        if (arrayList_append(heap->data, arrayList_get(list, i)) != 0) {
            heap_destroy(heap);
            return NULL;
        }
    }

    heap_bottom_up_construct(heap);
    return heap;
}

Heap *heap_create(size_t memSize, HeapType type, Compare cmp)
{
    return heap_alloc(memSize, 10, type, cmp);
}

void heap_destroy(Heap *heap)
{
    if (!heap) {
        return;
    }

    arrayList_destroy(heap->data);
    free(heap);
}
