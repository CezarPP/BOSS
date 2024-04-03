/*
 * paging.cpp
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

/*
 * Early paging that is set up in main.asm identity maps the first 4MB (2 huge pages).
 * The stack initially consists of 2 page-frames = 2 * 4KB = 8KB
 * The initial 4-level paging tables are 4 * 4KB = 16KB
 */

#include "util/literals.h"
#include "std/algorithm.h"
#include "phys_mm.h"

namespace paging {
    // Hopefully:
    // User Space: 0x0000000000000000 to 0x00007fffffffffff
    // Kernel Space: 0xffff800000000000 to 0xffffffffffffffff

    constexpr const size_t IDENTITY_MAPPED_EARLY = 8_MiB;
    constexpr const size_t PAGE_SIZE = 4_KiB;
    constexpr const size_t KERNEL_VIRTUAL_SIZE = 2_GiB;

    struct TableInterface {
        PageEntry pageEntries[PAGE_SIZE / sizeof(PageEntry)];
        static_assert(PAGE_SIZE / sizeof(PageEntry) == 512);
    };

    typedef uint64_t* page_entry;
    typedef page_entry* pt_t;
    typedef pt_t* pd_t;
    typedef pd_t* pdpt_t;
    typedef pdpt_t* pml4t_t;

    static_assert(AssertSize<TableInterface, PAGE_SIZE>());

    // On each page there are
    constexpr const size_t pml4e_allocations = 512_GiB; /// The physical memory that a PML4T Entry can map
    constexpr const size_t pdpte_allocations = 1_GiB;   /// The physical memory that a PDPT Entry can map
    constexpr const size_t pde_allocations = 2_MiB;   /// The physical memory that a PD Entry can map
    constexpr const size_t pte_allocations = 4_KiB;   /// The physical memory that a PD Entry can map

    constexpr const uint8_t PRESENT        = 0x1;  /// Paging flag for present page
    constexpr const uint8_t WRITE          = 0x2;  /// Paging flag for writable page
    constexpr const uint8_t USER           = 0x4;  /// Paging flag for user page
    constexpr const uint8_t WRITE_THROUGH  = 0x8;  /// Paging flag for write-through page
    constexpr const uint8_t CACHE_DISABLED = 0x10; /// Paging flag for cache disabled page
    constexpr const uint8_t ACCESSED       = 0x20; /// Paging flag for assessed page

    // Computes the number of entries necessary to map a certain size of memory
    constexpr size_t entries(size_t mem, size_t entry_size) {
        return std::max((size_t) 1, mem / entry_size + ((mem % entry_size == 0) ? 0 : 1));
    }

    // Initialize paging
    void init();

    // Clear all the bytes given the virtual address of a page
    void clearVirtual(size_t page);

    void flushTlb(size_t page);
}