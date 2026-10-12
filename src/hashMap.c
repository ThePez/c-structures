/**
 * @file hashMap.c
 * @brief Hash map using hashing for average O(1) key lookup. (implementation)
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#include "hashMap.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdalign.h>

#define LOAD_FACTOR      0.66
#define PRIVATE(m)       ((HashMapPriv *)(m))
#define CONST_PRIVATE(m) ((const HashMapPriv *)(m))

/**
 * List of prime numbers used to determine the size of the internal table.
 */
const int primes[] = {11,        23,        47,        97,         197,       397,
                      797,       1597,      3203,      6421,       12853,     25717,
                      51437,     102877,    205759,    411527,     823117,    1646237,
                      3292489,   6584983,   13169977,  26339969,   52679969,  105359939,
                      210719881, 421439783, 842879579, 1685759167, 2147483647};

typedef enum SlotState {
    SLOT_EMPTY,
    SLOT_FULL,
    SLOT_DELETED
} SlotState;

typedef struct Slot {
    SlotState state;
    union {
        uint64_t num;
        char *str;
    } key;
    char value[];
} Slot;

typedef struct HashMapPriv {
    HashMap pub;     // must stay first
    KeyType type;    // Kind of key this map uses, fixed at creation
    int primeIndex;  // Position of capacity in the primes table, used to grow
    size_t valSize;  // Bytes in each value
    size_t slotSize; // Bytes between the start of one slot and the next
    size_t capacity; // Number of slots in the table (always a prime)
    size_t count;    // Slots holding a live key-value pair
    size_t used;     // Live slots plus tombstones, compared with capacity for the load factor
    char *table;     // Actual data buffer

    // Maps a key to a starting slot index in [0, capacity)
    size_t (*hash)(const void *key, size_t capacity);
    // Returns non-zero if the slot's key matches key
    int (*equals)(const Slot *slot, const void *key);
    // Copies key into the slot, returns 0 on success or an errno code
    int (*key_store)(Slot *slot, const void *key);
    // Frees anything the slot's key owns, does nothing for number keys
    void (*key_release)(Slot *slot);
    // Returns a pointer to the slot's key in the form hash/equals expect
    const void *(*key_ref)(const Slot *slot);
} HashMapPriv;

typedef struct Probe {
    Slot *match;
    Slot *target;
    int reused;
} Probe;

static Probe find(const HashMapPriv *map, const void *key)
{
    Probe result = {0};

    size_t capacity = map->capacity;
    char *base = map->table;
    size_t index = map->hash(key, capacity);

    for (size_t i = 0; i < capacity; i++) {
        Slot *slot = (Slot *)(base + (index * map->slotSize));
        if (slot->state == SLOT_EMPTY) {
            if (!result.target) {
                result.target = slot;
            }

            return result;
        }

        if (slot->state == SLOT_DELETED) {
            if (!result.target) {
                result.target = slot;
                result.reused = 1;
            }
        } else if (map->equals(slot, key)) {
            result.match = slot;
            return result;
        }

        if (++index == capacity) {
            index = 0;
        }
    }

    return result;
}

static int rehash(HashMapPriv *map)
{
    size_t nextIndex = (size_t)map->primeIndex + 1;
    if (nextIndex >= sizeof(primes) / sizeof(primes[0])) {
        // Already at the largest prime
        return ENOMEM;
    }

    size_t newCap = (size_t)primes[nextIndex];
    if (newCap > SIZE_MAX / map->slotSize) {
        // Overflowed -> capacity too big for this valSize
        return ENOMEM;
    }

    char *newTable = calloc(newCap, map->slotSize);
    if (!newTable) {
        return ENOMEM;
    }

    size_t oldCap = map->capacity;
    char *oldTable = map->table;

    map->table = newTable;
    map->capacity = newCap;
    map->primeIndex = nextIndex;

    for (size_t i = 0; i < oldCap; i++) {
        const Slot *slot = (const Slot *)(oldTable + (i * map->slotSize));
        if (slot->state != SLOT_FULL) {
            continue;
        }

        Probe probe = find(map, map->key_ref(slot));
        assert(probe.target != NULL);
        memcpy(probe.target, slot, map->slotSize);
    }

    map->used = map->count;
    free(oldTable);
    return 0;
}

