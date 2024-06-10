/*
 * disk_driver.h
 *
 *  Created on: 6/10/24.
 *      Author: Cezar PP
 */


#pragma once

#include "util/types.h"
#include "arch/x86_64/exceptions.h"

/**
 * Implements Disk abstraction, used by file system to access and make changes to the disk.
 */
class Disk {
protected:
    size_t cntBlocks_; /// Number of blocks in disk image
    size_t totalSize_; /// The total size in bytes

    /// Statistics
    size_t cntReads_;  /// Number of reads performed
    size_t cntWrites_; /// Number of writes performed
    size_t cntMounts_; /// Number of mounts

    /**
     * Check if the block is within valid range
     * @param blockIndex index of the block into the free block bitmap
     * @param data data buffer
     */
    inline void sanityCheck(size_t blockIndex, const char *data) const {
        kAssert(blockIndex < cntBlocks_, "BlockIndex is too big!");
        kAssert(data != nullptr, "Data should not be a null pointer!");
    }

public:
    constexpr Disk() : cntBlocks_(0), totalSize_(0), cntReads_(0), cntWrites_(0), cntMounts_(0) {}

    /**
     * Check size of disk
     * @return size of disk in terms of blocks
     */
    [[nodiscard]] size_t size() const {
        return cntBlocks_;
    }

    [[nodiscard]] bool isMounted() const {
        return cntMounts_ > 0;
    }

    void mount() {
        cntMounts_++;
    }

    void unmount() {
        kAssert(cntMounts_ > 0, "The disk should have already been mounted!");
        if (cntMounts_ > 0)
            cntMounts_--;
    }

    /**
     * Read from disk
     * @param blockIndex block to read from
     * @param data data buffer to write into
     */
    virtual void read(size_t blockIndex, uint8_t *data) = 0;

    /**
     * Write to disk
     * @param blockIndex block to write into
     * @param data data buffer to read from
     */
    virtual void write(size_t blockIndex, const uint8_t *data) = 0;


    /**
     * Flush the cache of the hard drive
     */
    virtual void flush() = 0;
};