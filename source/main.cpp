#include <stdio.h>

#include "text_processing.h"
#include "hash_table.h"


int main()
{
    Text text = {};
    if (textLoad(&text, "test.txt"))
    {
        printf("Out\n");
        return 1; 
    }

    char buffer[50] = {};
    int c = 0;
    while((c = textNextWord(&text, buffer, sizeof(buffer))))
    {
        printf("%s %d\n", buffer, c);
    }
    
    textDtor(&text);

    const char* name = "Alice";

    HashTable* table = hashTableCtor();
    hashTableSet(table, "apple", (void*)"RED");
    hashTableSet(table, "banana", (void*)"YELLOW");
    hashTableSet(table, "orange", (void*)"ORANGE");
    hashTableSet(table, "nigga", (void*)"BLACKDKDS");

    hashTableDelete(table, "banana");


    HashTableIterator iterator = hashTableIterator(table);
    while (hashTableNext(&iterator))
    {
        printf("key: %s value: %s\n", iterator.key, (char*)iterator.value);
    }

    hashTableDtor(table);

    return 0;
}
