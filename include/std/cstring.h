#pragma once

#include <stdint.h>

typedef uint64_t size_t;

inline int strncmp(const char *str1, const char *str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        ++str1;
        ++str2;
        --n;
    }
    if (n == 0) {
        return 0;
    } else {
        return (*(unsigned char *) str1 - *(unsigned char *) str2);
    }
}

inline void *memcpy(void *dest, const void *src, size_t n) {
    auto d = static_cast<char *>(dest);
    auto s = static_cast<const char *>(src);
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

inline void *memset(void *s, int c, size_t n) {
    auto p = static_cast<unsigned char *>(s);
    for (size_t i = 0; i < n; ++i) {
        p[i] = static_cast<unsigned char>(c);
    }
    return s;
}

inline int memcmp(const void *s1, const void *s2, size_t n) {
    auto a = static_cast<const unsigned char *>(s1);
    auto b = static_cast<const unsigned char *>(s2);
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return a[i] < b[i] ? -1 : 1;
        }
    }
    return 0;
}

inline uint64_t strlen(const char *a) {
    uint64_t length = 0;
    while (*a++) {
        ++length;
    }
    return length;
}

inline char *strcpy(char *dest, const char *src) {
    char *start = dest;

    // Copy each character from source to destination
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }

    // Don't forget to add null terminator
    *dest = '\0';

    // Return the starting address of the destination
    return start;
}