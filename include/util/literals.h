/*
 * literals.h
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

#pragma once

#include "types.h"

static_assert(sizeof(size_t) == sizeof(unsigned long long));

inline constexpr size_t operator "" _GiB(unsigned long long n) {
    return n * 1024 * 1024 * 1024;
}

inline constexpr size_t operator "" _MiB(unsigned long long n) {
    return n * 1024 * 1024;
}

inline constexpr size_t operator "" _KiB(unsigned long long n) {
    return n * 1024;
}
