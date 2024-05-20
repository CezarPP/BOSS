/*
 * paging.cpp
 *
 *  Created on: 4/2/24.
 *      Author: Cezar PP
 */

#include "../../include/arch/x86_64/paging.h"

inline __attribute__((always_inline)) void paging::flushTlb(size_t page) {
    // asm volatile("invlpg [%0]"::"r" (page) : "memory");
    asm volatile(
            "invlpg (%0)"              // Invalidate the TLB entry for the given page
            :                          // No output operands
            : "r"(page)                // Input operand: page address in a register
            : "memory"                 // Clobbers: memory
            );
}

/*!
 * Sets the CR3 register to a new value, effectively changing the virtual address space
 * @param physicalPml4Start The physical address of the lvl 4 Page Table
 */
inline __attribute__((always_inline)) void setCR3(size_t physicalPml4Start) {
    static_assert(sizeof(physicalPml4Start) == 8);
    asm volatile(
            "mov %0, %%rax\n\t"          // Move pml4Addr into rax. Use %% for registers in GCC inline asm
            "mov %%rax, %%cr3"           // Move the value in rax into cr3
            :                            // No output operands
            : "r"(physicalPml4Start)              // Input operand: pml4Addr in a register
            : "rax", "memory"            // Clobbers: rax and memory
            );
}

/*!
 * Zeros out a virtual page
 * @param page The virtual address of the page to be cleared
 */
void paging::clearVirtual(size_t page) {
    for (size_t i = 0; i < paging::PAGE_SIZE; i++) {
        reinterpret_cast<uint8_t *>(page)[i] = 0;
    }
}

namespace paging {
    /* Use the memory at the 2nd Megabyte which is already identity mapped
     * That is, we already have `paging::IDENTITY_MAPPED_EARLY` identity mapped
     * This is not very modular, might not work for larger amounts of ram */
    constexpr auto physical_memory = 2_MiB;

    // We will have the physical page tables after the first 2MB
    constexpr auto physical_pml4t_start = physical_memory;
    // Tables are identity mapped
    constexpr auto virtual_pml4t_start = physical_pml4t_start;

    // The number of entries of the PML4 Table (Page Map Level 4)
    constexpr auto pml4_entries = entries(KERNEL_VIRTUAL_SIZE, pml4e_allocations);
    constexpr auto pdpt_entries = entries(KERNEL_VIRTUAL_SIZE, pdpte_allocations);
    constexpr auto pd_entries = entries(KERNEL_VIRTUAL_SIZE, pde_allocations);

    /* The PML4 table will only be one page
     So, the first Page-Directory-Pointer Table (PDPT) will start after one page*/
    constexpr auto physical_pdpt_start = physical_pml4t_start + paging::PAGE_SIZE;
    // There will be as many Page Directory (PD) Tables as the number of entries in the PML4 tables
    constexpr auto physical_pd_start = physical_pdpt_start + pml4_entries * paging::PAGE_SIZE;
    // There will be as many Page Tables (PT) as the number of entries in the Page Directory Tables (PD)
    constexpr auto physical_pt_start = physical_pd_start + pdpt_entries * paging::PAGE_SIZE;

    constexpr auto virtual_pdpt_start = physical_pdpt_start;
    constexpr auto virtual_pd_start = physical_pd_start;
    constexpr auto virtual_pt_start = physical_pt_start;


    pml4t_t find_pml4t() {
        return reinterpret_cast<paging::pml4t_t>(paging::virtual_pml4t_start);
    }
}

/*
 * Entries in the page tables contain the physical address of the next page table.
 * That is, if they were to contain a virtual address, the address finding process would start all over.
 */

paging::pdpt_t find_pdpt(paging::pml4t_t pml4t, size_t pml4e) {
    auto physical_pdpt = reinterpret_cast<uintptr_t>(pml4t[pml4e]) & ~0xFFF;
    auto physical_offset = physical_pdpt - paging::physical_pdpt_start;
    auto virtual_pdpt = paging::virtual_pdpt_start + physical_offset;
    return reinterpret_cast<paging::pdpt_t>(virtual_pdpt);
}

paging::pd_t find_pd(paging::pdpt_t pdpt, size_t pdpte) {
    auto physical_pd = reinterpret_cast<uintptr_t>(pdpt[pdpte]) & ~0xFFF;
    auto physical_offset = physical_pd - paging::physical_pd_start;
    auto virtual_pd = paging::virtual_pd_start + physical_offset;
    return reinterpret_cast<paging::pd_t>(virtual_pd);
}

paging::pt_t find_pt(paging::pd_t pd, size_t pde) {
    auto physical_pt = reinterpret_cast<uintptr_t>(pd[pde]) & ~0xFFF;
    auto physical_offset = physical_pt - paging::physical_pt_start;
    auto virtual_pt = paging::virtual_pt_start + physical_offset;
    return reinterpret_cast<paging::pt_t>(virtual_pt);
}

