#include <stdio.h>
#include <string.h>
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
        size_t bucket_capacity;
        if (argc != 2 || sscanf(argv[1], "%zu", &bucket_capacity) != 1) {
                fprintf(stderr, "Usage: %s <bucket_capacity>\n", *argv);
                return EXIT_FAILURE;
        }

        const char path[] = "res/data.txt";
        FILE* file = fopen(path, "rt");
        if (file == NULL) {
                fprintf(stderr, "Could not open %s for reading\n", path);
                return EXIT_FAILURE;
        }

        lh_hashtable_t* const table = lh_create(WORD_SIZE, sizeof(size_t), bucket_capacity,
                                                hash_fnv1a, cmp);
        // const char* word;
        // char buffer[BUFFER_SIZE];
        // size_t i = 0;
        // while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        //     word = strtok(buffer, "\n");
        //     printf("%s\n", word);
        // const void* const value = lh_lookup(table, word);
        // if (value == NULL) {
        // lh_insert(table, word, &(size_t){1});
        // } else {
        //     ++*(size_t*)value;
        // }

        //     while (word != NULL) {
        //         word = strtok(NULL, "\n");
        //     }
        //     if (++i == 12) {
        //         break;
        //     }
        // }

        // size_t count = 0;
        // printf("\nPrinting:\n");
        // const void *key, *val;
        // for (lh_iterator_t iterator = lh_iter(table); lh_next(&iterator, &key, &val);) {
        //     printf("%s\n", (char*)key);
        // if (*(size_t*)val >= 1024) {
        //     ++count;
        // }
        // }
        // printf("%zu words with >= 1024 repetitions\n", count);

        lh_destroy(table);
        fclose(file);
        return EXIT_SUCCESS;
}
