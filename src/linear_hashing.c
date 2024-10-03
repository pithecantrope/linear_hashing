#include "linear_hashing.h"

struct bucket {
    struct bucket* next;
    unsigned int item_count;
    void* items[]; // [hash][key][val]
};

struct linear_hashing_hashtable {
    struct bucket** buckets;
    size_t bucket_count;
    size_t split_bucket;
    unsigned int bucket_capacity;
    unsigned char round;

    size_t key_size;
    size_t val_size;
    size_t (*hash)(const void*);
    int (*cmp)(const void*, const void*);
};

lh_hashtable_t*
lh_create(size_t const key_size, size_t const val_size, unsigned int const bucket_capacity,
          size_t (*const hash)(const void*), int (*const cmp)(const void*, const void*)) {
    assert(key_size != 0 && val_size != 0 && bucket_capacity != 0 && hash != NULL && cmp != NULL);

    lh_hashtable_t* const table = malloc(sizeof(lh_hashtable_t));
    assert(table != NULL);
    *table = (lh_hashtable_t){
        .buckets = malloc(2 * sizeof(struct bucket*)),
        .bucket_count = 2,
        .split_bucket = 0,
        .bucket_capacity = bucket_capacity,
        .round = 1,

        .key_size = key_size,
        .val_size = val_size,
        .hash = hash,
        .cmp = cmp,
    };
    assert(table->buckets != NULL);

    for (size_t i = 0; i < 2; ++i) {
        table->buckets[i] = malloc(sizeof(struct bucket) + bucket_capacity * (sizeof(size_t) + key_size + val_size));
        assert(table->buckets[i] != NULL);
        *table->buckets[i] = (struct bucket){0};
    }

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
