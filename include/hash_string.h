#ifndef HASH_STRING_H
#define HASH_STRING_H

#include <stdint.h>
#include <string.h>
#include <x86intrin.h>

const uint32_t BASE_FOR_POLYNOMIAL = 13;

typedef uint32_t (*hashStringFunction)(const char*, size_t, size_t);

uint32_t hash_string_length(const char* str, size_t len, size_t size)
{
    return len % size;
}

uint32_t hash_string_sum(const char* str, size_t len, size_t size)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++)
    {
        sum += str[i];
    }
    return sum % size;
}

uint32_t hash_string_polynomial(const char* str, size_t len, size_t size)
{
    uint32_t hash = 0;
    for (size_t i = 0; i < len; i++)
    {
        hash = (hash * BASE_FOR_POLYNOMIAL + (str[i] - 'a' + 1)) & (size - 1);
    }

    return hash & (size - 1);
}


uint32_t hash_string_crc32(const char* str, size_t len, size_t size)
{
    uint32_t crc = 0xFFFFFFFF;
    size_t i = 0;
    uint64_t chunk;
    while (i + 8 < len)
    {
        memcpy(&chunk, str + i, sizeof(chunk));
        crc = _mm_crc32_u64(crc, chunk);
        i+=8;
    }

    while (i < len)
    {
        crc = _mm_crc32_u8(crc, str[i++]);
    }

    return (~crc) & (size - 1);

}


uint32_t hash_string_fnv_1a(const char* str, size_t len, size_t size)
{
    const uint32_t FNV_prime = 0x01000193;
    const uint32_t FNV_offset_basis = 0x811C9DC5;
    uint32_t hash = FNV_offset_basis;

    uint32_t chunk;
    while (len >= 4) 
    {
        memcpy(&chunk, str, 4);
        hash ^= chunk;
        hash *= FNV_prime;
        str += 4;
        len -= 4;
    }

    while (len--) 
    {
        hash ^= (uint32_t)*str++;
        hash *= FNV_prime;
    }

    return hash & (size - 1);
}


uint32_t hash_string_djb2(const char* str, size_t len, size_t size)
{
    uint32_t hash = 5381;
    size_t i = 0;

    while (i < len)
        hash = ((hash << 5) + hash) + str[i++]; 

    return hash & (size - 1);
}


uint32_t hash_string_sdbm(const char* str, size_t len, size_t size)
{
    uint32_t hash = 0;
    size_t i = 0;

    while (i < len)
        hash = (str[i++]) + (hash << 6) + (hash << 16) - hash;

    return hash & (size - 1);
}


#endif // HASH_STRING_H

