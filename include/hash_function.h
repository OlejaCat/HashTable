#ifndef HASH_FUNCTION_H
#define HASH_FUNCTION_H

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t crc_hash(const char* str, size_t len);

#if defined(__cplusplus)
}
#endif

#endif // HASH_FUNCTION_H
