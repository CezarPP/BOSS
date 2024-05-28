/*
 * virtual_allocator.h
 *
 *  Created on: 4/9/24.
 *      Author: Cezar PP
 */

#pragma once

#include "p_allocator_tests.h"

namespace virtual_allocator {
    class VirtualAllocator {
    private:
        /// Physical allocator type can be changed here with no additional modifications
        physical_allocator::BitmapAllocator physicalAllocator;
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
            physical_allocator::tests::runAllTests(alloc.physicalAllocator);


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
                : physicalAllocator(memBase, memSize),
                  KERNEL_VIRTUAL_START{
                          paging::PHYSICAL_ALLOCATOR_VIRTUAL_START +
                          physicalAllocator.cntAllocatorPages * paging::PAGE_SIZE} {
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
            size_t physicalAddress = physicalAllocator.allocate(pages);
            size_t virtualAddressStart = KERNEL_VIRTUAL_START + (physicalAddress - physicalAllocator.memBase);
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
            size_t physicalAddress = physicalAllocator.memBase + (vir - KERNEL_VIRTUAL_START);

            /// Unmap the pages from the virtual address space
            paging::unmapPages(vir, pages);

            /// Free the pages in the physical allocator
            physicalAllocator.free(physicalAddress, pages);
        }
    };

    VirtualAllocator *VirtualAllocator::instance_ = nullptr;

    /**
     * Uses the virtual allocator to allocate for std containers for kAlloc
     * After we have a working kAlloc, we can use std::allocator and ::operator new
     * @tparam T
     */
    template<typename T>
    class virtualStdAllocator {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        virtualStdAllocator() = default;

        virtualStdAllocator(const virtualStdAllocator &) noexcept = default;  // Copy constructor

        ~virtualStdAllocator() = default;  // Destructor

        [[nodiscard]] T *allocate(size_type n) {
            kAssert(n > 0, "Should allocate something");
            auto pagesToAllocate = physical_allocator::toPages(n * sizeof(T));
            T *p = static_cast<T *>(virtual_allocator::VirtualAllocator::instance()->vAlloc(pagesToAllocate));
            return p;
        }

        void deallocate(T *p, size_type n) noexcept {
            kAssert(n > 0, "Should deallocate something");
            auto pagesToDeallocate = physical_allocator::toPages(n * sizeof(T));
            virtual_allocator::VirtualAllocator::instance()->vFree(p, pagesToDeallocate);
        }
    };
}