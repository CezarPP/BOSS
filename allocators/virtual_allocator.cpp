/*
 * virtual_allocator.cpp
 *
 *  Created on: 6/8/24.
 *      Author: Cezar PP
 */

#include "allocators/virtual_allocator.h"

namespace virtual_allocator {
    VirtualAllocator *VirtualAllocator::instance_ = nullptr;

    VirtualAllocator *VirtualAllocator::instance() {
        return instance_;
    }

    void VirtualAllocator::init(size_t memBase, size_t memSize) {
        static VirtualAllocator alloc(memBase, memSize);
        instance_ = &alloc;

        Logger::instance().println("[V_ALLOC] Testing physical allocator...");

        physical_allocator::tests::PhysicalAllocatorTester physicalAllocatorTester(alloc.physicalAllocator);
        physicalAllocatorTester.runAllTests();

        Logger::instance().println("[V_ALLOC] Testing...");
        char *myBigString = (char *) instance_->vAlloc(1);
        for (size_t i = 0; i < paging::PAGE_SIZE; i++)
            myBigString[i] = 'A';

        char *myBigString2 = (char *) instance_->vAlloc(2);
        for (size_t i = 0; i < paging::PAGE_SIZE * 2; i++)
            myBigString2[i] = 'A';

        instance_->vFree(myBigString, 1);
        char *myBigString3 = (char *) instance_->vAlloc(1);
        kAssert(myBigString == myBigString3, "[V_ALLOC] These addresses should be the same");

        instance_->vFree(myBigString2, 2);
        instance_->vFree(myBigString3, 1);

        Logger::instance().println("[V_ALLOC] Finished testing.");
    }

    VirtualAllocator::VirtualAllocator(size_t memBase, size_t memSize)
            : physicalAllocator(memBase, memSize),
              KERNEL_VIRTUAL_START(paging::PHYSICAL_ALLOCATOR_VIRTUAL_START +
                                   physicalAllocator.cntAllocatorPages * paging::PAGE_SIZE) {
        Logger::instance().println("[V_ALLOC] Kernel Virtual start is %X", KERNEL_VIRTUAL_START);
    }

    void *VirtualAllocator::vAlloc(size_t pages) {
        Logger::instance().println("[V_ALLOC] Allocating %X pages...", pages);
        size_t physicalAddress = physicalAllocator.allocate(pages);
        size_t virtualAddressStart = KERNEL_VIRTUAL_START + (physicalAddress - physicalAllocator.memBase);
        paging::mapPages(virtualAddressStart, physicalAddress, pages);
        return reinterpret_cast<void *>(virtualAddressStart);
    }

    void VirtualAllocator::vFree(void *virtualAddress, size_t pages) {
        auto vir = reinterpret_cast<uint64_t>(virtualAddress);
        size_t physicalAddress = physicalAllocator.memBase + (vir - KERNEL_VIRTUAL_START);
        paging::unmapPages(vir, pages);
        physicalAllocator.free(physicalAddress, pages);
    }
}
