/**
 * @file stack.h
 * @brief LIFO stack. (interface)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef STACK_H_
#define STACK_H_

#include <stddef.h>

typedef struct Stack Stack;

/**
 * @brief Gets the number of elements currently on the stack.
 *
 * @param stack The stack to query.
 * @return The element count, or 0 if stack is NULL.
 */
size_t stack_size(const Stack *stack);

/**
 * @brief Pushes a copy of an item onto the top of the stack.
 *
 * The backing storage grows as needed, so this is amortised O(1). On failure
 * the stack is left unchanged.
 *
 * @param stack The stack to push onto.
 * @param item  Pointer to the item to copy in, must point to one element
 *              (the element size given to stack_create()).
 * @return 0 on success, EINVAL if stack or item is NULL, or ENOMEM if the
 *         stack could not grow.
 */
int stack_push(Stack *stack, const void *item);

/**
 * @brief Removes the top element, copying it into a caller-supplied buffer.
 *
 * This is O(1). On failure the stack is left unchanged.
 *
 * @param stack The stack to pop from.
 * @param dest  Buffer to copy the element into, must hold at least one
 *              element.
 * @return 0 on success, EINVAL if stack or dest is NULL, or ENOENT if
 *         the stack is empty.
 */
int stack_pop(Stack *stack, void *dest);

/**
 * @brief Copies the top element into a caller-supplied buffer without
 *        removing it.
 *
 * @param stack The stack to read from.
 * @param dest  Buffer to copy the element into, must hold at least one
 *              element.
 * @return 0 on success, EINVAL if stack or dest is NULL, or ENOENT if
 *         the stack is empty.
 */
int stack_peek(const Stack *stack, void *dest);

/**
 * @brief Creates an empty stack.
 *
 * Elements are stored by value: pushed items are copied in, byte for byte.
 * The stack must be released with stack_destroy().
 *
 * @param memberSize Size in bytes of one element, e.g. sizeof(int). Must be
 *                   at least 1.
 * @return The new stack, or NULL if memberSize is 0 or memory could not be
 *         allocated.
 */
Stack *stack_create(size_t memberSize);

/**
 * @brief Frees a stack and its storage.
 *
 * Only the stack's own memory is freed. If the stored elements contain
 * pointers to other allocations, the caller must free those first. Passing
 * NULL is a no-op. The stack must not be used afterwards.
 *
 * @param stack The stack to destroy.
 */
void stack_destroy(Stack *stack);

#endif /* STACK_H_ */
