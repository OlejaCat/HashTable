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
#define MIN(a,b) (((a)<(b))?(a):(b))

static ListOperationError listResize(List* list);
static bool compareKeys(const char* key1, const char* key2, int length);


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

    int index = 1;
    int simd_size = 4;
    for (; index  < (int)(capacity - simd_size + 1); index += simd_size)
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


    assert(real_index <= (int)list->capacity);
    list->free_node = list->node_array[real_index].next;

    assert(list->node_array[real_index].next <= (int)list->capacity);
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


//const char* listIncrementValue(List* list, const char* key, size_t length)
//{
//    assert(list != NULL);
//    assert(key  != NULL);
//
//    const int simd_size = 4;
//    size_t i = 1;
//
//    __m128i target_length = _mm_set1_epi32(length);
//    for (; i + simd_size + 1 <= list->size; i+=simd_size)
//    {
//        __m128i current = _mm_loadu_si128((__m128i*)(list->lengths + i));
//
//        __m128i cmp_result = _mm_cmpeq_epi32(current, target_length);
//        int mask = _mm_movemask_ps(_mm_castsi128_ps(cmp_result));
//        if (mask == 0)
//        {
//            continue; 
//        }
//
//        for (int j = 0; j < simd_size; j++)
//        {
//            if (!mask & (1 << j)) 
//            {
//                continue; 
//            }
//
//            NodeData* node_data = &list->data[i + j];
//            const char* key1 = node_data->key_pointer;
//            const char* key2 = key;
//
//            if (compareKeys(key1, key2, length))
//            {
//                node_data->count++;
//                return node_data->key_pointer;
//            }
//        }
//    }
//
//    for (; i <= list->size; i++)
//    {
//        NodeData* node_data = &list->data[i];
//        if (list->lengths[i] == length) 
//        {
//            //int result = 0;
//            const char* key1 = node_data->key_pointer;
//            const char* key2 = key;
//
//            if (compareKeys(key1, key2, length))
//            {
//                node_data->count++;
//                return node_data->key_pointer;
//            }
//        }
//    }
//
//    return NULL;
//}

