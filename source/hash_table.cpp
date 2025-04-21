#include "hash_table.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"


// static ----------------------------------------------------------------------


#define INITIAL_CAPACITY 2
#define LIST_INITIAL_CAPACITY 1000
#define LOAD_FACTOR 2
#define SCALE_FACTOR 2


typedef struct HashTable
{
    List*  buckets;
    size_t capacity;
    size_t length;
} HashTable;


static size_t hashFunction(const char* data, size_t length, size_t capacity);
static HashTableOperationError hashTableResize(HashTable* table);


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

    if ((double)table->length / table->capacity > LOAD_FACTOR)
    {
        if (hashTableResize(table) != HASH_TABLE_SUCCESS)
        {
            fprintf(stderr, "Error while resizing hash table\n");
            return NULL; 
        }
    }

    size_t index = hashFunction(key, length, table->capacity);
    List* list = &table->buckets[index];
    Node* node_array = list->node_array;

    size_t current_index = node_array[0].next;
    while (current_index != 0)
    {
        Node* node = &node_array[current_index];
        if (!memcmp(node->key_pointer, key, length))
        {
            node->count++;
            return node->key_pointer;
        }
        current_index = node->next;
    }

    int new_node_index = listInsertTail(list);
    if (new_node_index == 0)
    {
        fprintf(stderr, "Error while inserting\n");
        return NULL;
    }

    Node* new_node = &node_array[new_node_index];
    new_node->key_pointer = key;
    new_node->length      = length;
    new_node->count       = 1; 

    table->length++;
    return key;
}


HashTableOperationError hashTableDelete(HashTable* table, const char* key, size_t length)
{
    assert(table != NULL);
    assert(key   != NULL);

    size_t index = hashFunction(key, length, table->capacity);
    List* list = &table->buckets[index];
    Node* node_array = list->node_array;

    size_t current_index = node_array[0].next;
    while (current_index != 0)
    {
        Node* node = &node_array[current_index];
        if (!memcmp(node->key_pointer, key, length))
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

    size_t index = hashFunction(key, length, table->capacity);
    Node* node_array = table->buckets[index].node_array;

    size_t current_index = node_array[0].next;
    while (current_index != 0)
    {
        Node* node = &node_array[current_index];
        if (!memcmp(node->key_pointer, key, length))
        {
            return node->count;
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
            Node* node = &node_array[iterator->_node_index];

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


// static ----------------------------------------------------------------------


static HashTableOperationError hashTableResize(HashTable* table)
{
    assert(table != NULL);

    size_t old_capacity = table->capacity;
    table->capacity *= SCALE_FACTOR;

    List* new_buckets = (List*)calloc(table->capacity, sizeof(List));
    if (!new_buckets)
    {
        return HASH_TABLE_BAD_MEMORY_ALLOCATION; 
    }

    for (size_t index = 0; index < table->capacity; index++) 
    {
        if (listCtor(&new_buckets[index], LIST_INITIAL_CAPACITY) != ListOperationError_SUCCESS)
        {
            for (size_t i = 0; i < index; i++)
            {
                listDtor(&new_buckets[i]);
            }

            free(new_buckets);
            return HASH_TABLE_BAD_MEMORY_ALLOCATION;
        }
    }
    
    for (size_t index = 0; index < old_capacity; index++)
    {
        List* list = &table->buckets[index];
        Node* node_array = list->node_array;

        size_t current_index = node_array[0].next;
        while (current_index != 0)
        {
            Node* node = &node_array[current_index];

            size_t new_index = hashFunction(node->key_pointer, node->length, table->capacity);
            List* new_list = &new_buckets[new_index];
            
            int list_index = listInsertTail(new_list);
            if (list_index == 0)
            {
                fprintf(stderr, "Erro accured in resizing hash table");

                for (size_t i = 0; i < index; i++) 
                {
                    listDtor(&new_buckets[i]);
                }

                free(new_buckets);
                return HASH_TABLE_BAD_MEMORY_ALLOCATION;
            }

            Node* new_node = &new_list->node_array[list_index];

            new_node->key_pointer = node->key_pointer;
            new_node->length      = node->length;
            new_node->count       = node->count;

            current_index = node->next;
        }
    }

    for (size_t index = 0; index < old_capacity; index++)
    {
        listDtor(&table->buckets[index]);
    }

    free(table->buckets);
    table->buckets = new_buckets;
    
    return HASH_TABLE_SUCCESS;
}


static size_t hashFunction(const char* data, size_t length, size_t capacity)
{
    assert(data != NULL);

    unsigned long hash = 5381;
    for (size_t i = 0; i < length; i++)
    {
        hash = ((hash << 5) + hash) + (unsigned char)data[i];
    }

    return (size_t)hash % capacity;
}
