/*
 * literals.h
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

#pragma once

#include "stdint.h"

typedef uint64_t size_t;

static_assert(sizeof(size_t) == sizeof(unsigned long long));

inline constexpr size_t operator "" _TB(unsigned long long n) {
    return n * 1024 * 1024 * 1024 * 1024; // 1 TB = 1024^4 bytes
}

inline constexpr size_t operator "" _GiB(unsigned long long n) {
    return n * 1024 * 1024 * 1024; // 1 GB = 1024^3 bytes
}

inline constexpr size_t operator "" _MiB(unsigned long long n) {
    return n * 1024 * 1024; // 1 MB = 1024^2 bytes
}

inline constexpr size_t operator "" _KiB(unsigned long long n) {
    return n * 1024; // 1 KB = 1024 bytes
}
