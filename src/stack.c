/**
 * @file stack.c
 * @brief LIFO stack. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "stack.h"
#include "arrayList.h"

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

struct Stack {
    ArrayList *data;
};

size_t stack_size(const Stack *stack)
{
    if (!stack) {
        return 0;
    }

    return arrayList_size(stack->data);
}

int stack_push(Stack *stack, const void *item)
{
    if (!stack) {
        return EINVAL;
    }

    return arrayList_append(stack->data, item);
}

int stack_pop(Stack *stack, void *dest)
{
    if (!stack) {
        return EINVAL;
    }

    int error = arrayList_get_last_cpy(stack->data, dest);
    if (error) {
        return error;
    }

    return arrayList_delete_last(stack->data);
}

int stack_peek(const Stack *stack, void *dest)
{
    if (!stack) {
        return EINVAL;
    }

    return arrayList_get_last_cpy(stack->data, dest);
}

void stack_destroy(Stack *stack)
{
    if (!stack) {
        return;
    }

    arrayList_destroy(stack->data);
    free(stack);
}

Stack *stack_create(size_t memberSize)
{

    ArrayList *data = arrayList_create(memberSize, 10);
    if (!data) {
        return NULL;
    }

    Stack *stack = malloc(sizeof(Stack));
    if (stack == NULL) {
        arrayList_destroy(data);
        return NULL;
    }

    stack->data = data;
    return stack;
}
