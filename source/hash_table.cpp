#include "hash_table.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <immintrin.h>
#include <x86intrin.h>

#include "list.h"
#include "hash_function.h"


// static ----------------------------------------------------------------------


#define INITIAL_CAPACITY 4096
#define LIST_INITIAL_CAPACITY 4


typedef struct HashTable
{
    List*  buckets;
    size_t capacity;
    size_t length;
} HashTable;


static uint32_t hashFunction(const char* str, size_t len);


// public ----------------------------------------------------------------------


HashTable* hashTableCtor(void)
{
    HashTable* table = (HashTable*)calloc(1, sizeof(HashTable));
    if (!table)
    {
        fprintf(stderr, "Error while allocating memory for table struct\n");
        return NULL; 
    }

    table->length   = 0;
    table->capacity = INITIAL_CAPACITY;

    table->buckets = (List*)calloc(table->capacity, 
                                   sizeof(List));
    if (!table->buckets)
    {
        fprintf(stderr, "Error while creating hash table entries\n");
        free(table);
        return NULL;
    }

    for (int i = 0; i < INITIAL_CAPACITY; i++)
    {
        if(listCtor(&table->buckets[i], LIST_INITIAL_CAPACITY) != ListOperationError_SUCCESS)
        {
            fprintf(stderr, "Error while ctor\n");

            for (int j = 0; j <= i; j++)
            {
                listDtor(&table->buckets[j]);
            }

            free(table->buckets);
            free(table);
            return NULL;
        }
    }

    return table;
}


HashTableOperationError hashTableDtor(HashTable* table)
{
    if (!table)
    {
        fprintf(stderr, "Empty pointer on table while destroing\n");
        return HASH_TABLE_ERROR; 
    }

    for (size_t i = 0; i < table->capacity; i++)
    {
        listDtor(&table->buckets[i]);
    }

    free(table->buckets);
    free(table);

    return HASH_TABLE_SUCCESS;
}


const char* hashTableSet(HashTable* table, const char* key, size_t length)
{
    assert(table != NULL);
    assert(key   != NULL);

    size_t index = crc_hash(key, length);
    List* list = &table->buckets[index];

    const char* key_pointer = listIncrementValue(list, key, length);
    if (key_pointer) 
    {
            return key_pointer; 
    }

    int new_node_index = listInsertTail(list);
    if (new_node_index == 0)
    {
        fprintf(stderr, "Error while inserting\n");
        return NULL;
    }

    NodeData* new_node = &list->data[new_node_index];

    new_node->key_pointer = key;
    new_node->length      = length;
    new_node->count       = 1; 
    //list->lengths[new_node_index] = length;

    table->length++;

    return key;
}


HashTableOperationError hashTableDelete(HashTable* table, const char* key, size_t length)
{
    assert(table != NULL);
    assert(key   != NULL);

    size_t index = hashFunction(key, length);
    List* list = &table->buckets[index];
    Node* node_array = list->node_array;

    size_t current_index = node_array[0].next;
    while (current_index != 0)
    {
        Node* node = &node_array[current_index];
        NodeData* node_data = &list->data[current_index];
        if (!memcmp(node_data->key_pointer, key, length))
        {
            listDeleteElement(list, current_index);
            break;
        }
        current_index = node->next;
    }

    table->length--;

    return HASH_TABLE_SUCCESS;
}


size_t hashTableGet(HashTable* table, const char* key, size_t length)
{
    assert(table != NULL);
    assert(key   != NULL);

    size_t index = hashFunction(key, length);
    List* list = &table->buckets[index];
    Node* node_array = list->node_array;

    size_t current_index = node_array[0].next;
    while (current_index != 0)
    {
        Node* node = &node_array[current_index];
        NodeData* node_data = &list->data[current_index];
        if (!memcmp(node_data->key_pointer, key, length))
        {
            return node_data->count;
        }
        current_index = node->next;
    }

    return 0;
}


HashTableIterator hashTableIterator(HashTable* table)
{
    assert(table != NULL);
    
    return {
        .key    = NULL,
        .length = 0,
        .count  = 0,

        ._table = table,
        ._bucket_index = 0,
        ._node_index = 0,
    };
}


bool hashTableNext(HashTableIterator* iterator)
{
    assert(iterator         != NULL);
    assert(iterator->_table != NULL);

    while (iterator->_bucket_index < iterator->_table->capacity)
    {
        List* list = &iterator->_table->buckets[iterator->_bucket_index];
        Node* node_array = list->node_array;

        iterator->_node_index = node_array[iterator->_node_index].next;

        if (iterator->_node_index != 0)
        {
            NodeData* node = &list->data[iterator->_node_index];

            iterator->key    = node->key_pointer;
            iterator->length = node->length;
            iterator->count  = node->count;

            return true;
        }
        else
        {
            iterator->_bucket_index++; 
            iterator->_node_index = 0;
        }
    }

    return false;
}


size_t hashTableGetLength(HashTable* hash_table)
{
    assert(hash_table != NULL);

    return hash_table->length;
}


// static ----------------------------------------------------------------------


static uint32_t hashFunction(const char* str, size_t len) 
{
//    const uint32_t FNV_prime = 0x01000193;
//    const uint32_t FNV_offset_basis = 0x811C9DC5;
//    uint32_t hash = FNV_offset_basis;
//
//    uint32_t chunk;
//    while (len >= 4) 
//    {
//        memcpy(&chunk, str, 4);
//        hash ^= chunk;
//        hash *= FNV_prime;
//        str += 4;
//        len -= 4;
//    }
//
//    if (len > 0)
//    {
//        hash ^= *str++;
//        hash *= FNV_prime;
//    }
//    if (len > 1)
//    {
//        hash ^= *str++;
//        hash *= FNV_prime;
//    }
//    if (len > 2)
//    {
//        hash ^= *str++;
//        hash *= FNV_prime;
//    }
//
//    return hash & (INITIAL_CAPACITY - 1);

    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) 
    {
        crc = _mm_crc32_u8(crc, str[i]);
    }
    return (~crc) & (INITIAL_CAPACITY - 1);
}
//static uint32_t hashFunction(const char* str, size_t len)
//{
//    // crc32 hash function 
//    const char* i = NULL;
//    int j;
//    uint32_t byte = 0, crc = 0, mask = 0;
//    // very risk to make this variable static but i want to try
//    static uint32_t crc32_table[256] = {0};
//
//    if (crc32_table[1] == 0) 
//    {
//        for (byte = 0; byte <= 255; byte++) 
//        {
//            crc = byte;
//            for (j = 7; j >= 0; j--) 
//            {   
//                mask = -(crc & 1);
//                crc = (crc >> 1) ^ (0xEDB88320 & mask);
//            }
//            crc32_table[byte] = crc;
//        }
//    }
//
//    i = str;
//    const char* end = str + len;
//    crc = 0xFFFFFFFF;
//
//    for (; i < end - 3; i+=4)
//    {
//        crc = _mm_crc32_u32(crc, *(const uint32_t*)(i));
//    }
//
//    switch (end - i) {
//        case 3: crc = _mm_crc32_u8(crc, *i++);
//        case 2: crc = _mm_crc32_u8(crc, *i++);
//        case 1: crc = _mm_crc32_u8(crc, *i++);
//        default: break;
//    }
//    
//    return ~crc & (INITIAL_CAPACITY - 1);
//}
