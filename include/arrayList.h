/**
 * @file arrayList.h
 * @brief Dynamic array list that grows as elements are added. (interface)
 *
 * Lists are created with arrayList_create() and used through the function
 * pointers stored in the struct, e.g. list->append(list, &item). The first
 * argument is always the list itself.
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef ARRAY_LIST_H_
#define ARRAY_LIST_H_

#include <stdint.h>
#include <stddef.h>

typedef struct ArrayList ArrayList;

/**
 * @brief Public face of an array list: its methods.
 *
 * The remaining fields are private to arrayList.c. Only arrayList_create()
 * can make a valid list, so never declare or copy an ArrayList by value.
 */
struct ArrayList {
    /**
     * @brief Gets the number of elements currently stored in the list.
     *
     * @param self The list to query.
     * @return The element count.
     */
    size_t (*size)(const ArrayList *self);

    /**
     * @brief Checks whether the list holds no elements.
     *
     * @param self The list to query.
     * @return 1 if the list is empty, 0 if it has at least one element.
     */
    int (*empty)(const ArrayList *self);

    /**
     * @brief Gets a read-only pointer to the element at an index.
     *
     * The pointer refers to the list's internal storage and is only valid
     * until the list is next modified (insert, append, delete or destroy),
     * since those may shift or move elements. Use get_cpy() for a copy that
     * stays valid.
     *
     * @param self The list to read from.
     * @param idx  Index of the element, must be less than the list size.
     * @return Pointer to the element, or NULL if idx is out of range.
     */
    const void *(*get)(const ArrayList *self, size_t idx);

    /**
     * @brief Gets a read-only pointer to the first element.
     *
     * The same validity rules as get() apply.
     *
     * @param self The list to read from.
     * @return Pointer to the first element, or NULL if the list is empty.
     */
    const void *(*get_first)(const ArrayList *self);

    /**
     * @brief Gets a read-only pointer to the last element.
     *
     * The same validity rules as get() apply.
     *
     * @param self The list to read from.
     * @return Pointer to the last element, or NULL if the list is empty.
     */
    const void *(*get_last)(const ArrayList *self);

    /**
     * @brief Copies the element at an index into a caller-supplied buffer.
     *
     * @param self The list to read from.
     * @param idx  Index of the element, must be less than the list size.
     * @param dest Buffer to copy into, must hold at least one element
     *             (the element size given to arrayList_create()).
     * @return 0 on success, or EINVAL if dest is NULL or idx is out of
     *         range.
     */
    int (*get_cpy)(const ArrayList *self, size_t idx, void *dest);

    /**
     * @brief Copies the first element into a caller-supplied buffer.
     *
     * @param self The list to read from.
     * @param dest Buffer to copy into, must hold at least one element.
     * @return 0 on success, EINVAL if dest is NULL, or ENOENT if the list
     *         is empty.
     */
    int (*get_first_cpy)(const ArrayList *self, void *dest);

    /**
     * @brief Copies the last element into a caller-supplied buffer.
     *
     * @param self The list to read from.
     * @param dest Buffer to copy into, must hold at least one element.
     * @return 0 on success, EINVAL if dest is NULL, or ENOENT if the list
     *         is empty.
     */
    int (*get_last_cpy)(const ArrayList *self, void *dest);

    /**
     * @brief Removes the element at an index.
     *
     * Elements after idx are shifted down by one to close the gap, so this
     * is O(n). The list's capacity is not reduced.
     *
     * @param self The list to remove from.
     * @param idx  Index of the element to remove, must be less than the
     *             list size.
     * @return 0 on success, or EINVAL if idx is out of range.
     */
    int (*delete)(ArrayList *self, size_t idx);

    /**
     * @brief Removes the first element.
     *
     * Remaining elements are shifted down by one, so this is O(n).
     *
     * @param self The list to remove from.
     * @return 0 on success, or ENOENT if the list is empty.
     */
    int (*delete_first)(ArrayList *self);

