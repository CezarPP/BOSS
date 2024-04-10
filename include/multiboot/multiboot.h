/*
 * multiboot.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */


#pragma once

#include "multiboot2.h"
#include "../util/types.h"
#include "../util/literals.h"
#include "arch/x86_64/exceptions.h"
#include "../std/utility.h"

/*!
 * @param multibootAndMagic A 64 bit structure that contains both the Multiboot address and its magic number
 * @return A pair representing the bigger memory region, {base, size}
 */
std::pair<uint64_t, uint64_t> parseMultiboot(uint64_t multibootAndMagic) {
    uint32_t magic = (multibootAndMagic >> 32);
    uint64_t multibootAddress = (multibootAndMagic & 0xFFFFFFFF);
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        kPanic("[MULTIBOOT] Magic number if invalid");
    }

    for (auto tag = (multiboot_tag *) (multibootAddress + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7))) {
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                Logger::instance().println("[MULTIBOOT] Kernel loaded by: %s", ((multiboot_tag_string *) tag)->string);
                break;
            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_memory_map_t *mmap;

                for (mmap = ((multiboot_tag_mmap *) tag)->entries;
                     (multiboot_uint8_t *) mmap < (multiboot_uint8_t *) tag + tag->size;
                     mmap =
                             (multiboot_memory_map_t *) ((uint64_t) mmap +
                                                         ((multiboot_tag_mmap *) tag)->entry_size)) {
                    Logger::instance().println("[MULTIBOOT] Memory region at address: %X, length: %X, isAvailable: %d",
                                               mmap->addr, mmap->len, mmap->type == MULTIBOOT_MEMORY_AVAILABLE);

                    /// Don't map the reserved memory or the memory before the 1MiB mark
                    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE && mmap->addr > 0_MiB) {
                        Logger::instance().println("[MULTIBOOT] Returning the region from %X, length: %X",
                                                   mmap->addr, mmap->len);
                        return {mmap->addr, mmap->len};
                    }
                }
            }
                break;
        }
    }
    kPanic("[MULTIBOOT] No physical memory found");
    return {0, 0};
}