/**
 * @file linkedList.h
 * @brief Linked list of dynamically allocated nodes. (interface)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include <stddef.h>

typedef struct LinkedList LinkedList;

/**
 * @brief Gets the number of elements currently stored in the list.
 *
 * @param list The list to query.
 * @return The element count, or 0 if list is NULL.
 */
size_t linkedList_size(const LinkedList *list);

/**
 * @brief Gets a read-only pointer to the element at an index.
 *
 * The pointer refers to the node's own storage and stays valid until that
 * element is deleted or the list is destroyed. Inserting other elements does
 * not move it. Use linkedList_get_cpy() for a copy that stays valid.
 *
 * Nodes are walked from whichever end is closer, so this is O(n).
 *
 * @param list The list to read from.
 * @param idx  Index of the element, must be less than the list size.
 * @return Pointer to the element, or NULL if list is NULL or idx is
 *         out of range.
 */
const void *linkedList_get(const LinkedList *list, size_t idx);

/**
 * @brief Gets a read-only pointer to the first element.
 *
 * The same validity rules as linkedList_get() apply. This is O(1).
 *
 * @param list The list to read from.
 * @return Pointer to the first element, or NULL if list is NULL or empty.
 */
const void *linkedList_get_first(const LinkedList *list);

/**
 * @brief Gets a read-only pointer to the last element.
 *
 * The same validity rules as linkedList_get() apply. This is O(1).
 *
 * @param list The list to read from.
 * @return Pointer to the last element, or NULL if list is NULL or empty.
 */
const void *linkedList_get_last(const LinkedList *list);

/**
 * @brief Copies the element at an index into a caller-supplied buffer.
 *
 * @param list The list to read from.
 * @param idx  Index of the element, must be less than the list size.
 * @param dest Buffer to copy into, must hold at least one element
 *             (the element size given to linkedList_create()).
 * @return 0 on success, or EINVAL if list or dest is NULL or idx is
 *         out of range.
 */
int linkedList_get_cpy(const LinkedList *list, size_t idx, void *dest);

/**
 * @brief Copies the first element into a caller-supplied buffer.
 *
 * @param list The list to read from.
 * @param dest Buffer to copy into, must hold at least one element.
 * @return 0 on success, EINVAL if list or dest is NULL, or ENOENT if
 *         the list is empty.
 */
int linkedList_get_first_cpy(const LinkedList *list, void *dest);

/**
 * @brief Copies the last element into a caller-supplied buffer.
 *
 * @param list The list to read from.
 * @param dest Buffer to copy into, must hold at least one element.
 * @return 0 on success, EINVAL if list or dest is NULL, or ENOENT if
 *         the list is empty.
 */
int linkedList_get_last_cpy(const LinkedList *list, void *dest);

/**
 * @brief Removes the element at an index.
 *
 * The node is unlinked and freed; no other elements move. Finding the node
 * is O(n), though the first and last elements are O(1). Pointers previously
 * returned for the removed element become invalid.
 *
 * @param list The list to remove from.
 * @param idx  Index of the element to remove, must be less than the list size.
 * @return 0 on success, or EINVAL if list is NULL or idx is out of
 *         range.
 */
int linkedList_delete(LinkedList *list, size_t idx);

/**
 * @brief Removes the first element.
 *
 * This is O(1). Pointers previously returned for the removed element become
 * invalid.
 *
 * @param list The list to remove from.
 * @return 0 on success, EINVAL if list is NULL, or ENOENT if the list is
 *         empty.
 */
int linkedList_delete_first(LinkedList *list);

/**
 * @brief Removes the last element.
 *
 * This is O(1). Pointers previously returned for the removed element become
 * invalid.
 *
 * @param list The list to remove from.
 * @return 0 on success, EINVAL if list is NULL, or ENOENT if the list is
 *         empty.
 */
int linkedList_delete_last(LinkedList *list);

/**
 * @brief Inserts a copy of an item at an index.
 *
 * Existing elements at and after idx move up by one position, but they are
 * not copied or relocated in memory. Inserting at the front or back is O(1);
 * elsewhere, finding the position is O(n). On failure the list is left
 * unchanged.
 *
 * @param list The list to insert into.
 * @param idx  Index to insert at, from 0 up to and including the list size
 *             (inserting at the size appends).
 * @param item Pointer to the item to copy in, must point to one element.
 * @return 0 on success, EINVAL if list or item is NULL or idx is
 *         greater than the list size, or ENOMEM if the node could not be
 *         allocated.
 */
int linkedList_insert(LinkedList *list, size_t idx, const void *item);

/**
 * @brief Adds a copy of an item to the end of the list.
 *
 * This is O(1). On failure the list is left unchanged.
 *
 * @param list The list to append to.
 * @param item Pointer to the item to copy in, must point to one element.
 * @return 0 on success, EINVAL if list or item is NULL, or ENOMEM if
 *         the node could not be allocated.
 */
int linkedList_append(LinkedList *list, const void *item);

/**
 * @brief Overwrites the element at an index with a copy of an item.
 *
 * The list size does not change and no allocation happens, so this can only
 * fail on invalid arguments. Use linkedList_insert() to add a new element.
 *
 * @param list The list to modify.
 * @param idx  Index of the element to replace, must be less than the list
 *             size.
 * @param item Pointer to the item to copy in, must point to one element.
 * @return 0 on success, or EINVAL if list or item is NULL or idx is
 *         out of range.
 */
int linkedList_set(LinkedList *list, size_t idx, const void *item);

/**
 * @brief Creates an empty list.
 *
 * Elements are stored by value: inserted items are copied in, byte for byte.
 * The list must be released with linkedList_destroy().
 *
 * @param memSize Size in bytes of one element, e.g. sizeof(int). Must be at
 *                least 1.
 * @return The new list, or NULL if memSize is 0, the size is too large to
 *         allocate a node for, or memory could not be allocated.
 */
LinkedList *linkedList_create(size_t memSize);

/**
 * @brief Frees a list and all of its nodes.
 *
 * Only the list's own memory is freed. If the stored elements contain
 * pointers to other allocations, the caller must free those first. Passing
 * NULL is a no-op. The list must not be used afterwards.
 *
 * @param list The list to destroy.
 */
void linkedList_destroy(LinkedList *list);

#endif /* LINKED_LIST_H_ */
