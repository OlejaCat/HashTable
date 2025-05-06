#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "messages.h"
#include "hash_tests_logic.h"

const size_t bucket_size = 2048;


int main(int argc, char** argv)
{
    if (argc <= 1)
    {
        printf("%s", HELP_MESSAGE);
        return 0;
    }

    int arg_number = 1;
    while (arg_number < argc)
    {
        if (!strcmp(argv[arg_number], HELP_FLAG))
        {
            printf("%s", HELP_MESSAGE);
            return 0;
        }

        if (!strcmp(argv[arg_number], GENERATE_TESTS))  
        {
            generateTests();
        }
        else if (!strcmp(argv[arg_number], RUN_STRING_TEST))
        {
            testAllStringHashes(bucket_size);
        }
        else
        {
            printf("%s", UNKNOWM_ARG);
            printf("%s", HELP_MESSAGE);
            return 0;
        }
        arg_number++;
    }

    return 0;
}

