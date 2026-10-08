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

size_t linkedList_size(const LinkedList *list);

const void *linkedList_get(const LinkedList *list, size_t idx);
const void *linkedList_get_first(const LinkedList *list);
const void *linkedList_get_last(const LinkedList *list);

int linkedList_get_cpy(const LinkedList *list, size_t idx, void *dest);
int linkedList_get_first_cpy(const LinkedList *list, void *dest);
int linkedList_get_last_cpy(const LinkedList *list, void *dest);

int linkedList_delete(LinkedList *list, size_t idx);
int linkedList_delete_first(LinkedList *list);
int linkedList_delete_last(LinkedList *list);

int linkedList_insert(LinkedList *list, size_t idx, const void *item);
int linkedList_append(LinkedList *list, const void *item);
int linkedList_set(LinkedList *list, size_t idx, const void *item);

LinkedList *linkedList_create(size_t memSize);
void linkedList_destroy(LinkedList *list);

#endif /* LINKED_LIST_H_ */
