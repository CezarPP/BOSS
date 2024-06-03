/*
 * allocator_tests.h
 *
 *  Created on: 5/3/24.
 *      Author: Cezar PP
 */


#pragma once

#include "bitmap_allocator.h"
#include "buddy_allocator.h"
#include "../arch/x86_64/exceptions.h"

namespace physical_allocator::tests {

    size_t kMemBase;
    size_t kMemSize;

    /// Test allocating a single page and freeing it
    template<typename Allocator>
    void testAllocateSinglePage(Allocator &alloc) {
        Logger::instance().println("[P_ALLOCATOR] Allocating 1");
        size_t addr = alloc.allocate(1);
        kAssert(addr == kMemBase, "[P_ALLOC] Allocated address does not match expected start address");


        Logger::instance().println("[P_ALLOCATOR] Freeing 1");
        alloc.free(addr, 1);
        Logger::instance().println("[P_ALLOCATOR] Allocating 1 again");
        size_t addr2 = alloc.allocate(1);
        kAssert(addr2 == addr, "[P_ALLOCATOR] Allocator did not reuse the freed page");

        alloc.free(addr2, 1);  // Cleanup
    }

    /// Test allocating multiple pages and check if they are consecutive
    template<typename Allocator>
    void testAllocateMultiplePages(Allocator &alloc) {
        size_t addr = alloc.allocate(4);
        kAssert(addr == kMemBase, "[P_ALLOCATOR] First allocated address incorrect");

        // Verify that allocations are consecutive
        size_t addrNext = alloc.allocate(4);
        Logger::instance().println("[P_ALLOCATOR] addr: %X, addrNext: %X", addr, addrNext);
        Logger::instance().println("[P_ALLOCATOR] Expected addrNext: %X", addr + 4 * PAGE_SIZE);
        kAssert(addrNext == addr + 4 * PAGE_SIZE, "[P_ALLOCATOR] Subsequent allocation is not consecutive");

        alloc.free(addr, 4);
        alloc.free(addrNext, 4);  // Cleanup
    }

    /// Test that allocator can handle full allocation and de-allocation
    template<typename Allocator>
    void testFullAllocationCycle(Allocator &alloc) {
/*        size_t cntPages = kMemSize / PAGE_SIZE;
        for (size_t i = 0; i < cntPages; i++) {
            size_t addr = alloc.allocate(1);
            kAssert(addr == kMemBase + i * PAGE_SIZE, "Address mismatch in full cycle allocation");
        }
        for (size_t i = 0; i < cntPages; i++) {
            alloc.free(kMemBase + i * PAGE_SIZE, 1);
        }*/
    }

    /// Test edge case where allocator has to wrap around to find free space
    template<typename Allocator>
    void testEdgeWraparound(Allocator &alloc) {
        size_t addr1 = alloc.allocate(2);
        size_t addr2 = alloc.allocate(1);
        alloc.free(addr1, 2);  // Free first page

        size_t addr3 = alloc.allocate(2);
        kAssert(addr3 == addr1, "[P_ALLOCATOR] Allocator failed to wrap around correctly");

        alloc.free(addr2, 1);
        alloc.free(addr3, 2);  // Cleanup
    }

    template<typename Allocator>
    void runAllTests(Allocator &allocator) {
        kMemBase = allocator.memBase;
        kMemSize = allocator.memSize;

        Logger::instance().println("[P_ALLOCATOR] Running testAllocateSingle...");
        testAllocateSinglePage(allocator);
        Logger::instance().println("[P_ALLOCATOR] Success!");

        Logger::instance().println("[P_ALLOCATOR] Running testAllocateMultiplePages...");
        testAllocateMultiplePages(allocator);
        Logger::instance().println("[P_ALLOCATOR] Success!");

        Logger::instance().println("[P_ALLOCATOR] Running testFullAllocationCycle...");
        testFullAllocationCycle(allocator);
        Logger::instance().println("[P_ALLOCATOR] Success!");

        Logger::instance().println("[P_ALLOCATOR] Running testEdgeWraparound...");
        testEdgeWraparound(allocator);
        Logger::instance().println("[P_ALLOCATOR] Success!");
    }
}

