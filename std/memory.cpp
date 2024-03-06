#include "std/memory.h"

void *memcpy(void *dest, const void *src, size_t n) {
    auto d = static_cast<char *>(dest);
    auto s = static_cast<const char *>(src);
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

void *memset(void *s, int c, size_t n) {
    auto p = static_cast<unsigned char *>(s);
    for (size_t i = 0; i < n; ++i) {
        p[i] = static_cast<unsigned char>(c);
    }
    return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    auto a = static_cast<const unsigned char *>(s1);
    auto b = static_cast<const unsigned char *>(s2);
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) {
            return a[i] < b[i] ? -1 : 1;
        }
    }
    return 0;
}
