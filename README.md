# C Data Structures

My own implementations of the standard generic data structures in C2x, in the same vein as [my Java versions](https://github.com/ThePez/Data-Structures). Each structure owns the memory it uses and stores elements by value, so it works with any element type without macros or `void *` ownership headaches.

## Status

| Structure | Header | Status |
|-----------|--------|--------|
| Array list | [`arrayList.h`](include/arrayList.h) | Implemented |
| Linked list | [`linkedList.h`](include/linkedList.h) | Implemented |
| Stack | [`stack.h`](include/stack.h) | Implemented |
| Queue | [`queue.h`](include/queue.h) | Implemented |
| Priority queue | [`prioQueue.h`](include/prioQueue.h) | Implemented |
| Heap | [`heap.h`](include/heap.h) | Implemented |
| Tree | [`tree.h`](include/tree.h) | Planned |
| Hash map | [`hashMap.h`](include/hashMap.h) | Implemented (number keys; string key hashing in progress) |
| Ordered map | [`orderedMap.h`](include/orderedMap.h) | Planned |
| Bit vector | [`bitVector.h`](include/bitVector.h) | Planned |

[`list.h`](include/list.h) and [`map.h`](include/map.h) are reserved for generic interfaces shared by the list and map implementations.

## Layout

```
include/   public headers (the documented API)
src/       implementations
```

Each structure is used through a struct of function pointers: the header defines the public struct (its methods only), and the real fields live in a private struct in the `.c` file. See [Using the structures](#using-the-structures).

## Building

There is no build system yet. Compile the sources you need alongside your program:

```bash
cc -std=c2x -Wall -Wextra -Wpedantic -Iinclude src/arrayList.c main.c -o main
```

Structures that wrap another one need its source too, for example a stack needs `src/stack.c src/arrayList.c`, a queue needs `src/queue.c src/linkedList.c`, and a priority queue needs `src/prioQueue.c src/heap.c src/arrayList.c`. The hash map is standalone and needs only `src/hashMap.c`.

While developing, adding `-fsanitize=address,undefined -g` catches out-of-bounds access, leaks and undefined behaviour.

## Design conventions

- **Stored by value.** Elements are copied in on insert, using the element size given at creation. Structures never take ownership of what an element points to; if you store pointers, you free what they point at.
- **Errors are return codes.** Functions that can fail return `0` on success or an `errno` value: `EINVAL` for bad arguments, `ENOMEM` when allocation fails, `ENOENT` when the structure is empty or a map key is missing, and `EEXIST` when a map's `add` finds the key already present. On failure the structure is left unchanged.
- **Methods take the object first.** C has no implicit `this`, so every method takes the structure as its first argument: `list->append(list, &item)`.
- **Two ways to read.** `get` returns a `const` pointer into the structure (fast, but only valid until the next modification). `get_cpy` copies the element into a buffer you provide (safe to keep).
- **Overflow-checked sizes.** Size calculations are checked against `SIZE_MAX` before allocating.

## Using the structures

Create a structure with its `*_create()` function, then call methods through the returned pointer. The first argument is always the structure itself. Each method's documentation is on its member in the header, so hovering `list->get` in an editor with clangd shows it.

```c
ArrayList *list = arrayList_create(sizeof(int));
int x = 5;
list->append(list, &x);
const int *p = list->get(list, 0);
list->destroy(list);
```

Only the `*_create()` functions are public free functions. Everything else is a method, and the implementations are `static` in the `.c` files. Structures never need to be declared or copied by value; always go through the pointer from `create`.

### How it works

The public struct holds only function pointers. The private struct in the `.c` file has the public one as its **first member**, followed by the real fields, so a pointer to one is also a pointer to the other and the methods cast back to reach the private state:

```c
typedef struct ArrayListPriv {
    ArrayList pub;              /* must stay first */
    size_t capacity, size, elementSize;
    char *data;
} ArrayListPriv;
```

### Methods

| Structure | Methods | Created with |
|-----------|---------|--------------|
| Array list | `size`, `empty`, `get`, `get_first`, `get_last`, `get_cpy`, `get_first_cpy`, `get_last_cpy`, `delete`, `delete_first`, `delete_last`, `insert`, `append`, `set`, `swap`, `destroy` | `arrayList_create(memSize)`, `arrayList_create_cap(memSize, length)` |
| Linked list | `size`, `empty`, `get`, `get_first`, `get_last`, `get_cpy`, `get_first_cpy`, `get_last_cpy`, `delete`, `delete_first`, `delete_last`, `insert`, `append`, `set`, `destroy` | `linkedList_create(memSize)`, `linkedList_create_cap(memSize, length)` |
| Stack | `size`, `empty`, `push`, `pop`, `peek`, `destroy` | `stack_create(memberSize)` |
| Queue | `size`, `empty`, `enqueue`, `dequeue`, `peek`, `destroy` | `queue_create(memberSize)` |
| Priority queue | `size`, `empty`, `enqueue`, `dequeue`, `peek`, `destroy` | `prioQueue_create(memberSize, type, cmp)` |
| Heap | `size`, `empty`, `add`, `peek`, `pop`, `destroy` | `heap_create(memSize, type, cmp)`, `heap_from_list(list, memSize, type, cmp)` |
| Hash map | `size`, `type`, `empty`, `get`, `get_cpy`, `delete`, `add`, `set`, `destroy` | `hashMap_create(valSize, keyType)` |

Both lists have two constructors so they can be created with the same call. `*_create(memSize)` uses defaults, and `*_create_cap(memSize, length)` takes an initial capacity. A linked list has no capacity, so its `_cap` version ignores `length`.

The `empty` method returns `1` if there are no elements and `0` otherwise. The method names and their errors match across structures, so the array list and linked list are interchangeable.

## Array list

A growable array. Capacity doubles when full, so appends are amortised O(1).

| Operation | Methods | Cost |
|-----------|---------|------|
| Read | `get`, `get_first`, `get_last` | O(1) |
| Read (copy) | `get_cpy`, `get_first_cpy`, `get_last_cpy` | O(1) |
| Overwrite | `set` | O(1) |
| Swap | `swap` | O(1) |
| Add | `append` | amortised O(1) |
| Insert | `insert` | O(n) |
| Remove | `delete`, `delete_first` | O(n) |
| Remove last | `delete_last` | O(1) |
| Count | `size`, `empty` | O(1) |

### Example

```c
#include <stdio.h>
#include "arrayList.h"

int main(void)
{
    ArrayList *list = arrayList_create(sizeof(int));
    if (!list) {
        return 1;
    }

    for (int i = 1; i <= 5; i++) {
        list->append(list, &i);         /* grows past the initial capacity */
    }

    int ten = 10;
    list->insert(list, 0, &ten);        /* shifts everything up: 10 1 2 3 4 5 */
    list->delete(list, 3);              /* removes the 3:        10 1 2 4 5   */

    for (size_t i = 0; i < list->size(list); i++) {
        const int *value = list->get(list, i);
        printf("%d ", *value);
    }
    printf("\n");

    int last;
    if (list->get_last_cpy(list, &last) == 0) {
        printf("last = %d\n", last);
    }

    list->destroy(list);
    return 0;
}
```

Full API documentation is in the header comments of [`arrayList.h`](include/arrayList.h).

## Linked list

A doubly linked list with each element stored in its own node, so inserting and removing never moves other elements. The first and last elements are O(1) to read, add and remove; anything in the middle is O(n), walking from whichever end is closer. Pointers from `get` stay valid until that element is deleted. See [`linkedList.h`](include/linkedList.h).

```c
#include <stdio.h>
#include "linkedList.h"

int main(void)
{
    LinkedList *list = linkedList_create(sizeof(int));
    if (!list) {
        return 1;
    }

    for (int i = 1; i <= 3; i++) {
        list->append(list, &i);         /* 1 2 3 */
    }

    int zero = 0;
    list->insert(list, 0, &zero);       /* 0 1 2 3 */
    list->delete_last(list);            /* 0 1 2   */

    for (size_t i = 0; i < list->size(list); i++) {
        const int *value = list->get(list, i);
        printf("%d ", *value);
    }
    printf("\n");

    int last;
    if (list->get_last_cpy(list, &last) == 0) {
        printf("last = %d\n", last);
    }

    list->destroy(list);
    return 0;
}
```

## Stack, queue, priority queue and heap

These are thin wrappers over the lists and the heap.

### Stack

LIFO, backed by an array list. `push`, `pop` and `peek` are O(1) (push is amortised). See [`stack.h`](include/stack.h).

```c
#include <stdio.h>
#include "stack.h"

int main(void)
{
    Stack *s = stack_create(sizeof(int));
    if (!s) {
        return 1;
    }

    for (int i = 1; i <= 3; i++) {
        s->push(s, &i);
    }

    int top;
    if (s->peek(s, &top) == 0) {
        printf("top = %d\n", top);      /* 3, still on the stack */
    }

    while (!s->empty(s)) {
        s->pop(s, &top);
        printf("%d ", top);             /* 3 2 1 */
    }
    printf("\n");

    s->destroy(s);
    return 0;
}
```

### Queue

FIFO, backed by a linked list. `enqueue`, `dequeue` and `peek` are O(1). See [`queue.h`](include/queue.h).

```c
#include <stdio.h>
#include "queue.h"

int main(void)
{
    Queue *q = queue_create(sizeof(int));
    if (!q) {
        return 1;
    }

    for (int i = 1; i <= 3; i++) {
        q->enqueue(q, &i);
    }

    int front;
    if (q->peek(q, &front) == 0) {
        printf("front = %d\n", front);  /* 1, still in the queue */
    }

    while (!q->empty(q)) {
        q->dequeue(q, &front);
        printf("%d ", front);           /* 1 2 3 */
    }
    printf("\n");

    q->destroy(q);
    return 0;
}
```

### Heap

A binary min or max heap backed by an array list, ordered by a comparison function you supply. `add` and `pop` are O(log n), `peek` is O(1), and `heap_from_list` builds one from an existing array list in O(n). See [`heap.h`](include/heap.h).

```c
#include <stdio.h>
#include "heap.h"

static int cmp_int(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

int main(void)
{
    Heap *h = heap_create(sizeof(int), HEAP_MIN, cmp_int);
    if (!h) {
        return 1;
    }

    int values[] = {5, 1, 3, 4, 2};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        h->add(h, &values[i]);
    }

    int top;
    if (h->peek(h, &top) == 0) {
        printf("smallest = %d\n", top); /* 1, still in the heap */
    }

    while (!h->empty(h)) {
        h->pop(h, &top);
        printf("%d ", top);             /* 1 2 3 4 5 */
    }
    printf("\n");

    h->destroy(h);
    return 0;
}
```

### Priority queue

A queue interface over a heap; `dequeue` returns the highest-priority element first. See [`prioQueue.h`](include/prioQueue.h).

```c
#include <stdio.h>
#include "prioQueue.h"

static int cmp_int(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

int main(void)
{
    PrioQueue *pq = prioQueue_create(sizeof(int), HEAP_MAX, cmp_int);
    if (!pq) {
        return 1;
    }

    int values[] = {5, 1, 3, 4, 2};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        pq->enqueue(pq, &values[i]);
    }

    int next;
    if (pq->peek(pq, &next) == 0) {
        printf("highest = %d\n", next); /* 5, still in the queue */
    }

    while (!pq->empty(pq)) {
        pq->dequeue(pq, &next);
        printf("%d ", next);            /* 5 4 3 2 1 */
    }
    printf("\n");

    pq->destroy(pq);
    return 0;
}
```

## Hash map

A hash table using open addressing with linear probing, prime-sized tables and tombstones for deletes. It grows automatically once the table is about two-thirds full, so lookups, inserts and deletes are average O(1). See [`hashMap.h`](include/hashMap.h).

- Keys are numbers or strings, chosen at creation. Number keys are wrapped with `HASHMAP_NUM_KEY`. The map keeps its own copy of every key.
- `add` only inserts, and returns `EEXIST` if the key is already there. `set` inserts or replaces, and can hand back the previous value through its optional `old` argument.
- `get` returns a pointer into the table that is invalidated by the next `add`, `set` or `delete`. Use `get_cpy` to keep a value.
- String key hashing is not finished yet, so use `KEY_NUMBER` for now.

```c
#include <stdio.h>
#include <errno.h>
#include "hashMap.h"

int main(void)
{
    HashMap *m = hashMap_create(sizeof(int), KEY_NUMBER);
    if (!m) {
        return 1;
    }

    int score = 90;
    m->add(m, HASHMAP_NUM_KEY(1), &score);
    if (m->add(m, HASHMAP_NUM_KEY(1), &score) == EEXIST) {
        printf("key 1 already present\n");      /* add never replaces */
    }

    int better = 95, old;
    m->set(m, HASHMAP_NUM_KEY(1), &better, &old);   /* set replaces */
    printf("old = %d\n", old);                      /* 90 */

    const int *now = m->get(m, HASHMAP_NUM_KEY(1));
    if (now) {
        printf("now = %d\n", *now);                 /* 95 */
    }

    int copy;
    if (m->get_cpy(m, HASHMAP_NUM_KEY(2), &copy) == ENOENT) {
        printf("key 2 not found\n");
    }

    m->delete(m, HASHMAP_NUM_KEY(1));
    printf("size = %zu\n", m->size(m));             /* 0 */

    m->destroy(m);
    return 0;
}
```

## Author

Jack Cairns
