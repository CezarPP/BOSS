/*
 * physical_allocator.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */

#pragma once

#include "../arch/x86_64/phys_mm.h"
#include "../arch/x86_64/paging.h"

namespace physical_allocator {
    constexpr auto PAGE_SIZE = paging::PAGE_SIZE;

    template<typename Derived>
    class Allocator {
    private:
    public:
        /// The physical address of the start memory
        size_t memBase;
        /// The size of the allocator's physical memory, in bytes
        size_t memSize;

        /// The start of the allocator's own memory
        /// This will hold data structures specific to each allocator
        void *allocatorMemory;

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
                memBase{memBase}, memSize{memSize},
                allocatorMemory{reinterpret_cast<void *>(paging::PHYSICAL_ALLOCATOR_VIRTUAL_START)} {
            Logger::instance().println("[P_ALLOCATOR] Initializing Allocator...");

            VirtualAddress virtualAddressStart = paging::PHYSICAL_ALLOCATOR_VIRTUAL_START;
            Logger::instance().println("[P_ALLOCATOR] Mapping allocator, physical %X at virtual %X...",
                                       this->memBase, virtualAddressStart);
            const size_t cntPages = static_cast<Derived *>(this)->neededMemoryPages();
            paging::mapPages(virtualAddressStart, this->memBase, cntPages);

            Logger::instance().println("[P_ALLOCATOR] Allocator mapped successfully");

            this->memBase += cntPages * PAGE_SIZE;
            this->memSize -= cntPages * PAGE_SIZE;
        }
    };
}
