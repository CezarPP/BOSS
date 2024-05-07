/*
 * kalloc.h
 *
 *  Created on: 4/14/24.
 *      Author: Cezar PP
 */

#pragma once

#include "virtual_allocator.h"
#include "std/cstring.h"

namespace kalloc {
    /*!
     * The heap is organized in arenas, arenas are independent heaps
     */


    /// Any bigger than this and we will allocate entire pages
    constexpr const size_t mmap_threshold = 0; // TODO PAGE_SIZE;

    /// Statistics for mmap
    size_t mmapedMemory = 0, maxMmapedMemory = 0;

    struct MmapedRegion {
        void *ptr; /// The virtual address of the mmaped region
        size_t cntPages; /// The number of pages of the region, 0 if free
    };

    std::vector<MmapedRegion, virtual_allocator::virtualStdAllocator<MmapedRegion>>
            mmapedRegions{virtual_allocator::virtualStdAllocator<MmapedRegion>()};
    // MmapedRegion mmapedRegions[MAX_MMAPED_REGIONS];

    void init() {
        mmapedRegions.reserve(1000);
    }

    /*!
     * Allocates consecutive virtual pages for big allocations
     * @param size The size of the memory to be allocated with whole pages
     * @return The virtual address of the first page
     */
    void *allocMmaped(size_t size) {
        size_t cntPages = size / PAGE_SIZE + ((size % PAGE_SIZE == 0) ? 0 : 1);
        void *ptr = virtual_allocator::VirtualAllocator::instance()->vAlloc(cntPages);
        for (auto &mmapedRegion: mmapedRegions)
            if (mmapedRegion.cntPages == 0) {
                mmapedRegion = {ptr, cntPages};

                // Statistics
                mmapedMemory += cntPages;
                maxMmapedMemory = std::max(maxMmapedMemory, mmap_threshold);

                return ptr;
            }
        kPanic("No free slot available");
        // __builtin_unreachable();
        return nullptr; // unreachable
    }

    /*!
     * A basic form of malloc that allocates memory for the kernel
     * If size if bigger than a certain threshold, we mmap
     * Otherwise, we have a buddy allocator that allocated within the same page
     * 16 bytes minimum,
     * @param size The size of the chunk to be allocated
     * @return A pointer to the allocated memory chunk
     */
    void *kAlloc(size_t size) {
        if (size >= mmap_threshold) {
            return allocMmaped(size);
        }

        return nullptr; // TODO
    }


    /*!
     * Frees a chunk allocated through kAlloc
     * @param ptr The virtual address of the memory to free
     */
    void kFree(void *ptr) {
        /// If it is mmaped, release that memory and return
        for (auto &mmapedRegion: mmapedRegions)
            if (mmapedRegion.cntPages > 0 && mmapedRegion.ptr == ptr) {
                virtual_allocator::VirtualAllocator::instance()->vFree(ptr, mmapedRegion.cntPages);
                mmapedRegion.cntPages = 0;
                return;
            }

        // TODO
    }
}