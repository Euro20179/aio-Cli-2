#include "bucket.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _bucket_new(bucket* arr, size_t len, size_t member_size) {
    arr->items = calloc(len, member_size);
    arr->allocated = len;
    arr->item_count = 0;
    arr->member_size = member_size;

    arr->initialized = 1;
}

void bucket_del(bucket* b) {
    free(b->items);
    b->initialized = 0;
    b->member_size = 0;
    b->allocated = 0;
    b->items = 0;
}

size_t bucket_size(bucket * arr) {
    return arr->allocated;
}

bool bucket_full(bucket* arr) {
    return arr->item_count >= arr->allocated;
}

bool bucket_predict_fullness(bucket * arr, size_t count) {
    return (arr->item_count + count) >= arr->allocated;
}

int bucket_set(bucket* arr, size_t idx, void* value) {
    if(arr->item_count >= arr->allocated) {
        return -2;
    }

    if(idx >= arr->allocated) {
        return -1;
    }

    arr->items[idx * arr->member_size] = value;

    arr->item_count++;
    return 0;
}

int bucket_remove(bucket* arr, size_t idx) {
    if(idx >= arr->allocated) {
        return -1;
    }

    arr->items[idx * arr->member_size] = 0;

    arr->item_count--;

    return 0;
}

bool bucket_full_at(bucket* arr, size_t idx) {
    if(arr->items[idx] == NULL) {
        return false;
    }
    return true;
}

int bucket_at(bucket* arr, size_t idx, void* out) {
    if(idx >= arr->allocated) {
        return -1;
    }
    memcpy(out, arr->items + (idx * arr->member_size), arr->member_size);
    return 0;
}


void** bucket_get_ref(bucket* arr, size_t idx) {
    if(idx >= arr->allocated) {
        return NULL;
    }

    return &arr->items[idx * arr->member_size];
}

void* bucket_get(bucket* arr, size_t idx) {
    if(idx >= arr->allocated) {
        return NULL;
    }
    return arr->items[idx * arr->member_size];
}

void bucket_increase_size(bucket * b, size_t amount) {
    b->items = realloc(b->items, (b->allocated + amount) * b->member_size);
    b->allocated += amount;
}
