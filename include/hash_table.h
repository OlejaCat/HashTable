#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>
#include <stdbool.h>

#include "list.h"

typedef enum HashTableOperationError
{
    HASH_TABLE_SUCCESS               = 0,
    HASH_TABLE_ERROR                 = 1,
    HASH_TABLE_INVALID_INPUT         = 2,
    HASH_TABLE_BAD_MEMORY_ALLOCATION = 3,
    HASH_TABLE_KEY_NOT_FOUND         = 4,
} HashTableOperationError;

typedef struct HashTable HashTable;

HashTable* hashTableCtor(void);
HashTableOperationError hashTableDtor(HashTable* table);

void* hashTableGet(HashTable* table, const char* key);
const char* hashTableSet(HashTable* table, const char* key, void* value);
HashTableOperationError hashTableDelete(HashTable* table, const char* key);

size_t hashTabelLength(HashTable* table);

typedef struct HashTableIterator
{
    const char* key;
    void*       value;

    // this fields be addressed directly
    HashTable* _table;
    size_t     _bucket_index;
    size_t     _node_index;
} HashTableIterator;

HashTableIterator hashTableIterator(HashTable* table);
bool hashTableNext(HashTableIterator* iterator);

#endif // HASH_TABLE_H