//TODO Improve to support a status
size_t paging::physicalAddress(VirtualAddress virt) {
    if (!pagePresent(virt)) {
        return 0;
    }

    auto pml4t = find_pml4t();
    auto pdpt = find_pdpt(pml4t, virt.p4Index);
    auto pd = find_pd(pdpt, virt.p3Index);
    auto pt = find_pt(pd, virt.p2Index);
    Logger::instance().println("PML4T %X, pdpt %X, pd %X, pt %X", pml4t, pdpt, pd, pt);

    return virt.offset + (reinterpret_cast<uintptr_t>(pt[virt.p1Index]) & ~0xFFF);
}

bool paging::pagePresent(VirtualAddress virt) {
    auto pml4t = find_pml4t();
    if (!(reinterpret_cast<uintptr_t>(pml4t[virt.p4Index]) & PRESENT)) {
        return false;
    }

    auto pdpt = find_pdpt(pml4t, virt.p4Index);
    if (!(reinterpret_cast<uintptr_t>(pdpt[virt.p3Index]) & PRESENT)) {
        return false;
    }

    auto pd = find_pd(pdpt, virt.p3Index);
    if (!(reinterpret_cast<uintptr_t>(pd[virt.p2Index]) & PRESENT)) {
        return false;
    }

    auto pt = find_pt(pd, virt.p2Index);
    return reinterpret_cast<uintptr_t>(pt[virt.p1Index]) & PRESENT;
}

void paging::map(VirtualAddress virt, size_t physical, uint8_t flags) {
    // The address must be page-aligned
    kAssert(virt.isPageAligned(), "Page is not page-aligned");

    auto pml4t = find_pml4t();
    Logger::instance().println("[PAGING] Mapping pml4e %X, pdpte %X, pde %X, pte %X",
                               virt.p4Index, virt.p3Index, virt.p2Index, virt.p1Index);
    kAssert(reinterpret_cast<uintptr_t>(pml4t[virt.p4Index]) & PRESENT, "[PAGING] A PML4T entry is not PRESENT");

    auto pdpt = find_pdpt(pml4t, virt.p4Index);
    kAssert(reinterpret_cast<uintptr_t>(pdpt[virt.p3Index]) & PRESENT, "[PAGING] A PDPT entry is not PRESENT");

    auto pd = find_pd(pdpt, virt.p3Index);
    kAssert(reinterpret_cast<uintptr_t>(pd[virt.p2Index]) & PRESENT, "[PAGING] A PD entry is not PRESENT");

    auto pt = find_pt(pd, virt.p2Index);

    // Check if the page is already present
    if (reinterpret_cast<uintptr_t>(pt[virt.p1Index]) & PRESENT) {
        // Check that the page is set to the correct value
        kAssert(reinterpret_cast<uintptr_t>(pt[virt.p1Index]) == (physical | flags),
                "[PAGING] Page has incorrect flags");
    }

    // Map to the physical address
    pt[virt.p1Index] = reinterpret_cast<page_entry>(physical | flags);

    // Flush the TLB
    flushTlb(virt.address);
}

void paging::mapPages(VirtualAddress virt, size_t physical, size_t pages, uint8_t flags) {
    // The address must be page-aligned
    kAssert(virt.isPageAligned(), "[PAGING] Page is not page-aligned");

    // Map each page
    for (size_t page = 0; page < pages; page++) {
        auto virt_addr = virt.address + page * PAGE_SIZE;
        auto phys_addr = physical + page * PAGE_SIZE;

        map(virt_addr, phys_addr, flags);
    }
}

void paging::unmap(VirtualAddress virt) {
    // The address must be page-aligned
    kAssert(virt.isPageAligned(), "[PAGING] Page is not page aligned");

    auto pml4t = find_pml4t();

    // If not present, return
    if (!(reinterpret_cast<uintptr_t>(pml4t[virt.p4Index]) & PRESENT)) {
        return;
    }

    auto pdpt = find_pdpt(pml4t, virt.p4Index);

    // If not present, return
    if (!(reinterpret_cast<uintptr_t>(pdpt[virt.p3Index]) & PRESENT)) {
        return;
    }

    auto pd = find_pd(pdpt, virt.p3Index);

    // If not present, return
    if (!(reinterpret_cast<uintptr_t>(pd[virt.p2Index]) & PRESENT)) {
        return;
    }

    auto pt = find_pt(pd, virt.p2Index);

    // Unmap the virtual address
    pt[virt.p1Index] = 0x0;

    // Flush TLB
    flushTlb(virt.address);
}

void paging::unmapPages(VirtualAddress virt, size_t pages) {
    /// The address must be page-aligned
    kAssert(virt.isPageAligned(), "[PAGING] Page is not page aligned");

    /// Unmap each page
    for (size_t page = 0; page < pages; page++) {
        auto virt_addr = virt.address + page * PAGE_SIZE;
        unmap(virt_addr);
    }
}

