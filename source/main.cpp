#include <stdio.h>

#include "text_processing.h"
#include "hash_table.h"
#include "my_memcmp.h"


int main()
{
    Text text = {};
    if (textLoad(&text, "big_file.txt"))
    {
        fprintf(stderr, "Could not load text\n");
        return 1; 
    }

    HashTable* hash_table = hashTableCtor();

    char* word_pointer = NULL;
    int length = 0;
    int max_length = -1;
    while((length = textNextWordPointer(&text, &word_pointer)))
    {
        if (max_length < length)
        {
            max_length = length; 
        }
        hashTableSet(hash_table, word_pointer, length);
    }

//    HashTableIterator iterator = hashTableIterator(hash_table);    
//    while (hashTableNext(&iterator))
//    {        
//        printf("%.*s %zu\n", (int)iterator.length, iterator.key, iterator.count);
//    }

    printf("Words: %zu. Unigue words: %zu\nMax word len: %d\n", text.word_count, hashTableGetLength(hash_table), max_length);

    hashTableDtor(hash_table);
    textDtor(&text);
    return 0;
}
