#include "list.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
#include <assert.h>

#include "my_memcmp.h"


// static --------------------------------------------------------------------------------------------------------------


#define SCALE_FACTOR 2

static ListOperationError listResize(List* list);


// public --------------------------------------------------------------------------------------------------------------


// -------------------------------  CTOR AND DTOR  -------------------------------


ListOperationError listCtor(List* list, size_t capacity)
{
    assert(list != NULL);

    list->capacity = capacity;

    list->node_array = (Node*)aligned_alloc(32, capacity * sizeof(Node));
    if (list->node_array == NULL)
    {
        fprintf(stderr, "Error allocating node array for list\n");
        return ListOperationError_ERROR;
    }

    list->data = (NodeData*)calloc(capacity, sizeof(NodeData));
    if (list->data == NULL)
    {
        fprintf(stderr, "Error while allocating data for nodes\n");
        return ListOperationError_ERROR;
    }

    list->node_array[0].next = 0;
    list->node_array[0].prev = 0;

    size_t index = 1;
    int simd_size = 4;
    for (; index  < capacity - simd_size + 1; index += simd_size)
    {
        __m256i next = _mm256_setr_epi32(
            index + 1, 0, index + 2, 0, index + 3, 0, index + 4, 0
        );


        _mm256_storeu_si256((__m256i*)(list->node_array + index), next);
    }

    for (; index < capacity; index++)
    {
        list->node_array[index].next = index + 1;
        list->node_array[index].prev = 0;
    }

    list->free_node = 1;
    list->size = 0;

    return ListOperationError_SUCCESS;
}


ListOperationError listDtor(List* list)
{
    if (!list)
    {
        return ListOperationError_ERROR;
    }

    free(list->node_array);
    free(list->data);

    return ListOperationError_SUCCESS;
}


// -------------------------------  INDEX OPERATIONS -------------------------------


int getNextIndex(List* list, int index)
{
    assert(list != NULL);

    return list->node_array[index].next;
}


int getPreviousIndex(List* list, int index)
{
    assert(list != NULL);

    return list->node_array[index].prev;
}


// -------------------------------  INSERT OPERATIONS  -------------------------------


int listInsert(List* list, int index)
{
    assert(list != NULL);

    if (index > (int)list->capacity)
    {
        return 0;
    }

    size_t real_index = list->free_node;
    if (real_index >= list->capacity)
    {
        if (listResize(list) != ListOperationError_SUCCESS)
        {
            fprintf(stderr, "Error while resizing list\n");
            return 0;
        }
    }

    list->free_node = list->node_array[real_index].next;

    list->node_array[real_index].next = list->node_array[index].next;
    list->node_array[real_index].prev = index;

    list->node_array[list->node_array[index].next].prev = real_index;
    list->node_array[index].next = real_index;

    list->size++;

    return real_index;
}


int listInsertHead(List* list)
{
    assert(list != NULL);

    return listInsert(list, 0);
}


int listInsertTail(List* list)
{
    assert(list != NULL);

    return listInsert(list, getPreviousIndex(list, 0));
}


// ------------------------------- DELETE ELEMENTS -------------------------------


ListOperationError listDeleteElement(List* list, int index)
{
    assert(list != NULL);

    if (index > (int)list->capacity)
    {
        return ListOperationError_ERROR; 
    }

    size_t prev = list->node_array[index].prev;
    size_t next = list->node_array[index].next;

    list->node_array[prev].next = next;
    list->node_array[next].prev = prev;

    list->node_array[index].next = list->free_node;
    list->node_array[index].prev = 0;

    list->free_node = index;
    list->size--;

    return ListOperationError_SUCCESS;
}


ListOperationError listDeleteHead(List* list)
{
    assert(list != NULL);

    listDeleteElement(list, getNextIndex(list, 0));

    return ListOperationError_SUCCESS;
}


ListOperationError listDeleteTail(List* list)
{
    assert(list != NULL);

    listDeleteElement(list, getPreviousIndex(list, 0));

    return ListOperationError_SUCCESS;
}


// -------------------------------  CONTAINS OPERATIONS  -------------------------------


const char* listIncrementValue(List* list, const char* key, size_t length)
{
    assert(list != NULL);
    assert(key  != NULL);

    for (size_t i = 1; i <= list->size; i++) 
    {
        NodeData* node_data = &list->data[i];
        if (node_data->length == (int)length) 
        {
            if (myMemcmp(key, node_data->key_pointer, length)) 
            {
                node_data->count++;
                return node_data->key_pointer;
            }
        }
    }

    return NULL;
}


size_t listGetValue(List* list, const char* key, size_t length)
{
    assert(list != NULL);
    assert(key  != NULL);

    size_t size = list->size;
    NodeData* node_data_array = list->data;

    for (size_t i = 1; i <= size; i++) 
    {
        NodeData* node_data = node_data_array + i;
        if (node_data->length != (int)length)
        {
            continue; 
        }

        //if (myMemcmp(key, node_data->key_pointer, length))
        //{
        //    return node_data->count;
        //}

        uint32_t bitmask = 0;
        uint32_t mask    = 0;

        mask = (1U << length) - 1;

        __asm__ __volatile__ (
                "vmovdqu ymm0, [%[key1]]\n"      
                "vmovdqu ymm1, [%[key2]]\n"      
                "vpcmpeqb ymm2, ymm0, ymm1\n" 
                "vpmovmskb %[bitmask], ymm2\n"    
                : [bitmask] "=a" (bitmask)           
                : [key1] "r" (key),                
                  [key2] "r" (node_data->key_pointer)
                : "ymm0", "ymm1", "ymm2"       
                );

        if ((bitmask & mask) == mask) 
        {
            return node_data->count;
        }
    }

    return 0;
}


// static --------------------------------------------------------------------------------------------------------------


static ListOperationError listResize(List* list) 
{
    assert(list != NULL);

    size_t old_capacity = list->capacity;
    list->capacity = old_capacity * SCALE_FACTOR;

    Node* new_array = (Node*)realloc(list->node_array, list->capacity * sizeof(Node));
    if (!new_array)
    {
        fprintf(stderr, "Error while reallocating memory list\n");
        return ListOperationError_ERROR;
    }
    list->node_array = new_array;
    
    NodeData* new_array_data = (NodeData*)realloc(list->data, list->capacity * sizeof(NodeData));
    if (!new_array_data)
    {
        fprintf(stderr, "Error reallocating memory for data in chain\n");
        return ListOperationError_ERROR;
    }
    list->data = new_array_data;

    size_t index = list->free_node;
    int simd_size = 4;
    for (; index  < list->capacity - simd_size + 1; index += simd_size)
    {
        __m256i next = _mm256_setr_epi32(
            index + 1, 0, index + 2, 0, index + 3, 0, index + 4, 0
        );


        _mm256_storeu_si256((__m256i*)(list->node_array + index), next);
    }

    for (; index < list->capacity; index++)
    {
        list->node_array[index].next = index + 1;
        list->node_array[index].prev = 0;
    }

    return ListOperationError_SUCCESS;
}