static int insert(HashMapPriv *map, const Probe *probe, const void *key, const void *item)
{
    if (!probe->target) {
        // No Slots available
        return ENOMEM;
    }

    int error = map->key_store(probe->target, key);
    if (error) {
        return error;
    }

    probe->target->state = SLOT_FULL;
    memcpy(probe->target->value, item, map->valSize);
    map->count++;

    if (!probe->reused) {
        // Slot used has been used before
        map->used++;
    }

    if ((double)map->used / map->capacity > LOAD_FACTOR) {
        rehash(map);
    }

    return 0;
}

static inline size_t hash(uint64_t hash, size_t capacity)
{
    // Simple bit rotation and 'XOR'.
    hash *= 6321751UL; // Large prime number
    hash = (hash << 59) | (hash >> 37);
    hash ^= hash >> 32;
    hash ^= (hash << 29) | (hash >> 3);
    // Normalise the hash value to ensure it fits within the map's capacity
    return hash % capacity;
}

static const void *num_key_ref(const Slot *slot)
{
    return &slot->key.num;
}

static size_t num_hash(const void *key, size_t capacity)
{
    return hash(*(uint64_t *)key, capacity);
}

static int num_equals(const Slot *slot, const void *key)
{
    return slot->key.num == *(uint64_t *)key;
}

static int num_key_store(Slot *slot, const void *key)
{
    slot->key.num = *(const uint64_t *)key;
    return 0;
}

static void num_key_release(Slot *slot)
{
    // Function does nothing
    (void)slot;
}

static size_t string_hash(const void *key, size_t capacity)
{
    // TODO
    (void)key;
    (void)capacity;
    return 0;
}

static int string_equals(const Slot *slot, const void *key)
{
    char *string = (char *)key;
    return strcmp(string, slot->key.str) == 0;
}

static int string_key_store(Slot *slot, const void *key)
{
    slot->key.str = strdup(key);
    return (slot->key.str == NULL) ? ENOMEM : 0;
}

static void string_key_release(Slot *slot)
{
    free(slot->key.str);
}

static const void *string_key_ref(const Slot *slot)
{
    return slot->key.str;
}

static size_t size(const HashMap *self)
{
    assert(self != NULL);
    return CONST_PRIVATE(self)->count;
}

static KeyType key_type(const HashMap *self)
{
    assert(self != NULL);
    return CONST_PRIVATE(self)->type;
}

static int empty(const HashMap *self)
{
    assert(self != NULL);
    return CONST_PRIVATE(self)->count == 0;
}

static const void *get(const HashMap *self, const void *key)
{
    assert(self != NULL);
    if (!key) {
        return NULL;
    }

    const HashMapPriv *map = CONST_PRIVATE(self);
    Probe probe = find(map, key);

    if (probe.match) {
        return probe.match->value;
    }

    // Key not found in table
    return NULL;
}

static int get_cpy(const HashMap *self, const void *key, void *dest)
{
    assert(self != NULL);
    if (!key || !dest) {
        return EINVAL;
    }

    const HashMapPriv *map = CONST_PRIVATE(self);
    Probe probe = find(map, key);

    if (probe.match) {
        memcpy(dest, probe.match->value, map->valSize);
        return 0;
    }

    // Key not found in table
    return ENOENT;
}

