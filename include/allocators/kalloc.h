/*
 * kalloc.h
 *
 *  Created on: 4/14/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/types.h"
#include "arch/x86_64/paging_constants.h"

namespace kalloc {
    struct MmapedRegion {
        void *ptr; /// The virtual address of the mmaped region
        size_t cntPages; /// The number of pages of the region, 0 if free
    };

    /*!
     * Allocates consecutive virtual pages for big allocations
     * @param size The size of the memory to be allocated with whole pages
     * @return The virtual address of the first page
     */
    void *allocMmaped(size_t size);

    struct MemoryChunk {
        MemoryChunk *next, *prev;
        bool allocated;
        size_t size;
    };

    struct Arena {
        MemoryChunk *first;
        size_t totalSize;

        Arena(void *start, size_t size);

        [[nodiscard]] void *malloc(size_t size) const;

        void free(void *ptr);
    };

    void init();

    /*!
     * A basic form of malloc that allocates memory for the kernel
     * If size if bigger than a certain threshold, we mmap
     * Otherwise, we have a buddy allocator that allocated within the same page
     * 16 bytes minimum,
     * @param size The size of the chunk to be allocated
     * @return A pointer to the allocated memory chunk
     */
    void *kAlloc(size_t size);

    /*!
     * Frees a chunk allocated through kAlloc
     * @param ptr The virtual address of the memory to free
     */
    void kFree(void *ptr);
}