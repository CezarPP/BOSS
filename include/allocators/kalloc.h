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
     * For each arena we will use a linked list to allocate memory
     */

    constexpr const size_t ARENA_SIZE_PAGES = 8;
    constexpr const size_t ARENA_SIZE = ARENA_SIZE_PAGES * PAGE_SIZE;
    /// Any bigger than this and we will allocate entire pages
    constexpr const size_t MMAP_THRESHOLD = PAGE_SIZE;

    /// Statistics for mmap
    size_t mmapedMemory = 0, maxMmapedMemory = 0;

    struct MmapedRegion {
        void *ptr; /// The virtual address of the mmaped region
        size_t cntPages; /// The number of pages of the region, 0 if free
    };

    std::vector_early<MmapedRegion, virtual_allocator::virtualStdAllocator<MmapedRegion>>
            mmapedRegions{virtual_allocator::virtualStdAllocator<MmapedRegion>()};
    // MmapedRegion mmapedRegions[MAX_MMAPED_REGIONS];

    /*!
     * Allocates consecutive virtual pages for big allocations
     * @param size The size of the memory to be allocated with whole pages
     * @return The virtual address of the first page
     */
    void *allocMmaped(size_t size) {
        size_t cntPages = size / PAGE_SIZE + ((size % PAGE_SIZE == 0) ? 0 : 1);
        kAssert(cntPages > 0, "[V_ALLOC] Mmap should allocate at least one page");
        void *ptr = virtual_allocator::VirtualAllocator::instance()->vAlloc(cntPages);
        for (auto &mmapedRegion: mmapedRegions)
            if (mmapedRegion.cntPages == 0) {
                mmapedRegion = {ptr, cntPages};

                // Statistics
                mmapedMemory += cntPages;
                maxMmapedMemory = std::max(maxMmapedMemory, mmapedMemory);

                return ptr;
            }
        kPanic("No free slot available");
        // __builtin_unreachable();
        return nullptr; // unreachable
    }

    struct MemoryChunk {
        MemoryChunk *next, *prev;
        bool allocated;
        size_t size;
    };

    struct Arena {
        MemoryChunk *first;
        size_t totalSize;

        Arena(void *start, size_t size) {
            kAssert(size >= sizeof(MemoryChunk), "[KALLOC] Size should be bigger than a memory chunk!");
            this->first = (MemoryChunk *) start;
            this->totalSize = size;

            this->first->allocated = false;
            this->first->prev = nullptr;
            this->first->next = nullptr;
            this->first->size = size - sizeof(MemoryChunk);
        }

        [[nodiscard]] void *malloc(size_t size) const {
            MemoryChunk *result = nullptr;

            for (MemoryChunk *chunk = first; chunk != nullptr && result == nullptr; chunk = chunk->next)
                if (chunk->size >= size && !chunk->allocated)
                    result = chunk;

            if (result == nullptr)
                return nullptr;

            if (result->size >= size + sizeof(MemoryChunk) + 1) {
                auto *temp = (MemoryChunk *) ((size_t) result + sizeof(MemoryChunk) + size);

                temp->allocated = false;
                temp->size = result->size - size - sizeof(MemoryChunk);
                temp->prev = result;
                temp->next = result->next;
                if (temp->next != nullptr)
                    temp->next->prev = temp;

                result->size = size;
                result->next = temp;
            }

            result->allocated = true;
            return (void *) (((size_t) result) + sizeof(MemoryChunk));
        }

        void free(void *ptr) {
            auto *chunk = (MemoryChunk *) ((size_t) ptr - sizeof(MemoryChunk));

            chunk->allocated = false;

            if (chunk->prev != nullptr && !chunk->prev->allocated) {
                chunk->prev->next = chunk->next;
                chunk->prev->size += chunk->size + sizeof(MemoryChunk);
                if (chunk->next != nullptr)
                    chunk->next->prev = chunk->prev;

                chunk = chunk->prev;
            }

            if (chunk->next != nullptr && !chunk->next->allocated) {
                chunk->size += chunk->next->size + sizeof(MemoryChunk);
                chunk->next = chunk->next->next;
                if (chunk->next != nullptr)
                    chunk->next->prev = chunk;
            }

        }
    };

    std::vector_early<Arena, virtual_allocator::virtualStdAllocator<Arena>>
            arenas{virtual_allocator::virtualStdAllocator<Arena>()};


    void init() {
        Logger::instance().println("[KALLOC] Initializing...");
        mmapedRegions.reserve(1024);
        arenas.reserve(1024);
        Logger::instance().println("[KALLOC] Finished initializing");
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
        if (size >= MMAP_THRESHOLD) {
            Logger::instance().println("[KALLOC] Allocating %X via mmap", size);
            return allocMmaped(size);
        }
        Logger::instance().println("[KALLOC] Allocating %X into arenas", size);
        for (auto it: arenas) {
            auto ptr = it.malloc(size);
            if (ptr != nullptr)
                return ptr;
        }
        Logger::instance().println("[KALLOC] Allocating new arena...");
        void *arenaStart = virtual_allocator::VirtualAllocator::instance()->vAlloc(ARENA_SIZE_PAGES);
        Logger::instance().println("[KALLOC] Allocated memory for arena!");
        arenas.push_back({arenaStart, ARENA_SIZE});
        Logger::instance().println("[KALLOC] Allocating into new arena...");
        return arenas.back().malloc(size);
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

        for (auto it: arenas)
            if (ptr >= it.first && ptr < it.first + it.totalSize) {
                it.free(ptr);
                return;
            }

        kPanic("[KALLOC] We should have freed the pointer by now!");
    }
}