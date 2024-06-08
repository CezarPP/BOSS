/*
 * kalloc_tests.h
 *
 *  Created on: 6/3/24.
 *      Author: Cezar PP
 */


#pragma once

#include "util/types.h"

namespace kalloc::tests {
    struct MyObject {
        size_t x{}, y{}, z{};
    };

    /// Test allocating a single object and freeing it
    void testAllocateSingle();

    /// Test allocating multiple pages and check if they are consecutive
    void testAllocateMultiplePages();

    /// Test edge case where allocator has to wrap around to find free space
    void testEdgeWraparound();

    void runAllTests();
}