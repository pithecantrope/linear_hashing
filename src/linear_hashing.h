#ifndef LINEAR_HASHING_H
#define LINEAR_HASHING_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct linear_hashing_hashtable;
typedef struct linear_hashing_hashtable lh_hashtable_t;

lh_hashtable_t* lh_create(size_t key_size, size_t value_size, unsigned bucket_capacity,
                          size_t (*const hash)(const void*),
                          int (*const cmp)(const void*, const void*));
void lh_destroy(lh_hashtable_t* table);

#endif // LINEAR_HASHING_H
