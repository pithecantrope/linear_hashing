#include "linear_hashing.h"

struct bucket {
    struct bucket* next;
    unsigned item_count;
    void* items[];
};

struct linear_hashing_hashtable {
    struct bucket** buckets;
    size_t bucket_count;
    size_t split_bucket_index;
    size_t split_bucket_count_limit;

    size_t key_size;
    size_t val_size;
    size_t (*hash)(const void*);
    int (*cmp)(const void*, const void*);
    unsigned bucket_capacity;
};

lh_hashtable_t*
lh_create(size_t const key_size, size_t const val_size, unsigned const bucket_capacity,
          size_t (*const hash)(const void*), int (*const cmp)(const void*, const void*)) {
    assert(key_size != 0 && val_size != 0 && bucket_capacity > 1 && hash != NULL && cmp != NULL);

    lh_hashtable_t* const table = malloc(sizeof(lh_hashtable_t));
    assert(table != NULL);
    *table = (lh_hashtable_t){
        .buckets = malloc(1 * sizeof(struct bucket*)),
        .bucket_count = 1,
        .split_bucket_index = 0,
        .split_bucket_count_limit = 2,

        .key_size = key_size,
        .val_size = val_size,
        .hash = hash,
        .cmp = cmp,
        .bucket_capacity = bucket_capacity,
    };
    assert(table->buckets != NULL);

    table->buckets[0] = malloc(sizeof(struct bucket) + bucket_capacity * (key_size + val_size));
    assert(table->buckets[0] != NULL);
    *table->buckets[0] = (struct bucket){0};

    return table;
}

void
lh_destroy(lh_hashtable_t* const table) {
    assert(table != NULL);

    for (size_t i = 0; i < table->bucket_count; ++i) {
        struct bucket* bucket = table->buckets[i];
        while (bucket != NULL) {
            struct bucket* const next = bucket->next;
            free(bucket);
            bucket = next;
        }
    }
    free(table->buckets);
    free(table);
}
