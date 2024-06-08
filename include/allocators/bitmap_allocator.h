/*
 * basic_allocator.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */

#pragma once

#include "allocator.h"

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
        size_t cntPages_;
    public:
        BitmapAllocator(size_t memBase, size_t memSize);

        /*!
         * Allocated cntPages pages, consecutive in physical memory
         * @param cntPages The number of cntPages to allocate
         * @return The physical address of the first page
         */
        size_t allocateImplementation(size_t cntPages);

        /*!
         * Free cntPages, starting from base
         * @param base The address of the first page to free
         * @param cntPages The number of pages to free
         */
        void freeImplementation(size_t base, size_t cntPages);

        /*!
         * @return The number of pages of memory that should be mapped for the allocator
         */
        [[nodiscard]] inline constexpr static size_t neededMemoryPages(size_t memSize) {
            auto cntPages = memSize / PAGE_SIZE;
            /// We need one byte for each page, so just the number of pages that fit those bytes
            return toPages(cntPages);
        }
    };
}
