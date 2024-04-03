/*
 * paging.cpp
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

#include "../../include/arch/x86_64/paging.h"
#include "../../include/arch/x86_64/logging.h"

inline void flushTlb(size_t page) {
    asm volatile("invlpg [%0]"::"r" (page) : "memory");
}

void paging::clearVirtual(size_t page) {
    for (size_t i = 0; i < paging::PAGE_SIZE; i++) {
        reinterpret_cast<uint8_t *>(page)[i] = 0;
    }
}

constexpr size_t pml4_entry(size_t virtualAddress) {
    return (virtualAddress >> 39) & 0x1FF;
}

constexpr size_t pdpt_entry(size_t virtualAddress) {
    return (virtualAddress >> 30) & 0x1FF;
}

constexpr size_t pd_entry(size_t virtualAddress) {
    return (virtualAddress >> 21) & 0x1FF;
}

constexpr size_t pt_entry(size_t virtualAddress) {
    return (virtualAddress >> 12) & 0x1FF;
}

inline __attribute__((always_inline)) void setCR3(size_t physicalPml4Start) {
    // asm volatile("mov rax, %0; mov cr3, rax" : : "m"(physicalPml4Start) : "memory", "rax");
    /*asm volatile("mov rax, %0" : : "r"(physicalPml4Start) : "rax"); // First, move to rax
    asm volatile("mov cr3, rax" : : : "rax"); // Then, move from rax to cr3*/
    static_assert(sizeof(physicalPml4Start) == 8);
    asm volatile(
            "mov %0, %%rax\n\t"          // Move pml4Addr into rax. Use %% for registers in GCC inline asm
            "mov %%rax, %%cr3"           // Move the value in rax into cr3
            :                            // No output operands
            : "r"(physicalPml4Start)              // Input operand: pml4Addr in a register
            : "rax", "memory"            // Clobbers: rax and memory
            );
}

void paging::init() {
    Logger::instance().println("Starting to set up paging");
    clearVirtual(0x400000);
    // Use the memory at the 2nd Megabyte which is already identity mapped
    // That is, we already have `paging::IDENTITY_MAPPED_EARLY` identity mapped
    // This is not very modular, might not work for larger amounts of ram
    constexpr auto physical_memory = 2_MiB;

    // The number of entries of the PML4 Table (Page Map Level 4)
    constexpr auto pml4_entries = entries(KERNEL_VIRTUAL_SIZE, pml4e_allocations);
    constexpr auto pdpt_entries = entries(KERNEL_VIRTUAL_SIZE, pdpte_allocations);
    constexpr auto pd_entries = entries(KERNEL_VIRTUAL_SIZE, pde_allocations);

    Logger::instance().println("The number of pd_entries is %d", pd_entries);

    // We will have the physical page tables after the first 2MB
    constexpr auto physical_pml4t_start = physical_memory;
    // The PML4 table will only be one page
    // So, the first Page-Directory-Pointer Table (PDPT) will start after one page
    constexpr auto physical_pdpt_start = physical_pml4t_start + paging::PAGE_SIZE;
    // There will be as many Page Directory (PD) Tables as the number of entries in the PML4 tables
    constexpr auto physical_pd_start = physical_pdpt_start + pml4_entries * paging::PAGE_SIZE;
    // There will be as many Page Tables (PT) as the number of entries in the Page Directory Tables (PD)
    constexpr auto physical_pt_start = physical_pd_start + pdpt_entries * paging::PAGE_SIZE;

    // We will assume we can map all of these tables in the next 6MB of memory, starting from physical_memory
    // This static_assert ensures this holds
    static_assert(physical_pt_start + pd_entries * PAGE_SIZE < IDENTITY_MAPPED_EARLY);

    auto flags = PRESENT | WRITE | USER;

    /// 1. Prepare PML4T - Page Map Level 4
    Logger::instance().println("Starting to prepare PML4T");

    clearVirtual(physical_pml4t_start); // It is identity mapped
    auto virtual_pml4t = reinterpret_cast<pml4t_t>(physical_pml4t_start);
    for (size_t i = 0; i < pml4_entries; ++i) {
        virtual_pml4t[i] = reinterpret_cast<pdpt_t>((physical_pdpt_start + i * PAGE_SIZE) | flags);
    }

    /// 2. Prepare each PDPT - Page Directory Pointer Table
    Logger::instance().println("Starting to prepare PDPT");

    for (size_t i = 0; i < pml4_entries; ++i) {
        auto virtual_pdpt = reinterpret_cast<pdpt_t>(physical_pdpt_start + i * PAGE_SIZE);
        clearVirtual(physical_pdpt_start + i * PAGE_SIZE);

        for (size_t j = 0; j + i * 512 < pdpt_entries; ++j) {
            auto r = j + i * 512;

            virtual_pdpt[j] = reinterpret_cast<pd_t>((physical_pd_start + r * PAGE_SIZE) | flags);
        }
    }


    /// 3. Prepare each PD - Page Directory
    Logger::instance().println("Starting to prepare PD");

    for (size_t i = 0; i < pdpt_entries; ++i) {
        auto virtual_pd = reinterpret_cast<pd_t>(physical_pd_start + i * PAGE_SIZE);
        clearVirtual(physical_pd_start + i * PAGE_SIZE);

        for (size_t j = 0; j + i * 512 < pd_entries; ++j) {
            auto r = j + i * 512;

            virtual_pd[j] = reinterpret_cast<pt_t>((physical_pt_start + r * PAGE_SIZE) | flags);
        }
    }

    /// 4. Prepare each PT - Page Table
    Logger::instance().println("Starting to prepare PT, number of PD entries is %d", pd_entries);
    for (size_t i = 0; i < pd_entries; ++i) {
        auto pageToClear = physical_pt_start + i * PAGE_SIZE;
        clearVirtual(pageToClear);
    }

    /// 5. Identity map the first 8MB
    Logger::instance().println("Starting to do identity mapping");

    auto pageTableEntry = reinterpret_cast<uint64_t *>(physical_pt_start);
    auto phys = PRESENT | WRITE; // Flags for each Page Table Entry
    // We need to identity map 8MB and the page entries map 4KB each
    // We are assuming these Page Tables are in continuous physical (and virtual) memory
    for (size_t i = 0; i < 8_MiB / 4_KiB; ++i, ++pageTableEntry) {
        // Set Page Table Entry
        *pageTableEntry = phys;
        // Map next page
        phys += paging::PAGE_SIZE;
    }

    Logger::instance().println("Setting new CR3 to %X\n", physical_pml4t_start);

    static_assert(sizeof(physical_pml4t_start) == 8);
    setCR3(physical_pml4t_start);

    Logger::instance().println("New cr3 has been set!");
}