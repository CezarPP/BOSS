/*
 * kalloc_tests.h
 *
 *  Created on: 6/3/24.
 *      Author: Cezar PP
 */


#pragma once

#include "../std/new_custom.h"

namespace kalloc::tests {
    struct MyObject {
        size_t x{}, y{}, z{};
    };

    /// Test allocating a single object and freeing it
    void testAllocateSingle() {
        Logger::instance().println("[KALLOC] Allocating 1");
        auto *addr = new MyObject();
        // kAssert(addr == kMemBase, "Allocated address does not match expected start address");


        Logger::instance().println("[KALLOC] Freeing 1");
        delete addr;
        Logger::instance().println("[KALLOC] Allocating 1 again");
        auto *addr2 = new MyObject();
        kAssert(addr2 == addr, "[KALLOC] Allocator did not reuse the freed page");

        delete addr2; // clean up
    }

    /// Test allocating multiple pages and check if they are consecutive
    void testAllocateMultiplePages() {
        auto *addr = (uint8_t *) new MyObject[4];
        // kAssert(addr == kMemBase, "First allocated address incorrect");

        // Verify that allocations are consecutive
        auto *addrNext = (uint8_t *) new MyObject[4];
        Logger::instance().println("[KALLOC] addr: %X, addrNext: %X", addr, addrNext);
        auto *expectedAddress = addr + 4 * sizeof(MyObject) + sizeof(MemoryChunk);
        Logger::instance().println("[KALLOC] Expected addrNext: %X", expectedAddress);
        kAssert(addrNext == expectedAddress, "[KALLOC] Subsequent allocation is not consecutive");

        delete[] addr;
        delete[] addrNext; // Cleanup
    }

    /// Test edge case where allocator has to wrap around to find free space
    void testEdgeWraparound() {
        auto *addr1 = new MyObject[2];
        auto *addr2 = new MyObject[1];
        delete[] addr1; // Free the first 2 objects

        auto *addr3 = new MyObject[2];
        kAssert(addr3 == addr1, "[KALLOC] Allocator failed to wrap around correctly");

        delete[] addr2;
        delete[] addr3; // Cleanup
    }

    void runAllTests() {
        Logger::instance().println("[KALLOC] Running testAllocateSingle...");
        testAllocateSingle();
        Logger::instance().println("[KALLOC] Success!");

        Logger::instance().println("[KALLOC] Running testAllocateMultiplePages...");
        testAllocateMultiplePages();
        Logger::instance().println("[KALLOC] Success!");

        Logger::instance().println("[KALLOC] Running testEdgeWraparound...");
        testEdgeWraparound();
        Logger::instance().println("[KALLOC] Success!");
    }
}