#include <stdio.h>

#include "text_processing.h"
#include "hash_table.h"
#include "my_memcmp.h"


const size_t NUMBER_OF_SEARCHES = 1e7;


int main()
{
    srand(42);

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

    printf("Words: %zu. Unigue words: %zu\nMax word len: %d\n", text.word_count, hashTableGetLength(hash_table), max_length);

    size_t rand_sum = 0;
    for (size_t i = 0; i < NUMBER_OF_SEARCHES; i++)
    {
        int length = textGetRandomWord(&text, &word_pointer);
        rand_sum += hashTableGet(hash_table, word_pointer, length);
    }

    printf("Random quantity of words: %zu\n", rand_sum);

    hashTableDtor(hash_table);
    textDtor(&text);
    return 0;
}
