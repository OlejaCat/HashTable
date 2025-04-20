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

    list->node_array[0] = {
        .data = NULL,
        .next = 0,
        .prev = 0,
    };

    for (size_t index = 1; index <= capacity; index++)
    {
        list->node_array[index] = {
            .data = NULL,
            .next = index + 1,
            .prev = 0,
        };
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
    memset(list, 0, sizeof(List));

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


ListOperationError listInsert(List* list, size_t index, void* element)
{
    assert(list    != NULL);
    assert(element != NULL);

    if (index > list->capacity)
    {
        return ListOperationError_ERROR;
    }

    size_t real_index = list->free_node;
    if (real_index > list->capacity)
    {
        listResize(list);
    }

    list->free_node = list->node_array[real_index].next;

    list->node_array[real_index] = {
        .data = element,
        .next = list->node_array[index].next,
        .prev = index,
    };

    list->node_array[list->node_array[index].next].prev = real_index;
    list->node_array[index].next = real_index;

    list->size++;

    return ListOperationError_SUCCESS;
}


ListOperationError listInsertHead(List* list, void* element)
{
    assert(list != NULL);

    listInsert(list, 0, element);

    return ListOperationError_SUCCESS;
}


ListOperationError listInsertTail(List* list, void* element)
{
    assert(list != NULL);

    listInsert(list, getPreviousIndex(list, 0), element);

    return ListOperationError_SUCCESS;
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

    list->node_array[index] = {
        .data   = NULL,
        .next   = list->free_node,
        .prev   = 0,
    };

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


static ListOperationError listResize(List* list) {
    assert(list != NULL);

    size_t old_capacity = list->capacity;
    list->capacity *= SCALE_FACTOR;

    Node* new_array = (Node*)realloc(list->node_array, (list->capacity + 1) * sizeof(Node));
    if (new_array == NULL) return ListOperationError_ERROR;
    list->node_array = new_array;

    for (size_t i = old_capacity + 1; i <= list->capacity; i++) {
        list->node_array[i] = {
            .data = NULL,
            .next = (i == list->capacity) ? 0 : i + 1, 
            .prev = 0,
        };
    }

    size_t current_free = list->free_node;
    if (current_free != 0) {
        while (list->node_array[current_free].next != 0 && 
               list->node_array[current_free].next <= old_capacity) 
        {
            current_free = list->node_array[current_free].next;
        }
    } else {
        current_free = old_capacity + 1;
    }

    list->node_array[current_free].next = old_capacity + 1;

    return ListOperationError_SUCCESS;
}
