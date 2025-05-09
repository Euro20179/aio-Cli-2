#include "hashmap.h"
#include "bucket.h"
#include <stdint.h>
#include <stdlib.h>

struct _hashmap_item* _hashmap_item_create(hash_t hash, void* value) {
    struct _hashmap_item* item = malloc(sizeof(struct _hashmap_item));
    item->key_hash = hash;
    item->value = value;
    return item;
}

void _hashmap_item_destroy(struct _hashmap_item * item) {
    free(item);
}

void hashmap_new(hashmap * map) {
    bucket_new(&map->items, 16, struct _hashmap_item);
}

void hashmap_set(hashmap * map, const char *key, void *value) {
    hash_t h = hash_str(key);

    struct _hashmap_item* item = _hashmap_item_create(h, value);

    size_t idx = h % bucket_size(&map->items);

    bucket_set(&map->items, idx, item);
}

int hashmap_unset(hashmap * map, const char *key) {
    hash_t h = hash_str(key);
    size_t idx = h % bucket_size(&map->items);

    struct _hashmap_item* item = bucket_get(&map->items, idx);
    if(item == NULL) {
        return -1;
    }

    _hashmap_item_destroy(item);
    bucket_remove(&map->items, idx);

    return 0;
}

void* hashmap_get(hashmap* map, const char* key) {
    hash_t h = hash_str(key);
    size_t idx = h % bucket_size(&map->items);

    struct _hashmap_item* item = bucket_get(&map->items, idx);

    if(item == NULL) {
        return item;
    }

    return item->value;
}

hash_t hash_str(const char *str) {
    hash_t hash = 5381;
    int c;
    while(c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