const char* listIncrementValue(List* list, const char* key, size_t length)
{
    assert(list != NULL);
    assert(key  != NULL);

    //Node* node_array = list->node_array;

    //size_t current_index = node_array[0].next;
    for (size_t i = 1; i <= list->size; i++) 
    {
        NodeData* node_data = &list->data[i];
        //Node* node = &node_array[i];
        if (node_data->length == length) 
        {
            //int result = 0;
            const char* key1 = node_data->key_pointer;
            const char* key2 = key;
            size_t len = length;



    //        __asm__ __volatile__ (
    //                "1:                      \n"
    //                "   cmp %[len], 4        \n"
    //                "   jb 2                 \n"

    //                "   movd xmm0, [%[key1]] \n"
    //                "   movd xmm1, [%[key2]] \n"
    //                "   pcmpeqb xmm0, xmm1   \n"
    //                "   pmovmskb eax, xmm0   \n"
    //                "   and eax, 0x0F        \n"
    //                "   cmp eax, 0x0F        \n"
    //                "   jne 4                \n"

    //                "   add %[key1], 4       \n"
    //                "   add %[key2], 4       \n"
    //                "   sub %[len], 4        \n"
    //                "   jmp 1                \n"

    //                "2:                      \n"
    //                "   test %[len], %[len]  \n"
    //                "   jz 3                 \n"
    //                "   mov cl, [%[key1]]    \n"
    //                "   cmp cl, [%[key2]]    \n"
    //                "   jne 4                \n"
    //                "   inc %[key1]          \n"
    //                "   inc %[key2]          \n"
    //                "   dec %[len]           \n"
    //                "   jnz 2                \n"

    //                "3:                      \n"
    //                "   mov eax, 1           \n"
    //                "   jmp 5                \n"

    //                "4:                      \n"
    //                "   xor eax, eax         \n"
    //                "5:                      \n"

    //                : "=a" (result),
    //            [key1] "+r" (key1),
    //            [key2] "+r" (key2),
    //            [len] "+r" (len)
    //                :
    //                    : "rcx", "xmm0", "xmm1", "memory", "cc"
    //        );

            if (compareKeys(key1, key2, len)) 
            {
                node_data->count++;
                return node_data->key_pointer;
            }
        }
    }

    //while (current_index != 0)
    //{
    //    NodeData* node_data = &list->data[current_index];
    //    Node* node = &node_array[current_index];
        //if (!memcmp(node_data->key_pointer, key, length))
        //{
        //    node_data->count++;
        //    return node_data->key_pointer;
        //}
        //if (node_data->length == length 
        // && myMemcmp(node_data->key_pointer, key, length))
        //{
        //    node_data->count++;
        //    return node_data->key_pointer;
        //}
        //if (node_data->length == length) 
        //{
        //    int result;
        //    const char* key1 = node_data->key_pointer;
        //    const char* key2 = key;
        //    size_t len = length;

        //    __asm__ __volatile__ (
        //        "1:                      \n"
        //        "   cmp $4, %[len]       \n"          
        //        "   jb  2f               \n"                   
        //        "   mov (%[key1]), %%ecx \n"    
        //        "   cmp (%[key2]), %%ecx \n"    
        //        "   jne 4f               \n"                  
        //        "   add $4, %[key1]      \n"         
        //        "   add $4, %[key2]      \n"
        //        "   sub $4, %[len]       \n"
        //        "   jmp 1b               \n"                 

        //        "2:                      \n"
        //        "   test %[len], %[len]  \n"     
        //        "   jz 3f                \n"                   
        //        "   mov (%[key1]), %%cl  \n"     
        //        "   cmp (%[key2]), %%cl  \n"     
        //        "   jne 4f               \n"                  
        //        "   inc %[key1]          \n"             
        //        "   inc %[key2]          \n"
        //        "   dec %[len]           \n"
        //        "   jnz 2b               \n"                  

        //        "3:                      \n"
        //        "   mov $1, %%eax        \n"           
        //        "   jmp 5f               \n"
        //        "4:                      \n"
        //        "   xor %%eax, %%eax     \n"        
        //        "5:                      \n"
        //        : "=a" (result),              
        //          [key1] "+r" (key1),     
        //          [key2] "+r" (key2),
        //          [len]  "+r" (len)
        //        :                            
        //        : "ecx", "memory", "cc"      
        //    );

        //    if (result) {
        //        node_data->count++;
        //        return node_data->key_pointer;
        //    }
        //}
        //current_index = node->next;
    //}

    return NULL;
}


// static --------------------------------------------------------------------------------------------------------------


static bool compareKeys(const char* key1, const char* key2, int length)
{
    assert(key1 != NULL);
    assert(key2 != NULL);

    int i = 0;
    for (; i + 4 <= length; i+=4)
    {
        __m128i a = _mm_loadu_si32(key1 + i);
        __m128i b = _mm_loadu_si32(key2 + i);
        __m128i cmp_result = _mm_cmpeq_epi8(a, b);
        if ((_mm_movemask_epi8(cmp_result) & 0xF) != 0xF)
        {
            return false; 
        }
    }

    for (; i < length; i++)
    {
        if (key1[i] != key2[i]) 
        {
            return false; 
        }
    }
    
    return true;
}


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

    //int* new_lengths_array = (int*)realloc(list->lengths, list->capacity * sizeof(int));
    //if (!new_lengths_array)
    //{
    //    fprintf(stderr, "Error allocating memory for lengths\n");
    //    return ListOperationError_ERROR;
    //}

    //list->lengths = new_lengths_array;

    int index = list->free_node;
    int simd_size = 4;
    for (; index  < (int)(list->capacity - simd_size + 1); index += simd_size)
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


    //for (size_t i = list->free_node; i < list->capacity; i++) {
    //    list->node_array[i].next = i + 1;
    //    list->node_array[i].prev = 0;
    //}

    return ListOperationError_SUCCESS;
}
