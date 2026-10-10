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

typedef struct HeapPriv {
    Heap pub; // must stay first: Heap * and HeapPriv * are interchangeable
    ArrayList *data;
    Compare compFunc;
    size_t elementSize;
    HeapType type;
} HeapPriv;

#define PRIVATE(h)       ((HeapPriv *)(h))
#define CONST_PRIVATE(h) ((const HeapPriv *)(h))

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
static int above(const HeapPriv *heap, size_t a, size_t b)
{
    assert(heap != NULL);
    const ArrayList *data = heap->data;
    int cmp = heap->compFunc(data->get(data, a), data->get(data, b));
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
static size_t getSwapIdx(const HeapPriv *heap, size_t idx, size_t leftIdx, size_t length)
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
static void upHeap(HeapPriv *heap, size_t idx)
{
    assert(heap != NULL);
    if (idx == 0) {
        // Root can't go up
        return;
    }

    if (above(heap, idx, PARENT(idx))) {
        heap->data->swap(heap->data, idx, PARENT(idx));
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
static void downHeap(HeapPriv *heap, size_t idx, size_t length)
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
        heap->data->swap(heap->data, idx, swapIdx);
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
static void heap_bottom_up_construct(HeapPriv *heap)
{
    assert(heap != NULL);
    const ArrayList *data = heap->data;
    size_t n = data->size(data);

    for (size_t i = n / 2; i-- > 0;) {
        downHeap(heap, i, n);
    }
}

static size_t size(const Heap *self)
{
    assert(self != NULL);
    const ArrayList *data = CONST_PRIVATE(self)->data;
    return data->size(data);
}

static int empty(const Heap *self)
{
    assert(self != NULL);
    const ArrayList *data = CONST_PRIVATE(self)->data;
    return data->empty(data);
}

static int add(Heap *self, const void *item)
{
    assert(self != NULL);
    if (!item) {
        return EINVAL;
    }

    HeapPriv *heap = PRIVATE(self);
    ArrayList *data = heap->data;
    int error = data->append(data, item);
    if (error) {
        return error;
    }

    upHeap(heap, data->size(data) - 1);
    return 0;
}

static int peek(const Heap *self, void *dest)
{
    assert(self != NULL);
    const ArrayList *data = CONST_PRIVATE(self)->data;
    return data->get_first_cpy(data, dest);
}

static int pop(Heap *self, void *dest)
{
    assert(self != NULL);
    if (empty(self)) {
        return ENOENT;
    }

    HeapPriv *heap = PRIVATE(self);
    ArrayList *data = heap->data;
    int error = data->get_first_cpy(data, dest);
    if (error) {
        return error;
    }

    size_t count = data->size(data);

    // Rest should just pass without failing...
    error = data->swap(data, 0, count - 1);
    assert(error == 0);
    error = data->delete_last(data);
    assert(error == 0);
    (void)error;
    downHeap(heap, 0, count - 1);
    return 0;
}

static void destroy(Heap *self)
{
    if (!self) {
        return;
    }

    ArrayList *data = PRIVATE(self)->data;
    data->destroy(data);
    free(self);
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
static HeapPriv *heap_alloc(size_t memSize, size_t capacity, HeapType type, Compare cmp)
{
    if (memSize < 1 || !cmp || (type != HEAP_MIN && type != HEAP_MAX)) {
        return NULL;
    }

    ArrayList *data = arrayList_create_cap(memSize, capacity);
    if (!data) {
        return NULL;
    }

    HeapPriv *heap = malloc(sizeof(HeapPriv));
    if (!heap) {
        data->destroy(data);
        return NULL;
    }

    heap->pub = (const Heap){
        .size = size,
        .empty = empty,
        .add = add,
        .peek = peek,
        .pop = pop,
        .destroy = destroy,
    };

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

    size_t count = list->size(list);
    HeapPriv *heap = heap_alloc(memSize, count ? count : 1, type, cmp);
    if (!heap) {
        return NULL;
    }

    // Copy the old list data into the new space
    ArrayList *data = heap->data;
    for (size_t i = 0; i < count; i++) {
        if (data->append(data, list->get(list, i)) != 0) {
            destroy(&heap->pub);
            return NULL;
        }
    }

    heap_bottom_up_construct(heap);
    return &heap->pub;
}

Heap *heap_create(size_t memSize, HeapType type, Compare cmp)
{
    HeapPriv *heap = heap_alloc(memSize, 10, type, cmp);
    return heap ? &heap->pub : NULL;
}
