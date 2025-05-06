#include "hash_tests_logic.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hash_string.h"
#include "text_processing.h"


// static ----------------------------------------------------------------------


#define SRAND_SEED 42
#define MAX_LEN 20
#define NUMBER_OF_TESTS 20000
#define DEFAULT_BUCKET_SIZE 2048

static int generateStringTest(const char* input_file, 
                              FILE*       output_file);

static int testStringHash(hashStringFunction hash_func, 
                          size_t             bucket_size,
                          const char*        name_of_test,
                          const char*        output_path);

static long long getNanoTime();
static double calculateVariance(const size_t* buckets, 
                                size_t        bucket_size, 
                                size_t        total);

static int writeHashTestResults(HashTestResult hash_results, 
                                const char* filename, 
                                const char* title);

const char STRING_TEST_FILE[] = "tests/test_string.txt";


// public ----------------------------------------------------------------------


int generateTests()
{
    srand(SRAND_SEED);

    // Generate string tests
    FILE* test_file = fopen(STRING_TEST_FILE, "w");
    if (!test_file) 
    {
        fprintf(stderr, "Error opening string test file\n");
        return 1;
    }

    if (generateStringTest("big_file.txt",
                           test_file)) 
    {
        fclose(test_file);
        return 1;
    }
    fclose(test_file);

    return 0;
}


int testAllStringHashes(size_t bucket_size)
{
    testStringHash(hash_string_length,     bucket_size, "String length",          "results/hash_string_length.txt");
    testStringHash(hash_string_sum,        bucket_size, "String sum of elements", "results/hash_string_sum.txt");
    testStringHash(hash_string_polynomial, bucket_size, "String polynomial",      "results/hash_string_polynomial.txt");
    testStringHash(hash_string_crc32,      bucket_size, "String crc32",           "results/hash_string_crc32.txt");
    testStringHash(hash_string_fnv_1a,     bucket_size, "String FNV-1a",          "results/hash_string_fnv-1a.txt");
    testStringHash(hash_string_djb2,       bucket_size, "String DJB2",            "results/hash_string_djb2.txt");
    testStringHash(hash_string_sdbm,       bucket_size, "String SDBM",            "results/hash_string_sdbm.txt");

    return 0;
}


// static ----------------------------------------------------------------------


static int testStringHash(hashStringFunction hash_func, 
                          size_t             bucket_size,
                          const char*        name_of_test,
                          const char*        output_path)
{
    if (bucket_size == 0)
    {
        bucket_size = DEFAULT_BUCKET_SIZE;
    }

    FILE* file = fopen(STRING_TEST_FILE, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening %s\n", STRING_TEST_FILE);
        return 1;
    }

    size_t* buckets = (size_t*)calloc(bucket_size, sizeof(size_t));
    if (buckets == NULL)
    {
        fclose(file);
        fprintf(stderr, "Failed to allocate buckets for string test\n");
        return 1;
    }

    long long result_time = 0;
    char buffer[MAX_LEN + 1];
    int items_read = -1;
    
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n')
        {
            buffer[--len] = '\0';
        }

        if (len > MAX_LEN)
        {
            continue;
        }

        const long long start_time = getNanoTime();
        size_t hash = hash_func(buffer, len, bucket_size);
        const long long end_time = getNanoTime();

        result_time += end_time - start_time;
        buckets[hash]++;
        items_read++;
    }

    fclose(file);

    HashTestResult result = {};
    result.time_ns = result_time;
    result.variance = calculateVariance(buckets, bucket_size, items_read);
    result.bucket_size = bucket_size;
    result.buckets = buckets;

    writeHashTestResults(result, output_path, name_of_test);
    free(buckets);
    
    return 0;
}


static int generateStringTest(const char* input_file, 
                              FILE*       output_file)
{
    assert(input_file  != NULL);
    assert(output_file != NULL);

    Text text = {};
    if (textLoad(&text, input_file))
    {
        fprintf(stderr, "Could not load text\n");
        return 1; 
    }

    char* word_pointer = NULL;
    for (size_t i = 0; i < NUMBER_OF_TESTS; i++)
    {
        int length = textGetRandomWord(&text, &word_pointer);
        fprintf(output_file, "%.*s\n", length, word_pointer);
    }

    return 0;
}


static double calculateVariance(const size_t* buckets, size_t bucket_size, size_t total)
{
    const double mean = total / (double)bucket_size;
    double variance = 0.0;

    for (size_t i = 0; i < bucket_size; i++)
    {
        const double diff = buckets[i] - mean;
        variance += diff * diff;
    }

    return variance / bucket_size;
}


static long long getNanoTime()
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
        return -1;
    }
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}


static int writeHashTestResults(HashTestResult hash_results, 
                                const char* filename,
                                const char* title)
{
    FILE* file = fopen(filename, "w");
    if (!file)
    {
        fprintf(stderr, "Couldnt open file: %s\n", filename);
        return 1; 
    }

    fprintf(file, "%zu %lf %lld\n", 
            hash_results.bucket_size, 
            hash_results.variance, 
            hash_results.time_ns);
    fprintf(file, "%s\n", title);
    size_t* buckets = hash_results.buckets;
    for (size_t i = 0; i < hash_results.bucket_size; i++)
    {
        fprintf(file, "%zu\n", buckets[i]);
    }

    return 0;
}

