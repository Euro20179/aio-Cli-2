#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct bucket{
    void** items;

    size_t item_count;

    size_t allocated;

    size_t member_size;
    _Bool initialized;
} bucket;

#define bucket(ty) struct bucket

//creates a new bucket and sets all members to 0
#define bucket_new(arr, len, ty) _bucket_new(arr, len, sizeof(ty))

#define bucket_set_rval(arr, idx, val) bucket_set(arr, idx, (char*)val)

void _bucket_new(bucket*, size_t len, size_t member_size);

size_t bucket_size(bucket*);

//is the bucket full currently
bool bucket_full(bucket*);

//will the bucket be full if i add count more items
bool bucket_predict_fullness(bucket*, size_t count);

//returns -1 if idx is out of bounds
//returns -2 if bucket is full
int bucket_set(bucket*, size_t idx, void* value);

//returns -1 if idx is out of bounds
int bucket_remove(bucket*, size_t idx);

//get an item at idx, and put it into out
//returns -1 if idx is out of bounds
int bucket_at(bucket*, size_t idx, void* out);

//returns a pointer to the item at idx
//returns NULL if idx is out of bounds
void** bucket_get_ref(bucket*, size_t idx);

//returns the value at idx
//returns NULL if idx is out of bounds
void* bucket_get(bucket*, size_t idx);
