/*
 * paging_constants.h
 *
 *  Created on: 6/8/24.
 *      Author: Cezar PP
 */


#ifndef BOSS_PAGING_CONSTANTS_H
#define BOSS_PAGING_CONSTANTS_H

#include "util/types.h"
#include "util/literals.h"

namespace paging {
    constexpr const size_t IDENTITY_MAPPED_EARLY = 8_MiB;
    constexpr const size_t PAGE_SIZE = 4_KiB;
    constexpr const size_t KERNEL_VIRTUAL_SIZE = 2_GiB;
    constexpr const size_t MAX_PHYSICAL_SIZE = 32_GiB;

    // Start of the virtual addresses for the structures of the allocator
    constexpr const size_t PHYSICAL_ALLOCATOR_VIRTUAL_START = 8_MiB;
    // Linux uses 1% of memory to keep the page_frame structs, we will make sure not to use more than that
    // Don't need this constant anymore
    // constexpr const size_t PHYSICAL_ALLOCATOR_VIRTUAL_SIZE = MAX_PHYSICAL_SIZE / 100;

    // The start and end virtual addresses available to the Virtual Allocator
    // Only for kernel space, user space should be able to address more since it will have more page tables

    // On each page there are
    constexpr const size_t pml4e_allocations = 512_GiB; ///> The physical memory that a PML4T Entry can map
    constexpr const size_t pdpte_allocations = 1_GiB;   ///> The physical memory that a PDPT Entry can map
    constexpr const size_t pde_allocations = 2_MiB;   ///> The physical memory that a PD Entry can map
    constexpr const size_t pte_allocations = 4_KiB;   ///> The physical memory that a PD Entry can map

    constexpr const uint8_t PRESENT = 0x1;  ///> Paging flag for present page
    constexpr const uint8_t WRITE = 0x2;  ///> Paging flag for writable page
    constexpr const uint8_t USER = 0x4;  ///> Paging flag for user page
    constexpr const uint8_t WRITE_THROUGH = 0x8;  ///> Paging flag for write-through page
    constexpr const uint8_t CACHE_DISABLED = 0x10; ///> Paging flag for cache disabled page
    constexpr const uint8_t ACCESSED = 0x20; ///> Paging flag for accessed page
}

#endif //BOSS_PAGING_CONSTANTS_H
