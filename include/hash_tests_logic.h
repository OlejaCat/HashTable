#ifndef HASH_TESTS_LOGIC_H
#define HASH_TESTS_LOGIC_H

#include <stdio.h>
#include <stdlib.h>

typedef struct HashTestResult
{
    long long time_ns;
    double    variance;
    size_t    bucket_size;
    size_t*   buckets; 
} HashTestResult;

int generateTests();
int testAllStringHashes(size_t bucket_size);

#endif // HASH_TESTS_LOGIC_H

