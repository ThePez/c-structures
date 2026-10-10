/**
 * @file stack.c
 * @brief LIFO stack. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "stack.h"
#include "arrayList.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

typedef struct StackPriv {
    Stack pub; // Must stay first
    ArrayList *data;
} StackPriv;

#define PRIVATE(s)       ((StackPriv *)(s))
#define CONST_PRIVATE(s) ((const StackPriv *)(s))

static size_t size(const Stack *self)
{
    assert(self != NULL);
    const ArrayList *data = CONST_PRIVATE(self)->data;
    return data->size(data);
}

static int empty(const Stack *self)
{
    assert(self != NULL);
    const ArrayList *data = CONST_PRIVATE(self)->data;
    return data->empty(data);
}

static int push(Stack *self, const void *item)
{
    assert(self != NULL);
    ArrayList *data = PRIVATE(self)->data;
    return data->append(data, item);
}

static int pop(Stack *self, void *dest)
{
    assert(self != NULL);
    ArrayList *data = PRIVATE(self)->data;

    int error = data->get_last_cpy(data, dest);
    if (error) {
        return error;
    }

    return data->delete_last(data);
}

static int peek(const Stack *self, void *dest)
{
    assert(self != NULL);
    const ArrayList *data = CONST_PRIVATE(self)->data;
    return data->get_last_cpy(data, dest);
}

static void destroy(Stack *self)
{
    if (!self) {
        return;
    }

    ArrayList *data = PRIVATE(self)->data;
    data->destroy(data);
    free(self);
}

Stack *stack_create(size_t memberSize)
{

    ArrayList *data = arrayList_create(memberSize);
    if (!data) {
        return NULL;
    }

    StackPriv *self = malloc(sizeof(StackPriv));
    if (self == NULL) {
        data->destroy(data);
        return NULL;
    }

    self->data = data;

    self->pub = (const Stack){
        .size = size, .destroy = destroy, .peek = peek, .pop = pop, .push = push, .empty = empty};

    return &self->pub;
}
