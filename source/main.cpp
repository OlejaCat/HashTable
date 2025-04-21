#include <stdio.h>

#include "text_processing.h"
#include "hash_table.h"


int main()
{
    char book[256] = {};

    printf("Enter name of file: ");
    scanf("%s", book);

    Text text = {};
    if (textLoad(&text, book))
    {
        fprintf(stderr, "Could not load text\n");
        return 1; 
    }

    HashTable* hash_table = hashTableCtor();

    char* word_pointer = NULL;
    int length = 0;
    while((length = textNextWordPointer(&text, &word_pointer)))
    {
        hashTableSet(hash_table, word_pointer, length);
    }

    // HashTableIterator iterator = hashTableIterator(hash_table);    
    // while (hashTableNext(&iterator))
    // {        
    //    printf("%.*s %zu\n", (int)iterator.length, iterator.key, iterator.count);
    //}

    hashTableDtor(hash_table);
    textDtor(&text);

    return 0;
}
