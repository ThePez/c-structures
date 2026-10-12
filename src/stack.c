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

/**
 * @brief Moves an error from the inner ArrayList onto the wrapper.
 *
 * Clears the inner error so the next operation starts clean.
 */
static Stack *sync_err(Stack *self, ArrayList *data)
{
    self->err = data->err;
    data->err = 0;
    return self;
}

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

static Stack *push(Stack *self, const void *item)
{
    if (!self || self->err) {
        return self;
    }

    ArrayList *data = PRIVATE(self)->data;
    data->append(data, item);
    return sync_err(self, data);
}

static Stack *pop(Stack *self, void *dest)
{
    if (!self || self->err) {
        return self;
    }

    ArrayList *data = PRIVATE(self)->data;

    int error = data->get_last_cpy(data, dest);
    if (error) {
        self->err = error;
        return self;
    }

    data->erase_last(data);
    return sync_err(self, data);
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
