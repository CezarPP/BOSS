/*
 * new.h
 *
 *  Created on: 4/13/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/types.h"

// Placement new
inline void *operator new(size_t, void *place) noexcept {
    return place;
}

inline void *operator new[](size_t, void *place) noexcept {
    return place;
}