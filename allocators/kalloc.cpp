/*
 * kalloc.cpp
 *
 *  Created on: 6/8/24.
 *      Author: Cezar PP
 */

/*
 * kalloc.h
 *
 *  Created on: 4/14/24.
 *      Author: Cezar PP
 */

#include "allocators/kalloc.h"
#include "std/vector_early.h"
#include "allocators/virtual_allocator.h"

namespace kalloc {
    constexpr const size_t PAGE_SIZE = paging::PAGE_SIZE;
    /*!
     * The heap is organized in arenas, arenas are independent heaps
     * For each arena we will use a linked list to allocate memory
     */

    constexpr const size_t ARENA_SIZE_PAGES = 8;
    constexpr const size_t ARENA_SIZE = ARENA_SIZE_PAGES * PAGE_SIZE;
    /// Any bigger than this and we will allocate entire pages
    constexpr const size_t MMAP_THRESHOLD = PAGE_SIZE;

    /// Statistics for mmap
    size_t mmapedPointers = 0, mmapedMemory = 0, maxMmapedMemory = 0;

    std::vector_early<MmapedRegion, virtual_allocator::virtualStdAllocator<MmapedRegion>>
            mmapedRegions{virtual_allocator::virtualStdAllocator<MmapedRegion>()};

    std::vector_early<Arena, virtual_allocator::virtualStdAllocator<Arena>>
            arenas{virtual_allocator::virtualStdAllocator<Arena>()};

    void *allocMmaped(size_t size) {
        size_t cntPages = size / PAGE_SIZE + ((size % PAGE_SIZE == 0) ? 0 : 1);
        Logger::instance().println("[KALLOC] Allocating %X pages via mmap", cntPages);

        kAssert(cntPages > 0, "[V_ALLOC] Mmap should allocate at least one page");
        void *ptr = virtual_allocator::VirtualAllocator::instance()->vAlloc(cntPages);
        for (auto &mmapedRegion: mmapedRegions)
            if (mmapedRegion.isEmpty()) {
                mmapedRegion = {ptr, cntPages};

                // Statistics
                mmapedPointers++;
                mmapedMemory += cntPages;
                maxMmapedMemory = std::max(maxMmapedMemory, mmapedMemory);

                return ptr;
            }
        mmapedRegions.push_back({ptr, cntPages});

        mmapedPointers++;
        mmapedMemory += cntPages;
        maxMmapedMemory = std::max(maxMmapedMemory, mmapedMemory);

        return ptr;
        // __builtin_unreachable();
        return nullptr; // unreachable
    }


    Arena::Arena(void *start, size_t size) {
        kAssert(size >= sizeof(MemoryChunk), "[KALLOC] Size should be bigger than a memory chunk!");
        this->first = (MemoryChunk *) start;
        this->totalSize = size;

        this->first->allocated = false;
        this->first->prev = nullptr;
        this->first->next = nullptr;
        this->first->size = size - sizeof(MemoryChunk);
    }

    [[nodiscard]] void *Arena::malloc(size_t size) const {
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

    void Arena::free(void *ptr) {
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


    void init() {
        Logger::instance().println("[KALLOC] Initializing...");
        mmapedRegions.resize(4096);
        std::fill(mmapedRegions.begin(), mmapedRegions.end(), MmapedRegion{});
        arenas.reserve(1024);
        Logger::instance().println("[KALLOC] Finished initializing");
    }

    void *kAlloc(size_t size) {
        if (size >= MMAP_THRESHOLD) {
            return allocMmaped(size);
        }
        Logger::instance().println("[KALLOC] Allocating %X bytes into arenas", size);
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


    void kFree(void *ptr) {
        /// If it is mmaped, release that memory and return
        for (auto &mmapedRegion: mmapedRegions)
            if (!mmapedRegion.isEmpty() && mmapedRegion.ptr == ptr) {
                virtual_allocator::VirtualAllocator::instance()->vFree(ptr, mmapedRegion.cntPages);
                mmapedRegion = {nullptr, 0};
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