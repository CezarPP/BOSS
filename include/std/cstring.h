#pragma once

#include <stdint.h>

typedef uint64_t size_t;

void *memcpy(void *dest, const void *src, size_t n);

void *memset(void *s, int c, size_t n);

int memcmp(const void *s1, const void *s2, size_t n);

inline uint64_t strlen(const char *a) {
    uint64_t length = 0;
    while (*a++) {
        ++length;
    }
    return length;
}