/*
 * mm.h
 *
 *  Created on: 3/23/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/types.h"
#include "std/extra_std.h"
#include "util/literals.h"

// TODO use this
union PageEntry {
    struct __attribute__((packed)) {
        bool isPresent: 1;             //0
        bool isWritable: 1;            //1
        bool isUserAccessible: 1;      //2
        bool isWriteThroughCaching: 1; //3
        bool isNoCache: 1;             //4
        bool isAccessed: 1;            //5
        bool isDirty: 1;               //6
        bool isHugeOrNull: 1;          //7
        bool isGlobal: 1;              //8
        long long skipped: 54;         //9-62
        bool isNoExecute: 1;           //63
    };

    Address value;

    // PageEntry *getNextLevelEntry(unsigned int index);

    // Address getPhysicalAddress();

    // void fillZero();
};
static_assert(AssertSize<PageEntry, 8>());

constexpr uint64_t PAGE_SIZE = 4_KiB;

union VirtualAddress {
    struct __attribute__((packed)) {
        unsigned int offset: 12{};
        unsigned int p1Index: 9{};
        unsigned int p2Index: 9{};
        unsigned int p3Index: 9{};
        unsigned int p4Index: 9{};
        unsigned int signExt: 16{};
    };

    struct __attribute__((packed)) {
        unsigned int offset2M: 21;
    };

    Address address;

    constexpr VirtualAddress() : address(0) {}

    constexpr VirtualAddress(Address address) {
        this->address = address;
    }

    [[nodiscard]] constexpr bool isPageAligned() const {
        return !(address & (PAGE_SIZE - 1));
    }
};

static_assert(AssertSize<VirtualAddress, 8>());