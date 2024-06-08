/*
 * boss_new.h
 *
 *  Created on: 5/6/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/types.h"

void *operator new(uint64_t size);

void *operator new[](uint64_t size);

void operator delete(void *p) noexcept;

void operator delete[](void *p) noexcept;

// Sized de-allocation
/* The compiler is flagging that my custom delete operators are missing a variant that accepts a size parameter.
 * This is a C++14 feature that allows the size of the object being deleted to be passed to the delete operator,
 * which can be useful for optimizations. */
void operator delete(void *p, uint64_t) noexcept;

void operator delete[](void *p, uint64_t) noexcept;