void paging::init() {
    Logger::instance().println("[PAGING] Setting up paging...");
    // clearVirtual(0x400000);

    /// We will assume we can map all of these tables in the next 6MB of memory, starting from physical_memory
    /// This static_assert ensures this holds
    static_assert(physical_pt_start + pd_entries * PAGE_SIZE < IDENTITY_MAPPED_EARLY);

    auto flags = PRESENT | WRITE | USER;

    /// 1. Prepare PML4T - Page Map Level 4
    Logger::instance().println("[PAGING] Preparing PML4T, mapping %X entries", pml4_entries);

    clearVirtual(physical_pml4t_start); // It is identity mapped
    auto virtual_pml4t = reinterpret_cast<pml4t_t>(physical_pml4t_start);
    for (size_t i = 0; i < pml4_entries; i++) {
        virtual_pml4t[i] = reinterpret_cast<pdpt_t>((physical_pdpt_start + i * PAGE_SIZE) | flags);
    }

    /// 2. Prepare each PDPT - Page Directory Pointer Table
    Logger::instance().println("[PAGING] Preparing PDPT, mapping %X entries", pdpt_entries);

    for (size_t i = 0; i < pml4_entries; i++) {
        auto virtual_pdpt = reinterpret_cast<pdpt_t>(physical_pdpt_start + i * PAGE_SIZE);
        clearVirtual(physical_pdpt_start + i * PAGE_SIZE);

        for (size_t j = 0; j + i * 512 < pdpt_entries; j++) {
            auto r = j + i * 512;

            virtual_pdpt[j] = reinterpret_cast<pd_t>((physical_pd_start + r * PAGE_SIZE) | flags);
        }
    }


    /// 3. Prepare each PD - Page Directory
    Logger::instance().println("[PAGING] Preparing PD, mapping %X entries", pd_entries);

    for (size_t i = 0; i < pdpt_entries; i++) {
        auto virtual_pd = reinterpret_cast<pd_t>(physical_pd_start + i * PAGE_SIZE);
        clearVirtual(physical_pd_start + i * PAGE_SIZE);

        for (size_t j = 0; j + i * 512 < pd_entries; j++) {
            auto r = j + i * 512;

            virtual_pd[j] = reinterpret_cast<pt_t>((physical_pt_start + r * PAGE_SIZE) | flags);
        }
    }

    /// 4. Prepare each PT - Page Table
    Logger::instance().println("[PAGING] Preparing PT, mapping %X entries...", pd_entries);
    for (size_t i = 0; i < pd_entries; i++) {
        auto pageToClear = physical_pt_start + i * PAGE_SIZE;
        clearVirtual(pageToClear);
    }

    /// 5. Identity map the first IDENTITY_MAPPED_EARLY MB (8MB)
    Logger::instance().println("[PAGING] Doing identity mapping on the first %X bytes...", IDENTITY_MAPPED_EARLY);

    auto pageTableEntry = reinterpret_cast<uint64_t *>(physical_pt_start);
    uint64_t phys = PRESENT | WRITE; // Flags for each Page Table Entry
    // We need to identity map 8MB and the page entries map 4KB each
    // We are assuming these Page Tables are in continuous physical (and virtual) memory
    for (size_t i = 0; i < IDENTITY_MAPPED_EARLY / PAGE_SIZE; i++, pageTableEntry++) {
        // Set Page Table Entry
        *pageTableEntry = phys;
        // Map next page
        phys += paging::PAGE_SIZE;
    }

    Logger::instance().println("[PAGING] Setting new CR3 to %X...", physical_pml4t_start);

    static_assert(sizeof(physical_pml4t_start) == 8);

    setCR3(physical_pml4t_start);

    Logger::instance().println("[PAGING] New cr3 has been set!\n");

    /// 6. Perform some tests to check whether the mapping works

    Logger::instance().println("[PAGING] Testing...");

    kAssert(pagePresent(virtual_pml4t_start), "[PAGING] PML4T is not present");

    Logger::instance().println("Virtual address for PML4T is %X", virtual_pml4t_start);
    Logger::instance().println("Physical address for virtual PML4T is %X", physicalAddress(virtual_pml4t_start));
    Logger::instance().println("Physical address for virtual 0x0 is %X", physicalAddress(0x0));


    kAssert(physicalAddress(0x0) == 0x0, "[PAGING] Invalid identity mapping of the first MiB");

    kAssert(physicalAddress(virtual_pml4t_start) == physical_pml4t_start, "[PAGING] PML4T is not correctly mapped");

    kAssert(physicalAddress(0x6969) == 0x6969, "[PAGING] Invalid identity mapping of the first MiB");

    kAssert(physicalAddress(4_MiB) == 4_MiB, "[PAGING] Invalid identity mapping of the first 8MiB");


    Logger::instance().println("[PAGING] Tests finished");
}