    /**
     * @brief Removes the last element.
     *
     * No elements are shifted, so this is O(1).
     *
     * @param self The list to remove from.
     * @return 0 on success, or ENOENT if the list is empty.
     */
    int (*delete_last)(ArrayList *self);

    /**
     * @brief Inserts a copy of an item at an index.
     *
     * Elements at and after idx are shifted up by one to make room, so this
     * is O(n). The backing storage is doubled if the list is full. On
     * failure the list is left unchanged.
     *
     * @param self The list to insert into.
     * @param idx  Index to insert at, from 0 up to and including the list
     *             size (inserting at the size appends).
     * @param item Pointer to the item to copy in, must point to one element.
     * @return 0 on success, EINVAL if item is NULL or idx is greater than
     *         the list size, or ENOMEM if the list could not grow.
     */
    int (*insert)(ArrayList *self, size_t idx, const void *item);

    /**
     * @brief Adds a copy of an item to the end of the list.
     *
     * The backing storage is doubled if the list is full, so this is
     * amortised O(1). On failure the list is left unchanged.
     *
     * @param self The list to append to.
     * @param item Pointer to the item to copy in, must point to one element.
     * @return 0 on success, EINVAL if item is NULL, or ENOMEM if the list
     *         could not grow.
     */
    int (*append)(ArrayList *self, const void *item);

    /**
     * @brief Overwrites the element at an index with a copy of an item.
     *
     * The list size does not change and no allocation happens, so this can
     * only fail on invalid arguments. Use insert() to add a new element.
     *
     * @param self The list to modify.
     * @param idx  Index of the element to replace, must be less than the
     *             list size.
     * @param item Pointer to the item to copy in, must point to one element.
     * @return 0 on success, or EINVAL if item is NULL or idx is out of
     *         range.
     */
    int (*set)(ArrayList *self, size_t idx, const void *item);

    /**
     * @brief Swaps the elements at two indices.
     *
     * Elements are exchanged byte for byte in place, so no allocation
     * happens. Swapping an index with itself is a successful no-op.
     *
     * @param self The list to modify.
     * @param i    Index of the first element, must be less than the list
     *             size.
     * @param j    Index of the second element, must be less than the list
     *             size.
     * @return 0 on success, or EINVAL if i or j is out of range.
     */
    int (*swap)(ArrayList *self, size_t i, size_t j);

    /**
     * @brief Frees a list and its storage.
     *
     * Only the list's own memory is freed. If the stored elements contain
     * pointers to other allocations, the caller must free those first. The
     * list must not be used afterwards.
     *
     * @param self The list to destroy.
     */
    void (*destroy)(ArrayList *self);
};

/**
 * @brief Creates an empty list.
 *
 * The list starts with a default capacity and grows automatically as items
 * are added. Use arrayList_create_cap() to choose the starting capacity.
 * Elements are stored by value: inserted items are copied in, byte for byte.
 * The list must be released with its destroy() method.
 *
 * @param memSize Size in bytes of one element, e.g. sizeof(int). Must be at
 *                least 1.
 * @return The new list, or NULL if memSize is 0, the initial allocation size
 *         overflows, or memory could not be allocated.
 */
ArrayList *arrayList_create(size_t memSize);

/**
 * @brief Creates an empty list with a chosen initial capacity.
 *
 * Same as arrayList_create(), but reserves room for length elements up
 * front, which avoids regrowing when the final size is roughly known.
 *
 * @param memSize Size in bytes of one element, e.g. sizeof(int). Must be at
 *                least 1.
 * @param length  Initial capacity in elements. Must be at least 1. The list
 *                grows automatically past this.
 * @return The new list, or NULL if either argument is 0, the requested size
 *         overflows, or memory could not be allocated.
 */
ArrayList *arrayList_create_cap(size_t memSize, size_t length);

#endif /* ARRAY_LIST_H_ */
