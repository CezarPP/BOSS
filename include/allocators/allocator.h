/*
 * physical_allocator.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */

#pragma once

#include "../arch/x86_64/phys_mm.h"
#include "../arch/x86_64/paging.h"
#include "std/cstddef.h"

namespace physical_allocator {
    constexpr auto PAGE_SIZE = paging::PAGE_SIZE;

    template<typename T>
    class physicalStdAllocator {
    private:
        T *p_;
        size_t memSize_, crtSize_ = 0;
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        physicalStdAllocator(T *p, size_t memSize) noexcept: p_(p), memSize_(memSize) {}

        physicalStdAllocator(const physicalStdAllocator &) noexcept = default;  // Copy constructor

        ~physicalStdAllocator() = default;  // Destructor

        [[nodiscard]] T *allocate(size_type n) {
            T *p = p_ + crtSize_;
            Logger::instance().println("Custom allocator is allocating %X objects at address %X...", n, p);
            // Allocate memory for n objects of type T
            kAssert(crtSize_ + n * sizeof(T) <= memSize_, "Overflowing physical allocator memory");
            crtSize_ += n * sizeof(T);
            return p;
        }

        void deallocate(T *p, size_type n) noexcept {
            // Deallocating 0 bytes is alright
            if(n != 0)
                kPanic("This memory should not be deallocated");
        }
    };

    template<class T1, class T2>
    constexpr bool operator==(const physicalStdAllocator<T1> &lhs, const physicalStdAllocator<T2> &rhs) noexcept {
        // All instances of allocator are interchangeable, hence always equal
        return true;
    }

    template<typename Derived>
    class Allocator {
    protected:
        /// The physical address of the start memory
        const size_t initialMemBase_;
        /// The size of the allocator's physical memory, in bytes
        const size_t initialMemSize_;
    public:
        /// The physical address of the start memory that is usable for allocations
        size_t memBase;
        /// The size of the physical memory that is usable for allocations
        size_t memSize;

        /// The start of the allocator's own memory, holds data structures specific to each allocator
        void *allocatorMemory;

        /// The number of pages of memory the allocator asked for (for its internal representation)
        size_t cntAllocatorPages;


        /*!
         * @param mem The number of bytes to be converted to pages
         * @return The amount of pages that could fit mem bytes
         */
        constexpr static size_t toPages(size_t mem) {
            return mem / PAGE_SIZE + ((mem % PAGE_SIZE == 0) ? 0 : 1);

        }

        /*!
         * Allocates cntPages pages, consecutive in physical memory
         * @param cntPages The number of pages to allocate
         * @return The physical address of the first page
         */
        size_t allocate(size_t cntPages) {
            return static_cast<Derived *>(this)->allocateImplementation(cntPages);
        }


        /*!
         * Frees the specified number of pages starting at base
         * @param base The physical address of the first page to free
         * @param pages The number of pages to free
         */
        void free(size_t base, size_t pages) {
            static_cast<Derived *>(this)->freeImplementation(base, pages);
        }

    protected:
        /*!
         * Any allocator should, in its constructor, map the memory pages
         * that it will use to store its internal representation of page frames
         *
         * In the base ctor, we will map the number of pages specified by each allocator
         *
         * @param memBase The first address of physical memory
         * @param memSize The size of the memory starting at memBase
         */
        Allocator(size_t memBase, size_t memSize) :
                initialMemBase_{memBase}, initialMemSize_{memSize},
                memBase{memBase}, memSize{memSize},
                allocatorMemory{reinterpret_cast<void *>(paging::PHYSICAL_ALLOCATOR_VIRTUAL_START)},
                cntAllocatorPages{Derived::neededMemoryPages(initialMemSize_)} {
            Logger::instance().println("[P_ALLOCATOR] Initializing Allocator...");

            VirtualAddress virtualAddressStart = paging::PHYSICAL_ALLOCATOR_VIRTUAL_START;
            Logger::instance().println("[P_ALLOCATOR] Mapping allocator, physical %X at virtual %X...",
                                       this->memBase, virtualAddressStart);

            paging::mapPages(virtualAddressStart, this->memBase, cntAllocatorPages);

            Logger::instance().println("[P_ALLOCATOR] Allocator mapped successfully");

            this->memBase += cntAllocatorPages * PAGE_SIZE;
            this->memSize -= cntAllocatorPages * PAGE_SIZE;
        }
    };
}
