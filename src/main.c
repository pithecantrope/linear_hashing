#include <stdio.h>
#include "linear_hashing.h"

#define FNV_OFFSET 14695981039346656037ULL
#define FNV_PRIME  1099511628211ULL

size_t
hash_fnv1a(const void* key) {
    size_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (size_t)*p;
        hash *= FNV_PRIME;
    }
    return hash;
}

int
cmp(const void* key1, const void* key2) {
    return strcmp(key1, key2);
}

#define BUFFER_SIZE 4096
#define WORD_SIZE   32

int
main(int argc, char* argv[]) {
    unsigned bucket_capacity;
    if (argc != 2 || sscanf(argv[1], "%u", &bucket_capacity) != 1) {
        fprintf(stderr, "Usage: %s <bucket_capacity>\n", *argv);
        return EXIT_FAILURE;
    }

    const char path[] = "res/data.txt";
    FILE* file = fopen(path, "rt");
    if (file == NULL) {
        fprintf(stderr, "Could not open %s for reading\n", path);
        return EXIT_FAILURE;
    }

    lh_hashtable_t* const table = lh_create(WORD_SIZE, sizeof(size_t), bucket_capacity, hash_fnv1a, cmp);

    lh_destroy(table);
    fclose(file);
    return EXIT_SUCCESS;
}
