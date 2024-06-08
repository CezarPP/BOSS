/*
 * new_custom.cpp
 *
 *  Created on: 6/8/24.
 *      Author: Cezar PP
 */

#include "allocators/kalloc.h"

void *operator new(uint64_t size) {
    return kalloc::kAlloc(size);
}

void *operator new[](uint64_t size) {
    return kalloc::kAlloc(size);
}

void operator delete(void *p) noexcept {
    kalloc::kFree(p);
}

void operator delete[](void *p) noexcept {
    return kalloc::kFree(p);
}

void operator delete(void *p, uint64_t) noexcept {
    kalloc::kFree(p);
}

void operator delete[](void *p, uint64_t) noexcept {
    kalloc::kFree(p);
}