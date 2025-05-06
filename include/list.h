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
    // Data needed for list work
    int next;
    int prev;
} Node;

typedef struct NodeData
{
    // Data for hash table
    const char* key_pointer;
    int length;
    int count;
} NodeData;

typedef struct List
{
    Node*     node_array;
    NodeData* data;
    //int*      lengths; 
    
    int free_node;
    size_t capacity;
    size_t size;
} List;


int getNextIndex(List* list, int index);
int getPreviousIndex(List* list, int index);

ListOperationError listCtor(List* list, size_t capacity);
ListOperationError listDtor(List* list);

int listInsert(List* list, int index);
int listInsertTail(List* list);
int listInsertHead(List* list);

ListOperationError listDeleteElement(List* list, int index);
ListOperationError listDeleteTail(List* list);
ListOperationError listDeleteHead(List* list);

const char* listIncrementValue(List* list, const char* key, size_t len);
size_t listGetValue(List* list, const char* key, size_t length);

#endif // LIST_H
