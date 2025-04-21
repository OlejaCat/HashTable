#include "list.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>


// static --------------------------------------------------------------------------------------------------------------


#define SCALE_FACTOR 2

static ListOperationError listResize(List* list);


// public --------------------------------------------------------------------------------------------------------------


// -------------------------------  CTOR AND DTOR  -------------------------------


ListOperationError listCtor(List* list, size_t capacity)
{
    assert(list != NULL);

    list->capacity = capacity;

    list->node_array = (Node*)calloc(capacity + 1, sizeof(Node));
    if (list->node_array == NULL)
    {
        fprintf(stderr, "Error allocating node array for list\n");
        return ListOperationError_ERROR;
    }

    list->node_array[0].next = 0;
    list->node_array[0].prev = 0; 

    for (size_t index = 1; index <= capacity; index++)
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

    return ListOperationError_SUCCESS;
}


// -------------------------------  INDEX OPERATIONS -------------------------------


size_t getNextIndex(List* list, size_t index)
{
    assert(list != NULL);

    return list->node_array[index].next;
}


size_t getPreviousIndex(List* list, size_t index)
{
    assert(list != NULL);

    return list->node_array[index].prev;
}


// -------------------------------  INSERT OPERATIONS  -------------------------------


int listInsert(List* list, size_t index)
{
    assert(list != NULL);

    if (index > list->capacity)
    {
        return ListOperationError_ERROR;
    }

    if (list->free_node == 0
     || list->free_node >= list->capacity)
    {
        if (listResize(list) != ListOperationError_SUCCESS)
        {
            fprintf(stderr, "Error while resizing list\n");
            return 0;
        }
    }


    size_t real_index = list->free_node;

    assert(real_index <= list->capacity);
    list->free_node = list->node_array[real_index].next;

    assert(list->node_array[real_index].next <= list->capacity);
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


ListOperationError listDeleteElement(List* list, size_t index)
{
    assert(list != NULL);

    if (index > list->capacity)
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


// static --------------------------------------------------------------------------------------------------------------


static ListOperationError listResize(List* list) 
{
    assert(list != NULL);

    list->capacity *= SCALE_FACTOR;

    Node* new_array = (Node*)realloc(list->node_array, (list->capacity + 1) * sizeof(Node));
    if (!list->node_array)
    {
        fprintf(stderr, "Error while reallocating memory list\n");
        return ListOperationError_ERROR;
    }
    list->node_array = new_array;

    for (size_t i = list->free_node; i <= list->capacity; i++) {
        list->node_array[i].next = i + 1;
        list->node_array[i].prev = 0;
    }

    return ListOperationError_SUCCESS;
}
