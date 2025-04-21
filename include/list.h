#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>

typedef enum ListOperationError
{
    ListOperationError_SUCCESS      =  0,
    ListOperationError_ERROR        =  1,
    ListOperationError_MEMORY_ERROR =  2,
} ListOperationError;

typedef struct Node
{
    // Data for hash table
    const char* key_pointer;
    size_t length;
    size_t hash;
    size_t count;

    // Data needed for list work
    size_t next;
    size_t prev;
} Node;

typedef struct List
{
    Node*  node_array;

    size_t free_node;
    size_t capacity;
    size_t size;
} List;


size_t getNextIndex(List* list, size_t index);
size_t getPreviousIndex(List* list, size_t index);

ListOperationError listCtor(List* list, size_t capacity);
ListOperationError listDtor(List* list);

int listInsert(List* list, size_t index);
int listInsertTail(List* list);
int listInsertHead(List* list);

ListOperationError listDeleteElement(List* list, size_t index);
ListOperationError listDeleteTail(List* list);
ListOperationError listDeleteHead(List* list);

#endif // LIST_H
