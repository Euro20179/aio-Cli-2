#pragma once

#include <stdint.h>

#include "bucket.h"

typedef uint32_t hash_t;

struct _hashmap_item {
    hash_t key_hash;
    void* value;
};

struct _hashmap_item* _hashmap_item_create(hash_t hash, void* value);
void _hashmap_item_destroy(struct _hashmap_item*);

typedef struct {
    bucket(struct _hashmap_item*) items;
} hashmap;

void hashmap_new(hashmap*);

//if item is not found, return NULL
void* hashmap_get(hashmap*, const char* key);

void hashmap_set(hashmap*, const char* key, void* value);
//returns -1 if the key is not in the map
int hashmap_unset(hashmap*, const char* key);

hash_t hash_str(const char* str);
