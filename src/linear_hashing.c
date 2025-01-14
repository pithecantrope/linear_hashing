#include "linear_hashing.h"
#include <stdio.h>

struct bucket {
        struct bucket* next;
        size_t item_count;
        char items[]; // [hash][key][val]
};

struct linear_hashing_hashtable {
        struct bucket** buckets;
        size_t bucket_count;
        size_t bucket_limit; // 1 << (1 + round)
        size_t split_bucket_index;

        size_t bucket_capacity;
        size_t key_size;
        size_t val_size;
        size_t (*hash)(const void*);
        int (*cmp)(const void*, const void*);
};

static inline struct bucket*
create_bucket(lh_hashtable_t* const table) {
        struct bucket* bucket = malloc(
                sizeof(struct bucket)
                + table->bucket_capacity * (sizeof(size_t) + table->key_size + table->val_size));
        assert(bucket != NULL);

        *bucket = (struct bucket){0};
        return bucket;
}

lh_hashtable_t*
lh_create(size_t const key_size, size_t const val_size, size_t const bucket_capacity,
          size_t (*const hash)(const void*), int (*const cmp)(const void*, const void*)) {
        assert(key_size != 0 && val_size != 0 && bucket_capacity != 0 && hash != NULL
               && cmp != NULL);

        lh_hashtable_t* const table = malloc(sizeof(lh_hashtable_t));
        assert(table != NULL);
        *table = (lh_hashtable_t){
                .buckets = malloc(2 * sizeof(struct bucket*)),
                .bucket_count = 2,
                .bucket_limit = 4,
                .split_bucket_index = 0,

                .bucket_capacity = bucket_capacity,
                .key_size = key_size,
                .val_size = val_size,
                .hash = hash,
                .cmp = cmp,
        };
        assert(table->buckets != NULL);

        for (size_t i = 0; i < 2; ++i) {
                table->buckets[i] = create_bucket(table);
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
hash_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, size_t const i) {
        return (size_t*)(bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size));
}

static inline void*
key_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, size_t const i) {
        return (char*)bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size)
               + sizeof(size_t);
}

static inline void*
val_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, size_t const i) {
        return (char*)bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size)
               + sizeof(size_t) + table->key_size;
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
        // PERF: format
        if (hash % (table->bucket_limit >> 1) < table->split_bucket_index) {
                return table->buckets[hash % table->bucket_limit];
        }
        return table->buckets[hash % (table->bucket_limit >> 1)];
}



static void
split(lh_hashtable_t* const table, struct bucket* const bucket) {
        (void)table;
        (void)bucket;
        exit(EXIT_FAILURE);
        return;
}

void
lh_insert(lh_hashtable_t* const table, const void* const key, const void* const val) {
        assert(table != NULL && key != NULL && val != NULL);

        const size_t hash = table->hash(key);
        struct bucket* bucket = bucket_ptr(table, hash);
        do {
                for (size_t i = 0; i < bucket->item_count; ++i) {
                        if (*hash_ptr(table, bucket, i) == hash
                            && table->cmp(key_ptr(table, bucket, i), key) == 0) {
                                memcpy(val_ptr(table, bucket, i), val, table->val_size);
                                return; // Update
                        }
                }
        } while (bucket->next != NULL && (bucket = bucket->next));

        *hash_ptr(table, bucket, bucket->item_count) = hash;
        memcpy(key_ptr(table, bucket, bucket->item_count), key, table->key_size);
        memcpy(val_ptr(table, bucket, bucket->item_count), val, table->val_size);
        ++bucket->item_count;
        if (bucket->item_count < table->bucket_capacity) {
                return; // Insert
        }

        split(table, bucket);
}
