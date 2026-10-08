# C Data Structures

My own implementations of the standard generic data structures in C11, in the same vein as [my Java versions](https://github.com/ThePez/Data-Structures). Each structure owns the memory it uses and stores elements by value, so it works with any element type without macros or `void *` ownership headaches.

## Status

| Structure | Header | Status |
|-----------|--------|--------|
| Array list | [`arrayList.h`](include/arrayList.h) | Implemented |
| Linked list | [`linkedList.h`](include/linkedList.h) | Planned |
| Stack | [`stack.h`](include/stack.h) | Planned |
| Queue | [`queue.h`](include/queue.h) | Planned |
| Priority queue | [`prioQueue.h`](include/prioQueue.h) | Planned |
| Heap | [`heap.h`](include/heap.h) | Planned |
| Tree | [`tree.h`](include/tree.h) | Planned |
| Hash map | [`hashMap.h`](include/hashMap.h) | Planned |
| Ordered map | [`orderedMap.h`](include/orderedMap.h) | Planned |
| Bit vector | [`bitVector.h`](include/bitVector.h) | Planned |

[`list.h`](include/list.h) and [`map.h`](include/map.h) are reserved for generic interfaces shared by the list and map implementations.

## Layout

```
include/   public headers (the documented API)
src/       implementations
```

Each structure is an opaque type: the header declares the type and its functions, and the struct definition lives in the `.c` file.

## Building

There is no build system yet. Compile the sources you need alongside your program:

```bash
cc -std=c11 -Wall -Wextra -Wpedantic -Iinclude src/arrayList.c main.c -o main
```

While developing, adding `-fsanitize=address,undefined -g` catches out-of-bounds access, leaks and undefined behaviour.

## Design conventions

- **Stored by value.** Elements are copied in on insert, using the element size given at creation. Structures never take ownership of what an element points to; if you store pointers, you free what they point at.
- **Errors are return codes.** Functions that can fail return `0` on success or an `errno` value: `EINVAL` for bad arguments, `ENOMEM` when allocation fails, `ENOENT` when the structure is empty. On failure the structure is left unchanged.
- **Two ways to read.** `get` returns a `const` pointer into the structure (fast, but only valid until the next modification). `get_cpy` copies the element into a buffer you provide (safe to keep).
- **Overflow-checked sizes.** Size calculations are checked against `SIZE_MAX` before allocating.

## Array list

A growable array. Capacity doubles when full, so appends are amortised O(1).

| Operation | Functions | Cost |
|-----------|-----------|------|
| Create / destroy | `arrayList_create`, `arrayList_destroy` | O(1) |
| Read | `arrayList_get`, `_get_first`, `_get_last` | O(1) |
| Read (copy) | `arrayList_get_cpy`, `_get_first_cpy`, `_get_last_cpy` | O(1) |
| Overwrite | `arrayList_set` | O(1) |
| Add | `arrayList_append` | amortised O(1) |
| Insert | `arrayList_insert` | O(n) |
| Remove | `arrayList_delete`, `_delete_first` | O(n) |
| Remove last | `arrayList_delete_last` | O(1) |
| Count | `arrayList_size` | O(1) |

### Example

```c
#include <stdio.h>
#include "arrayList.h"

int main(void)
{
    ArrayList *list = arrayList_create(sizeof(int), 4);
    if (!list) {
        return 1;
    }

    for (int i = 1; i <= 5; i++) {
        arrayList_append(list, &i);     /* grows past the initial capacity of 4 */
    }

    int ten = 10;
    arrayList_insert(list, 0, &ten);    /* shifts everything up: 10 1 2 3 4 5 */
    arrayList_delete(list, 3);          /* removes the 3:        10 1 2 4 5   */

    for (size_t i = 0; i < arrayList_size(list); i++) {
        const int *value = arrayList_get(list, i);
        printf("%d ", *value);
    }
    printf("\n");

    int last;
    if (arrayList_get_last_cpy(list, &last) == 0) {
        printf("last = %d\n", last);
    }

    arrayList_destroy(list);
    return 0;
}
```

Full API documentation is in the header comments of [`arrayList.h`](include/arrayList.h).

## Author

Jack Cairns
