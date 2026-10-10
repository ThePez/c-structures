/**
 * @file stack.h
 * @brief LIFO stack. (interface)
 *
 * Stacks are created with stack_create() and used through the function
 * pointers stored in the struct, e.g. s->push(s, &item). The first argument
 * is always the stack itself.
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef STACK_H_
#define STACK_H_

#include <stddef.h>

typedef struct Stack Stack;

/**
 * @brief Public face of a stack: its methods.
 *
 * The remaining fields are private to stack.c. Only stack_create() can make
 * a valid stack, so never declare or copy a Stack by value.
 */
struct Stack {
    /**
     * @brief Gets the number of elements currently on the stack.
     *
     * @param self The stack to query.
     * @return The element count.
     */
    size_t (*size)(const Stack *self);

    /**
     * @brief Checks whether the stack holds no elements.
     *
     * @param self The stack to query.
     * @return 1 if the stack is empty, 0 if it has at least one element.
     */
    int (*empty)(const Stack *self);

    /**
     * @brief Pushes a copy of an item onto the top of the stack.
     *
     * The backing storage grows as needed, so this is amortised O(1). On
     * failure the stack is left unchanged.
     *
     * @param self The stack to push onto.
     * @param item Pointer to the item to copy in, must point to one element
     *             (the element size given to stack_create()).
     * @return 0 on success, EINVAL if item is NULL, or ENOMEM if the stack
     *         could not grow.
     */
    int (*push)(Stack *self, const void *item);

    /**
     * @brief Removes the top element, copying it into a caller-supplied
     *        buffer.
     *
     * This is O(1). On failure the stack is left unchanged.
     *
     * @param self The stack to pop from.
     * @param dest Buffer to copy the element into, must hold at least one
     *             element.
     * @return 0 on success, EINVAL if dest is NULL, or ENOENT if the stack
     *         is empty.
     */
    int (*pop)(Stack *self, void *dest);

    /**
     * @brief Copies the top element into a caller-supplied buffer without
     *        removing it.
     *
     * @param self The stack to read from.
     * @param dest Buffer to copy the element into, must hold at least one
     *             element.
     * @return 0 on success, EINVAL if dest is NULL, or ENOENT if the stack
     *         is empty.
     */
    int (*peek)(const Stack *self, void *dest);

    /**
     * @brief Frees a stack and its storage.
     *
     * Only the stack's own memory is freed. If the stored elements contain
     * pointers to other allocations, the caller must free those first. The
     * stack must not be used afterwards.
     *
     * @param self The stack to destroy.
     */
    void (*destroy)(Stack *self);
};

/**
 * @brief Creates an empty stack.
 *
 * Elements are stored by value: pushed items are copied in, byte for byte.
 * The stack must be released with its destroy() method.
 *
 * @param memberSize Size in bytes of one element, e.g. sizeof(int). Must be
 *                   at least 1.
 * @return The new stack, or NULL if memberSize is 0 or memory could not be
 *         allocated.
 */
Stack *stack_create(size_t memberSize);

#endif /* STACK_H_ */