static HashMap *erase(HashMap *self, const void *key)
{
    if (!self || self->err) {
        return self;
    }

    if (!key) {
        self->err = EINVAL;
        return self;
    }

    HashMapPriv *map = PRIVATE(self);
    Probe probe = find(map, key);

    if (probe.match) {
        probe.match->state = SLOT_DELETED;
        map->key_release(probe.match);
        map->count--;
        memset(probe.match->value, 0, map->valSize);
        return self;
    }

    // Key not found in table
    self->err = ENOENT;
    return self;
}

static HashMap *add(HashMap *self, const void *key, const void *item)
{
    if (!self || self->err) {
        return self;
    }

    if (!key || !item) {
        self->err = EINVAL;
        return self;
    }

    HashMapPriv *map = PRIVATE(self);
    Probe probe = find(map, key);

    if (probe.match) {
        self->err = EEXIST;
        return self;
    }

    self->err = insert(map, &probe, key, item);
    return self;
}

static HashMap *set(HashMap *self, const void *key, const void *item, void *old)
{
    if (!self || self->err) {
        return self;
    }

    if (!key || !item) {
        self->err = EINVAL;
        return self;
    }

    HashMapPriv *map = PRIVATE(self);
    Probe probe = find(map, key);

    if (probe.match) {
        // Copy old value
        if (old) {
            memcpy(old, probe.match->value, map->valSize);
        }

        memcpy(probe.match->value, item, map->valSize);
        return self;
    }

    self->err = insert(map, &probe, key, item);
    return self;
}

static void destroy(HashMap *self)
{
    assert(self != NULL);
    HashMapPriv *map = PRIVATE(self);

    // Free the keys is slot is used and keys are string
    for (size_t i = 0; i < map->capacity; i++) {
        Slot *slot = (Slot *)(map->table + (i * map->slotSize));
        if (slot->state == SLOT_FULL) {
            map->key_release(slot);
        }
    }

    // Free the rest
    free(map->table);
    free(map);
}

HashMap *hashMap_create(size_t valSize, KeyType keyType)
{
    if (valSize < 1 || (keyType != KEY_NUMBER && keyType != KEY_STRING)) {
        return NULL;
    }

    HashMapPriv *hashMap = malloc(sizeof(HashMapPriv));
    if (!hashMap) {
        return NULL;
    }

    hashMap->primeIndex = 0;
    hashMap->used = 0;
    hashMap->valSize = valSize;
    hashMap->count = 0;
    hashMap->capacity = primes[hashMap->primeIndex];
    hashMap->type = keyType;

    if (valSize > SIZE_MAX - offsetof(Slot, value) - (alignof(Slot) - 1)) {
        // Overflowed -> value size too big
        free(hashMap);
        return NULL;
    }

    size_t rawSize = offsetof(Slot, value) + valSize;
    hashMap->slotSize = (rawSize + alignof(Slot) - 1) & ~(alignof(Slot) - 1);
    if (hashMap->capacity > SIZE_MAX / hashMap->slotSize) {
        // Overflowed -> capacity too big for this valSize
        free(hashMap);
        return NULL;
    }

    char *table = calloc(hashMap->capacity, hashMap->slotSize);
    if (!table) {
        free(hashMap);
        return NULL;
    }

    hashMap->table = table;

    switch (keyType) {
    case KEY_NUMBER:
        hashMap->key_ref = num_key_ref;
        hashMap->key_release = num_key_release;
        hashMap->key_store = num_key_store;
        hashMap->equals = num_equals;
        hashMap->hash = num_hash;
        break;
    case KEY_STRING:
        hashMap->key_ref = string_key_ref;
        hashMap->key_release = string_key_release;
        hashMap->key_store = string_key_store;
        hashMap->equals = string_equals;
        hashMap->hash = string_hash;
        break;
    }

    hashMap->pub = (const HashMap){.size = size,
                                   .empty = empty,
                                   .get = get,
                                   .get_cpy = get_cpy,
                                   .erase = erase,
                                   .set = set,
                                   .destroy = destroy,
                                   .add = add,
                                   .type = key_type};

    return &hashMap->pub;
}
