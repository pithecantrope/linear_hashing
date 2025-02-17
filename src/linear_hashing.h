#ifndef LINEAR_HASHING_H
#define LINEAR_HASHING_H

#include <stdlib.h>

#ifndef EH_HASH_T
#define EH_HASH_T size_t
#endif

struct linear_hashing_hashtable;
typedef struct linear_hashing_hashtable lh_hashtable_t;

lh_hashtable_t* lh_create(size_t key_size, size_t value_size, size_t bucket_capacity,
                          EH_HASH_T (*const hash)(const void*),
                          int (*const cmp)(const void*, const void*));
void lh_destroy(lh_hashtable_t* table);
// void lh_insert(lh_hashtable_t* table, const void* key, const void* value);

// typedef struct {
//         const lh_hashtable_t* table;
//         struct bucket* bucket;
//         size_t bucket_index;
//         size_t item_index;
// } lh_iterator_t;

// static inline lh_iterator_t
// lh_iter(const lh_hashtable_t* table) {
//         assert(table != NULL);
//         return (lh_iterator_t){.table = table};
// }

// int lh_next(lh_iterator_t* iterator, const void** key, const void** value);

#endif // LINEAR_HASHING_H
