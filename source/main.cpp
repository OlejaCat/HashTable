#include <stdio.h>

#include "text_processing.h"
#include "hash_table.h"
#include "my_memcmp.h"


const size_t NUMBER_OF_SEARCHES = 1e7;


int main()
{
    srand(42);

    Text text_build = {};
    if (textLoad(&text_build, "big_file_preprocessed.txt"))
    {
        fprintf(stderr, "Could not load text\n");
        return 1; 
    }

    HashTable* hash_table = hashTableCtor();

    char* word_pointer = NULL;
    int length = 0;
    int max_length = -1;
    while((length = textGetNextLine(&text_build, &word_pointer)))
    {
        if (max_length < length)
        {
            max_length = length; 
        }
        hashTableSet(hash_table, word_pointer, length);
    }

    printf("Words: %zu. Unigue words: %zu\nMax word len: %d\n", text_build.word_count, hashTableGetLength(hash_table), max_length);

    Text text_random = {};
    if (textLoad(&text_random, "random_words.txt"))
    {
        fprintf(stderr, "Could not load text\n");
        return 1; 
    }

    size_t rand_sum = 0;
    for (size_t i = 0; i < NUMBER_OF_SEARCHES; i++)
    {
        int length = textGetNextLine(&text_random, &word_pointer);
        rand_sum += hashTableGet(hash_table, word_pointer, length);
    }

    printf("Random quantity of words: %zu\n", rand_sum);
    textDtor(&text_random);

    hashTableDtor(hash_table);
    textDtor(&text_build);
    return 0;
}
