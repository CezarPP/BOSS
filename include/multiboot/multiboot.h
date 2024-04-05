/*
 * multiboot.h
 *
 *  Created on: 4/5/24.
 *      Author: Cezar PP
 */


#pragma once

#include "multiboot2.h"
#include "../util/types.h"
#include "arch/x86_64/exceptions.h"

void parseMultiboot(uint64_t multibootAndMagic) {
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
                    // TODO Don't map the reserved memory or the memory before the 8MiB mark
                    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        // TODO memory->addRegion(mmap->addr, mmap->len);
                        Logger::instance().println("[MULTIBOOT] TODO Added region to allocator");
                    }
                }
            }
                break;
        }
    }
}