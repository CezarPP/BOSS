/*
 * bitmap_allocator.cpp
 *
 *  Created on: 6/8/24.
 *      Author: Cezar PP
 */

#include "allocators/bitmap_allocator.h"

namespace physical_allocator {
    BitmapAllocator::BitmapAllocator(size_t memBase, size_t memSize) : Allocator(memBase, memSize) {
        kAssert(paging::pageAligned(this->memBase), "memBase is not page aligned");

        this->cntPages_ = this->memSize / PAGE_SIZE;
        this->isFree_ = (bool *) this->allocatorMemory;

        std::fill(this->isFree_, this->isFree_ + this->cntPages_, false);
    }

    size_t BitmapAllocator::allocateImplementation(size_t cntPages) {
        for (size_t i = 0; i < cntPages_ - cntPages + 1; i++) {
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

    void BitmapAllocator::freeImplementation(size_t base, size_t cntPages) {
        kAssert(paging::pageAligned(base), "[P_ALLOC] Address to free is not page aligned");

        kAssert(base >= this->memBase, "Address has to be bigger than memBase");
        auto indexStart = (base - this->memBase) / PAGE_SIZE;
        auto indexEnd = indexStart + cntPages;

        kAssert(indexEnd <= cntPages_, "[P_ALLOC] Wrong index value");

        std::fill(isFree_ + indexStart, isFree_ + indexEnd, false);
    }
}
