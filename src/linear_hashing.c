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

static inline size_t*
hash_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, unsigned int const i) {
    return (size_t*)((char*)bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size));
}

static inline void*
key_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, unsigned int const i) {
    return (char*)bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size) + sizeof(size_t);
}

static inline void*
val_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, unsigned int const i) {
    return (char*)bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size) + sizeof(size_t)
           + table->key_size;
}

int
lh_next(lh_iterator_t* const iterator, const void** const key, const void** const val) {
    assert(iterator != NULL && key != NULL && val != NULL);

    const struct bucket* bucket;
    do { // Skip empty buckets
        if (iterator->bucket_index == iterator->table->bucket_count) {
            return 0;
        }

        bucket = iterator->table->buckets[iterator->bucket_index];
    } while (0 == bucket->item_count && ++iterator->bucket_index);

    *key = key_ptr(iterator->table, bucket, iterator->item_index);
    *val = val_ptr(iterator->table, bucket, iterator->item_index);
    ++iterator->item_index;

    if (iterator->item_index == bucket->item_count) {
        if (bucket->next != NULL) {
            bucket = bucket->next;
        } else {
            ++iterator->bucket_index;
        }
        iterator->item_index = 0;
    }
    return 1;
}

static inline struct bucket*
bucket_ptr(const lh_hashtable_t* const table, size_t const hash) {
    size_t i = hash & ((1 << table->round) - 1);
    if (i < table->split_bucket) {
        i = i ^ (1 << (table->round - 1));
    }
    return table->buckets[i];
}
