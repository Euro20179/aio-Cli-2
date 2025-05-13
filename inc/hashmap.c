#include "hashmap.h"
#include "bucket.h"
#include "llist.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct _hashmap_item* _hashmap_item_create(hash_t hash, void* value) {
    struct _hashmap_item* item = malloc(sizeof(struct _hashmap_item));
    item->key_hash = hash;
    item->value = value;
    return item;
}

size_t collide(size_t idx, size_t maxlen) {
    return (idx + 1) % maxlen;
}

void _hashmap_item_destroy(struct _hashmap_item * item) {
    free(item);
}

void hashmap_new(hashmap * map) {
    bucket_new(&map->items, 256, llist(struct _hashmap_item*));
}

void hashmap_del(hashmap* map) {
    bucket_del(&map->items);
}

size_t get_idx_from_hash(hashmap* map, hash_t h) {
    return h % bucket_size(&map->items);
}

void hashmap_set(hashmap * map, const char *key, void *value) {
    hash_t h = hash_str(key);

    struct _hashmap_item* item = _hashmap_item_create(h, value);

    size_t idx = get_idx_from_hash(map, h);

    size_t initial_idx = idx;

    llist i;

    llist* ref;

    if(!bucket_full_at(&map->items, idx)) {
        llist_new(&i);
        llist_append(&i, item);
        bucket_set(&map->items, idx, &i);
    } else {
        ref = (llist*)bucket_get_ref(&map->items, idx);
        llist_append(ref, item);
    }

    map->item_count++;
}

int hashmap_unset(hashmap * map, const char *key) {
    hash_t h = hash_str(key);
    size_t idx = get_idx_from_hash(map, h);

    llist item = *(llist*)bucket_get(&map->items, idx);
    if(item.len) {
        return -1;
    }

    llist_node* cur = item.head;
    size_t llist_idx = 0;
    while(((struct _hashmap_item*)cur->data)->key_hash != h && cur->next != NULL) {
        cur = cur->next;
        llist_idx++;
    }

    _hashmap_item_destroy(cur->data);
    llist_remove(&item, llist_idx);
    if(item.len == 0) {
        llist_del(&item);
    }
    bucket_remove(&map->items, idx);

    map->item_count--;
    return 0;
}

void* hashmap_get(hashmap* map, const char* key) {
    hash_t h = hash_str(key);
    size_t idx = h % bucket_size(&map->items);
    llist item = *(llist*) bucket_get(&map->items, idx);

    llist_node* cur = item.head;
    while(cur != NULL && ((struct _hashmap_item*)cur->data)->key_hash != h && cur->next != NULL) {
        cur = cur->next;
    }
    if(cur == NULL) {
        return 0;
    }

    return ((struct _hashmap_item*)cur->data)->value;
}

bool hashmap_exists(hashmap * map, const char *key) {
    hash_t h = hash_str(key);

    size_t idx = h % bucket_size(&map->items);
    llist* item = (llist*)bucket_get_ref(&map->items, idx);

    llist_node* cur = item->head;
    while(((struct _hashmap_item*)cur->data)->key_hash != h && cur->next != NULL) {
        cur = cur->next;
    }
    if(cur == NULL) {
        return false;
    } return true;
}

hash_t hash_str(const char *str) {
    hash_t hash = 5381;
    int c;
    while(c = *str++) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}
