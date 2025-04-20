#include "hash_table.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"


// static ----------------------------------------------------------------------


#define INITIAL_CAPACITY 16
#define LIST_INITIAL_CAPACITY 4
#define LOAD_FACTOR 2
#define SCALE_FACTOR 2

typedef struct HashTableElement
{
    char* key;
    void* value;
} HashTableElement;


typedef struct HashTable
{
    List*  buckets;
    size_t capacity;
    size_t length;
} HashTable;


static size_t hashFunction(const char* data, size_t capacity);
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

    table->length = 0;
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
        if(listCtor(&table->buckets[i], 
                    LIST_INITIAL_CAPACITY) != ListOperationError_SUCCESS)
        {
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
        List* list = &table->buckets[i];

        size_t current_index = list->node_array[0].next;
        while (current_index != 0)
        {
            HashTableElement* element = (HashTableElement*)list->node_array[current_index].data;
            free(element->key);
            free(element);
            current_index = list->node_array[current_index].next;
        }

        listDtor(&table->buckets[i]);
    }

    free(table->buckets);
    free(table);
    return HASH_TABLE_SUCCESS;
}


const char* hashTableSet(HashTable* table, const char* key, void* value)
{
    assert(table != NULL);
    assert(key   != NULL);
    assert(value != NULL);

    if ((double)table->length / table->capacity > LOAD_FACTOR)
    {
        if (hashTableResize(table) != HASH_TABLE_SUCCESS)
        {
            fprintf(stderr, "Error while resizing hash table\n");
            return NULL; 
        }
    }

    size_t index = hashFunction(key, table->capacity);
    Node* bucket = table->buckets[index].node_array;

    size_t current_index = bucket[0].next;
    while (current_index != 0)
    {
        HashTableElement* element = (HashTableElement*)bucket[current_index].data;
        if (!strcmp(element->key, key))
        {
            element->value = value;
            return element->key;
        }
        current_index = bucket[current_index].next;
    }
    
    HashTableElement* new_element = (HashTableElement*)malloc(sizeof(HashTableElement));
    char* key_copy = strdup(key);
    
    if (new_element == NULL 
     || key_copy    == NULL)
    {
        free(new_element);
        free(key_copy);
        return NULL;
    }

    new_element->key = key_copy;
    new_element->value = value;

    if (listInsertTail(&table->buckets[index], new_element))
    {
        free(new_element);
        free(key_copy);
        return NULL;
    }

    table->length++;
    return new_element->key;
}


HashTableOperationError hashTableDelete(HashTable* table, const char* key)
{
    assert(table != NULL);
    assert(key   != NULL);

    size_t index = hashFunction(key, table->capacity);
    List* list = &table->buckets[index];

    size_t current_index = list->node_array[0].next;
    bool found = false;

    while (current_index != 0)
    {
        HashTableElement* element = (HashTableElement*)list->node_array[current_index].data;
        if (!strcmp(element->key, key))
        {
            found = true; 
            break;
        }

        current_index = list->node_array[current_index].next;
    }

    if (!found)
    {
        return HASH_TABLE_KEY_NOT_FOUND;
    }

    HashTableElement* element = (HashTableElement*)list->node_array[current_index].data;
    free(element->key);
    free(element);

    listDeleteElement(list, current_index);


    table->length--;

    return HASH_TABLE_SUCCESS;
}


void* hashTableGet(HashTable* table, const char* key)
{
    assert(table != NULL);
    assert(key   != NULL);

    size_t index = hashFunction(key, table->capacity);
    Node* bucket = table->buckets[index].node_array;

    size_t current_index = bucket[0].next;
    while (current_index != 0)
    {
        HashTableElement* element = (HashTableElement*)bucket[current_index].data;
        if (!strcmp(element->key, key))
        {
            return element->value;
        }
        current_index = bucket[current_index].next;
    }

    return NULL;
}


HashTableIterator hashTableIterator(HashTable* table)
{
    assert(table != NULL);
    
    return {
        .key   = NULL,
        .value = NULL,
        ._table = table,
        ._bucket_index = 0,
        ._node_index = 0,
    };
}


bool hashTableNext(HashTableIterator* iterator)
{
    assert(iterator        != NULL);
    assert(iterator->_table != NULL);

    while (iterator->_bucket_index < iterator->_table->capacity)
    {
        List* list = &iterator->_table->buckets[iterator->_bucket_index];

        iterator->_node_index = list->node_array[iterator->_node_index].next;
        if (iterator->_node_index != 0)
        {
            HashTableElement* element = (HashTableElement*)list->node_array[iterator->_node_index].data;
            iterator->key = element->key;
            iterator->value = element->value;
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
        if (listCtor(&new_buckets[index], LIST_INITIAL_CAPACITY))
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
        size_t current_index = list->node_array[0].next;

        while (current_index != 0)
        {
            HashTableElement* element = (HashTableElement*)list->node_array[current_index].data;

            size_t new_index = hashFunction(element->key, list->capacity);
            List* new_list = &new_buckets[new_index];

            if (listInsertTail(new_list, element))
            {
                for (size_t i = 0; i < index; i++) 
                {
                    listDtor(&new_buckets[i]);
                }

                free(new_buckets);
                return HASH_TABLE_BAD_MEMORY_ALLOCATION;
            }

            current_index = list->node_array[current_index].next;
        }
    }

    for (size_t index = 0; index < old_capacity; index++)
    {
        listDtor(&table->buckets[index]);
    }

    table->buckets = new_buckets;
    
    return HASH_TABLE_SUCCESS;
}


static size_t hashFunction(const char* data, size_t capacity)
{
    assert(data != NULL);

    unsigned long hash = 5381;
    int c = 0;
    while ((c = *data++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return (size_t)hash % capacity;
}
