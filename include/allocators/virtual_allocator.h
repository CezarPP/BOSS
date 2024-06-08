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
        physical_allocator::BitmapAllocator physicalAllocator;
        uint64_t KERNEL_VIRTUAL_START;
        static VirtualAllocator *instance_;

    public:
        VirtualAllocator(const VirtualAllocator &) = delete;

        VirtualAllocator &operator=(const VirtualAllocator &) = delete;

        VirtualAllocator(VirtualAllocator &&) = delete;

        VirtualAllocator &operator=(VirtualAllocator &&) = delete;

        static VirtualAllocator *instance();

        static void init(size_t memBase, size_t memSize);

        VirtualAllocator(size_t memBase, size_t memSize);

        void *vAlloc(size_t pages);

        void vFree(void *virtualAddress, size_t pages);
    };

    // Not to self: One shouldn't split templates into .h & .cpp
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
            kAssert(n > 0, "[V_ALLOC] This allocator should allocate something");
            auto pagesToAllocate = physical_allocator::toPages(n * sizeof(T));
            T *p = static_cast<T *>(virtual_allocator::VirtualAllocator::instance()->vAlloc(pagesToAllocate));
            return p;
        }

        void deallocate(T *p, size_type n) noexcept {
            kAssert(n > 0, "[V_ALLOC] This allocator should deallocate something");
            auto pagesToDeallocate = physical_allocator::toPages(n * sizeof(T));
            virtual_allocator::VirtualAllocator::instance()->vFree(p, pagesToDeallocate);
        }
    };
}
