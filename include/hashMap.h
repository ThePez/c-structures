/**
 * @file hashMap.h
 * @brief Hash map using hashing for average O(1) key lookup. (interface)
 *
 * Maps are created with hashMap_create() and used through the function
 * pointers stored in the struct, e.g. m->add(m, HASHMAP_NUM_KEY(7), &value).
 * The first argument is always the map itself.
 *
 * Keys are either numbers or strings, fixed when the map is created. Number
 * keys must use the HASHMAP_NUM_KEY macro, string keys are passed as plain
 * NULL-terminated strings. The map keeps its own copy of every key, so the
 * caller's key does not need to outlive the call. Passing the wrong kind of
 * key for the map's KeyType is undefined behaviour.
 *
 * Values are stored by value: items are copied in, byte for byte.
 *
 * @author Jack Cairns
 * @date 2026-10-06
 */

#ifndef HASH_MAP_H_
#define HASH_MAP_H_

#include <stdint.h>
#include <stddef.h>

typedef struct HashMap HashMap;

/** @brief The kind of key a map uses. */
typedef enum {
    KEY_NUMBER, /**< Keys are numbers, passed with HASHMAP_NUM_KEY. */
    KEY_STRING, /**< Keys are NUL-terminated strings. */
} KeyType;

/**
 * @brief Public face of a hash map: its methods.
 *
 * The remaining fields are private to hashMap.c. Only hashMap_create() can
 * make a valid map, so never declare or copy a HashMap by value.
 */
struct HashMap {
    /**
     * @brief Gets the number of key-value pairs currently in the map.
     *
     * @param self The map to query.
     * @return The pair count.
     */
    size_t (*size)(const HashMap *self);

    /**
     * @brief Gets the kind of key the map was created with.
     *
     * @param self The map to query.
     * @return KEY_NUMBER or KEY_STRING.
     */
    KeyType (*type)(const HashMap *self);

    /**
     * @brief Checks whether the map holds no pairs.
     *
     * @param self The map to query.
     * @return 1 if the map is empty, 0 if it has at least one pair.
     */
    int (*empty)(const HashMap *self);

    /**
     * @brief Looks up the value stored for a key.
     *
     * Average O(1). The returned pointer refers to storage inside the map, so
     * it is only valid until the next add(), set() or delete() on the map.
     * Use get_cpy() to keep the value.
     *
     * @param self The map to read from.
     * @param key  The key to look up.
     * @return A read-only pointer to the value, or NULL if key is NULL or
     *         not in the map.
     */
    const void *(*get)(const HashMap *self, const void *key);

    /**
     * @brief Copies the value stored for a key into a caller-supplied buffer.
     *
     * Average O(1). On failure dest is left unchanged.
     *
     * @param self The map to read from.
     * @param key  The key to look up.
     * @param dest Buffer to copy the value into, must hold at least one value
     *             (the value size given to hashMap_create()).
     * @return 0 on success, EINVAL if key or dest is NULL, or ENOENT if the
     *         key is not in the map.
     */
    int (*get_cpy)(const HashMap *self, const void *key, void *dest);

    /**
     * @brief Removes a key and its value from the map.
     *
     * Average O(1). Any copy of the key the map made is freed. If the stored
     * value contains pointers to other allocations, the caller must free
     * those first, for example by reading the value with get_cpy().
     *
     * @param self The map to remove from.
     * @param key  The key to remove.
     * @return 0 on success, EINVAL if key is NULL, or ENOENT if the key is
     *         not in the map.
     */
    int (*delete)(HashMap *self, const void *key);

    /**
     * @brief Inserts a new key-value pair, refusing to replace an existing one.
     *
     * Average O(1), amortised over table growth. On failure the map is left
     * unchanged. Use set() to insert or replace.
     *
     * @param self The map to add to.
     * @param key  The key to insert.
     * @param item Pointer to the value to copy in, must point to one value.
     * @return 0 on success, EINVAL if key or item is NULL, EEXIST if the key
     *         is already in the map, or ENOMEM if memory could not be
     *         allocated.
     */
    int (*add)(HashMap *self, const void *key, const void *item);

    /**
     * @brief Inserts a key-value pair, replacing the value if the key exists.
     *
     * Average O(1), amortised over table growth. On failure the map is left
     * unchanged.
     *
     * @param self The map to write to.
     * @param key  The key to insert or update.
     * @param item Pointer to the value to copy in, must point to one value.
     * @param old  Optional buffer that receives the previous value if the key
     *             was already present, must hold at least one value. May be
     *             NULL if the previous value is not needed. Left unchanged if
     *             the key was not present.
     * @return 0 on success (whether the key was inserted or replaced), EINVAL
     *         if key or item is NULL, or ENOMEM if memory could not be
     *         allocated.
     */
    int (*set)(HashMap *self, const void *key, const void *item, void *old);

    /**
     * @brief Frees a map, its keys and its storage.
     *
     * If the stored values contain pointers to other allocations, the caller
     * must free those first. The map must not be used afterwards.
     *
     * @param self The map to destroy.
     */
    void (*destroy)(HashMap *self);
};

/**
 * @brief Wraps a number as a key argument for a KEY_NUMBER map.
 *
 * Every number key passed to a map method must go through this macro, e.g.
 * m->get(m, HASHMAP_NUM_KEY(42)).
 */
#define HASHMAP_NUM_KEY(k) (&(uint64_t){(uint64_t)(k)})

/**
 * @brief Creates an empty hash map.
 *
 * The map grows automatically as pairs are added. It must be released with
 * its destroy() method.
 *
 * @param valSize Size in bytes of one value, e.g. sizeof(int). Must be at
 *                least 1.
 * @param keyType KEY_NUMBER or KEY_STRING.
 * @return The new map, or NULL if valSize is 0 or too large, keyType is not
 *         a valid KeyType, or memory could not be allocated.
 */
HashMap *hashMap_create(size_t valSize, KeyType keyType);

#endif /* HASH_MAP_H_ */
