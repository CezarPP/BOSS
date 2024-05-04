/*
 * virtual_allocator.h
 *
 *  Created on: 4/9/24.
 *      Author: Cezar PP
 */

#pragma once

#include "p_allocator_tests.h"

class VirtualAllocator {
private:
    /// Physical allocator type can be changed here with no additional modifications
    physical_allocator::BitmapAllocator allocator;
    /// The virtual address where the kernel can start allocating memory
    uint64_t KERNEL_VIRTUAL_START;
    static VirtualAllocator *instance_; /// Pointer to the singleton instance
public:
    VirtualAllocator(const VirtualAllocator &) = delete;

    VirtualAllocator &operator=(const VirtualAllocator &) = delete;

    VirtualAllocator(VirtualAllocator &&) = delete;

    VirtualAllocator &operator=(VirtualAllocator &&) = delete;

    static VirtualAllocator *instance() {
        return instance_;
    }

    static void init(size_t memBase, size_t memSize) {
        static VirtualAllocator alloc(memBase, memSize);
        instance_ = &alloc;

        Logger::instance().println("[V_ALLOC] Testing physical allocator...");
        physical_allocator::tests::runAllTests(alloc.allocator);


        Logger::instance().println("[V_ALLOC] Testing...");
        char *myBigString = (char *) instance_->vAlloc(1);
        for (size_t i = 0; i < paging::PAGE_SIZE; i++)
            myBigString[i] = 'A';

        char *myBigString2 = (char *) instance_->vAlloc(2);
        for (size_t i = 0; i < paging::PAGE_SIZE * 2; i++)
            myBigString2[i] = 'A';

        instance_->vFree(myBigString, 1);

        // Should be the same as myBigString
        char *myBigString3 = (char *) instance_->vAlloc(1);
        kAssert(myBigString == myBigString3, "[V_ALLOC] These addresses should be the same");

        instance_->vFree(myBigString2, 2);
        instance_->vFree(myBigString3, 1);

        Logger::instance().println("[V_ALLOC] Finished testing.");
    }

    VirtualAllocator(size_t memBase, size_t memSize)
            : allocator(memBase, memSize),
              KERNEL_VIRTUAL_START{
                      paging::PHYSICAL_ALLOCATOR_VIRTUAL_START + allocator.cntAllocatorPages * paging::PAGE_SIZE} {
        Logger::instance().println("[V_ALLOC] Kernel Virtual start is %X", KERNEL_VIRTUAL_START);
    }


    /*!
     * Similar to the Linux version, allocates pages continuous in virtual memory
     * Pages are not necessarily continuous in physical memory
     * Future work: At the time, it actually allocated them continuous in physical memory
     *              Could map them differently to have non-continuous physical memory
     * @param pages The number of pages to allocate
     * @return The virtual address of the first page
     */
    void *vAlloc(size_t pages) {
        size_t physicalAddress = allocator.allocate(pages);
        size_t virtualAddressStart = KERNEL_VIRTUAL_START + (physicalAddress - allocator.memBase);
        paging::mapPages(virtualAddressStart, physicalAddress, pages);

        return reinterpret_cast<void *>(virtualAddressStart);
    }

    /*!
     *
     * @param virtualAddress The virtual address of the first page to free
     * @param pages The number of pages to free
     * @return
     */
    void vFree(void *virtualAddress, size_t pages) {
        auto vir = reinterpret_cast<uint64_t>(virtualAddress);
        /// Convert the virtual address back to a physical address
        size_t physicalAddress = allocator.memBase + (vir - KERNEL_VIRTUAL_START);

        /// Unmap the pages from the virtual address space
        paging::unmapPages(vir, pages);

        /// Free the pages in the physical allocator
        allocator.free(physicalAddress, pages);
    }
};

VirtualAllocator *VirtualAllocator::instance_ = nullptr;