#pragma once

#include <stdint.h>

#include "bucket.h"
#include "llist.h"

typedef uint32_t hash_t;

struct _hashmap_item {
    hash_t key_hash;
    void* value;
};

struct _hashmap_item* _hashmap_item_create(hash_t hash, void* value);
void _hashmap_item_destroy(struct _hashmap_item*);

typedef struct {
    bucket(llist(struct _hashmap_item*)) items;

    size_t item_count;
} hashmap;

void hashmap_new(hashmap*);

void hashmap_del(hashmap*);

//if item is not found, return NULL
void* hashmap_get(hashmap*, const char* key);

void hashmap_set(hashmap*, const char* key, void* value);
//returns -1 if the key is not in the map
int hashmap_unset(hashmap*, const char* key);

//checks if key exists in the map
bool hashmap_exists(hashmap*, const char* key);

hash_t hash_str(const char* str);
