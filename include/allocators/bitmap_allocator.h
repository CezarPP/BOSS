/*
 * basic_allocator.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */

#pragma once

#include "allocator.h"
#include "std/bitset.h"

namespace physical_allocator {
    /*!
     * A basic version of a physical allocator
     *
     * The Bitmap Allocator will keep a bit for each page frame from memBase up to memBase + memSize (exclusive)
     * Allocation iterates through all pages until it find a free one
     * Freeing just frees the pages the user requested
     */
    class BitmapAllocator : public Allocator<BitmapAllocator> {
    private:
        bool *isFree_;
        size_t mxPages;
    public:
        BitmapAllocator(size_t memBase, size_t memSize) : Allocator(memBase, memSize) {
            kAssert(paging::pageAligned(this->memBase), "memBase is not page aligned");

            this->mxPages = this->memSize / PAGE_SIZE;
            this->isFree_ = (bool *) this->allocatorMemory;

            std::fill(this->isFree_, this->isFree_ + this->mxPages, false);
        }

        /*!
         * Allocated cntPages pages, consecutive in physical memory
         * @param cntPages The number of cntPages to allocate
         * @return The physical address of the first page
         */
        size_t allocateImplementation(size_t cntPages) {
            for (size_t i = 0; i < mxPages - cntPages; i++) {
                if (std::all_of(isFree_ + i, isFree_ + i + cntPages, [](bool x) {
                    return !x;
                })) {
                    std::fill(isFree_ + i, isFree_ + i + cntPages, true);

                    return memBase + i * PAGE_SIZE;
                }
            }
            kPanic("[P_ALLOC] Can't allocate enough memory");
            return 0;
        }

        /*!
         * Free cntPages, starting from base
         * @param base The address of the first page to free
         * @param cntPages The number of pages to free
         */
        void freeImplementation(size_t base, size_t cntPages) {
            kAssert(paging::pageAligned(base), "[P_ALLOC] Address to free is not page aligned");

            auto indexStart = (base - this->memBase) / PAGE_SIZE;
            auto indexEnd = indexStart + cntPages;

            std::fill(isFree_ + indexStart, isFree_ + indexEnd, false);
        }

        /*!
         * @return The number of pages of memory that should be mapped for the allocator
         */
        constexpr size_t neededMemoryPages() {
            auto cntPages = memSize / PAGE_SIZE;
            return cntPages / PAGE_SIZE + ((cntPages % PAGE_SIZE == 0) ? 0 : 1);
        }
    };
}
