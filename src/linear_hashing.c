#include "linear_hashing.h"
#include <assert.h>
#include <string.h>

typedef unsigned char byte_t;

struct bucket {
        struct bucket* next;
        size_t item_count;
        byte_t items[]; // [hash][key][val]
};

#define ITEM_SIZE(table) (sizeof(EH_HASH_T) + (table)->key_size + (table)->val_size)

struct linear_hashing_hashtable {
        struct bucket** buckets;
        size_t bucket_limit;
        size_t bucket_count;
        size_t split_idx;

        size_t bucket_capacity;
        size_t key_size;
        size_t val_size;
        EH_HASH_T (*hash)(const void*);
        int (*cmp)(const void*, const void*);
};

static inline void*
xmalloc(size_t const size) {
        void* const ptr = malloc(size);
        assert(ptr != NULL);
        return ptr;
}

static inline void*
xrealloc(void* const ptr, size_t const size) {
        void* const tmp = realloc(ptr, size);
        assert(tmp != NULL);
        return tmp;
}

static inline struct bucket*
create_bucket(const lh_hashtable_t* const table) {
        struct bucket* const bucket = xmalloc(sizeof(*table->buckets)
                                              + table->bucket_capacity * ITEM_SIZE(table));
        *bucket = (struct bucket){0};
        return bucket;
}

lh_hashtable_t*
lh_create(size_t const key_size, size_t const val_size, size_t const bucket_capacity,
          EH_HASH_T (*const hash)(const void*), int (*const cmp)(const void*, const void*)) {
        assert(key_size != 0 && val_size != 0 && bucket_capacity > 1 && hash != NULL);

        lh_hashtable_t* const table = xmalloc(sizeof(*table));
        const size_t init_count = 2;
        const size_t init_limit = init_count << 1;
        *table = (lh_hashtable_t){
                .buckets = xmalloc(init_limit * sizeof(*table->buckets)),
                .bucket_limit = init_limit,
                .bucket_count = init_count,
                .split_idx = 0,

                .bucket_capacity = bucket_capacity,
                .key_size = key_size,
                .val_size = val_size,
                .hash = hash,
                .cmp = cmp,
        };

        for (size_t i = 0; i < init_count; ++i) {
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

// static inline size_t*
// hash_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, size_t const i) {
//         return (size_t*)(bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size));
// }

// static inline void*
// key_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, size_t const i) {
//         return (char*)bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size)
//                + sizeof(size_t);
// }

// static inline void*
// val_ptr(const lh_hashtable_t* const table, const struct bucket* const bucket, size_t const i) {
//         return (char*)bucket->items + i * (sizeof(size_t) + table->key_size + table->val_size)
//                + sizeof(size_t) + table->key_size;
// }

// int
// lh_next(lh_iterator_t* const iterator, const void** const key, const void** const val) {
//         assert(iterator != NULL && key != NULL && val != NULL);

//         const struct bucket* bucket;
//         do { // Skip empty buckets
//                 if (iterator->bucket_index == iterator->table->bucket_count) {
//                         return 0;
//                 }

//                 if (iterator->bucket == NULL) {
//                         bucket = iterator->table->buckets[iterator->bucket_index];
//                 } else {
//                         bucket = iterator->bucket;
//                 }
//         } while (0 == bucket->item_count && ++iterator->bucket_index);

//         *key = key_ptr(iterator->table, bucket, iterator->item_index);
//         *val = val_ptr(iterator->table, bucket, iterator->item_index);
//         ++iterator->item_index;

//         if (iterator->item_index == bucket->item_count) {
//                 if (bucket->next != NULL) {
//                         iterator->bucket = bucket->next;
//                 } else {
//                         ++iterator->bucket_index;
//                 }
//                 iterator->item_index = 0;
//         }
//         return 1;
// }

// static inline struct bucket*
// bucket_ptr(const lh_hashtable_t* const table, size_t const hash) {
//         // PERF: format
//         if (hash % (table->bucket_limit >> 1) < table->split_bucket_index) {
//                 return table->buckets[hash % table->bucket_limit];
//         }
//         return table->buckets[hash % (table->bucket_limit >> 1)];
// }

// static void
// split(lh_hashtable_t* const table) {
//         if (table->bucket_count == table->bucket_limit) {
//                 // exit(EXIT_FAILURE);
//                 // assert(table->round <= 8 * sizeof(table->bucket_count)
//                 //        && "Increase bucket_capacity");
//                 table->buckets = realloc(table->buckets,
//                                          2 * table->bucket_count * sizeof(struct bucket*));
//                 assert(table->buckets != NULL);
//                 table->bucket_limit *= 2;
//                 table->split_bucket_index = 0;
//         }

//         struct bucket* bucket = table->buckets[table->split_bucket_index];
//         size_t bucket_item_count;
//         struct bucket* old_bucket = bucket;
//         struct bucket* new_bucket = create_bucket(table);
//         struct bucket* save = new_bucket;

//         for (; bucket != NULL; bucket = bucket->next) {
//                 bucket_item_count = bucket->item_count;
//                 bucket->item_count = 0;
//                 for (size_t i = 0; i < bucket_item_count; ++i) {
//                         const size_t* h = hash_ptr(table, bucket, i);
//                         struct bucket* const target = *h % table->bucket_limit ? old_bucket
//                                                                                : new_bucket;

//                         memcpy(hash_ptr(table, target, target->item_count), h,
//                                sizeof(size_t) + table->key_size + table->val_size);
//                         ++target->item_count;
//                         if (target->item_count == table->bucket_capacity) {
//                                 if (target == old_bucket) {
//                                         old_bucket = old_bucket->next;
//                                 } else {
//                                         new_bucket->next = create_bucket(table);
//                                         new_bucket = new_bucket->next;
//                                 }
//                         }
//                 }
//         }

//         while (old_bucket != NULL) {
//                 struct bucket* const next = old_bucket->next;
//                 old_bucket->next = NULL;
//                 free(old_bucket);
//                 old_bucket = next;
//         }

//         table->buckets[table->bucket_count] = save;
//         ++table->bucket_count;
//         ++table->split_bucket_index;
// }

// void
// lh_insert(lh_hashtable_t* const table, const void* const key, const void* const val) {
//         assert(table != NULL && key != NULL && val != NULL);

//         const size_t hash = table->hash(key);
//         struct bucket* bucket = bucket_ptr(table, hash);
//         do {
//                 for (size_t i = 0; i < bucket->item_count; ++i) {
//                         if (*hash_ptr(table, bucket, i) == hash
//                             && table->cmp(key_ptr(table, bucket, i), key) == 0) {
//                                 memcpy(val_ptr(table, bucket, i), val, table->val_size);
//                                 return; // Update
//                         }
//                 }
//         } while (bucket->next != NULL && (bucket = bucket->next));

//         if (bucket->item_count == table->bucket_capacity) {
//                 bucket->next = create_bucket(table);

//                 *hash_ptr(table, bucket->next, bucket->next->item_count) = hash;
//                 memcpy(key_ptr(table, bucket->next, bucket->next->item_count), key,
//                        table->key_size);
//                 memcpy(val_ptr(table, bucket->next, bucket->next->item_count), val,
//                        table->val_size);
//                 ++bucket->next->item_count;

//                 split(table);
//                 return;
//         }

//         *hash_ptr(table, bucket, bucket->item_count) = hash;
//         memcpy(key_ptr(table, bucket, bucket->item_count), key, table->key_size);
//         memcpy(val_ptr(table, bucket, bucket->item_count), val, table->val_size);
//         ++bucket->item_count;
// }
