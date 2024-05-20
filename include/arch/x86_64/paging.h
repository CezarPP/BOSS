/*
 * paging.cpp
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

/*
 * Early paging that is set up in main.asm identity maps the first 8MB (4 huge pages).
 * The stack initially consists of 2 page-frames = 2 * 4KB = 8KB
 * The initial 4-level paging tables are 4 * 4KB = 16KB
 */

/*
 * Tables will be accessed through their virtual addresses.
 */

#include "util/literals.h"
#include "std/algorithm.h"
#include "phys_mm.h"
#include "logging.h"
#include "exceptions.h"

namespace paging {
    /* Future work:
     * User Space: 0x0000000000000000 to 0x00007fffffffffff
     * Theoretical Kernel Space: 0xffff800000000000 to 0xffffffffffffffff
     * Practical Kernel Space: 0xffff800000000000 to 0xffff800077359400 for 2GB virtual size */

    /* Current config
     * Kernel Space: 0x0000000000000000 to 2GB
     * User Space: not implemented
     */


    constexpr const size_t IDENTITY_MAPPED_EARLY = 8_MiB;
    constexpr const size_t PAGE_SIZE = 4_KiB;
    constexpr const size_t KERNEL_VIRTUAL_SIZE = 2_GiB;
    constexpr const size_t MAX_PHYSICAL_SIZE = 32_GiB;

    // Start of the virtual addresses for the structures of the allocator
    constexpr const size_t PHYSICAL_ALLOCATOR_VIRTUAL_START = 8_MiB;
    // Linux uses 1% of memory to keep the page_frame structs, we will make sure not to use more than that
    // Don't need this constant anymore
    // constexpr const size_t PHYSICAL_ALLOCATOR_VIRTUAL_SIZE = MAX_PHYSICAL_SIZE / 100;

    /*constexpr const size_t KERNEL_VIRTUAL_START = PHYSICAL_ALLOCATOR_VIRTUAL_START + PHYSICAL_ALLOCATOR_VIRTUAL_SIZE;
    // The start and end virtual addresses available to the Virtual Allocator
    // Only for kernel space, user space should be able to address more since it will have more page tables
    constexpr const size_t VIRTUAL_ALLOCATOR_START = KERNEL_VIRTUAL_START + PHYSICAL_ALLOCATOR_VIRTUAL_SIZE;
    constexpr const size_t VIRTUAL_ALLOCATOR_END = KERNEL_VIRTUAL_START + KERNEL_VIRTUAL_SIZE;*/

    typedef uint64_t *page_entry;
    typedef page_entry *pt_t;
    typedef pt_t *pd_t;
    typedef pd_t *pdpt_t;
    typedef pdpt_t *pml4t_t;

    // On each page there are
    constexpr const size_t pml4e_allocations = 512_GiB; /// The physical memory that a PML4T Entry can map
    constexpr const size_t pdpte_allocations = 1_GiB;   /// The physical memory that a PDPT Entry can map
    constexpr const size_t pde_allocations = 2_MiB;   /// The physical memory that a PD Entry can map
    constexpr const size_t pte_allocations = 4_KiB;   /// The physical memory that a PD Entry can map

    constexpr const uint8_t PRESENT = 0x1;  /// Paging flag for present page
    constexpr const uint8_t WRITE = 0x2;  /// Paging flag for writable page
    constexpr const uint8_t USER = 0x4;  /// Paging flag for user page
    constexpr const uint8_t WRITE_THROUGH = 0x8;  /// Paging flag for write-through page
    constexpr const uint8_t CACHE_DISABLED = 0x10; /// Paging flag for cache disabled page
    constexpr const uint8_t ACCESSED = 0x20; /// Paging flag for accessed page

    size_t physicalAddress(VirtualAddress virt);

    bool pagePresent(VirtualAddress virt);

    pml4t_t find_pml4t();

    void map(VirtualAddress virt, size_t physical, uint8_t flags = PRESENT | WRITE);

    void mapPages(VirtualAddress virt, size_t physical, size_t pages, uint8_t flags = PRESENT | WRITE);

    void unmap(VirtualAddress virt);

    void unmapPages(VirtualAddress virt, size_t pages);

    /*!
     * Computes the number of entries necessary to map a certain size of memory
     *
     * The memory could be anything, while the entry could be the size of a page for example
     * @param mem The size of the memory to be mapped
     * @param entry_size The size of the entry
     * @return The number of entries necessary
     */
    constexpr size_t entries(size_t mem, size_t entry_size) {
        return std::max((size_t) 1, mem / entry_size + ((mem % entry_size == 0) ? 0 : 1));
    }

    constexpr bool pageAligned(size_t addr) {
        return !(addr & (paging::PAGE_SIZE - 1));
    }

    constexpr size_t pageAlign(size_t addr) {
        return (addr / paging::PAGE_SIZE) * paging::PAGE_SIZE;
    }

    constexpr std::pair<uint64_t, uint64_t> physicalExcludingEarly(const std::pair<uint64_t, uint64_t> &memory) {
        if (memory.first < IDENTITY_MAPPED_EARLY) {
            uint64_t dif = IDENTITY_MAPPED_EARLY - memory.first;
            return {IDENTITY_MAPPED_EARLY, memory.second - dif};
        }
        return memory;
    }

    // Initialize paging
    void init();

    // Clear all the bytes given the virtual address of a page
    void clearVirtual(size_t page);

    __attribute__((always_inline)) void flushTlb(size_t page);
}