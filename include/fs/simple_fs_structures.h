/*
 * simple_fs_structures.h
 *
 *  Created on: 6/13/24.
 *      Author: Cezar PP
 */

#pragma once

#include "util/types.h"
#include "std/array.h"

namespace simple_fs {
    const constexpr uint32_t MAGIC_NUMBER = 0xf0f03410; ///> Magic number helps in checking Validity of the FileSystem on disk
    const constexpr uint32_t BLOCK_SIZE = ata::SECTOR_SIZE; ///> The size of a block is equal to the size of a sector on disk
    const constexpr uint32_t POINTERS_PER_INODE = 5; ///> Number of Direct block pointers in an Inode Block

    using BlockPointer = uint32_t;
    const constexpr uint32_t POINTERS_PER_BLOCK =
            BLOCK_SIZE / sizeof(BlockPointer); ///> Number of block pointers in one Indirect pointer
    const constexpr uint32_t NAME_SIZE = 16; /// Max Name size for a dentry


    /**
     * Inode Structure
     *
     * Corresponds to a file stored on the disk.
     * Contains the list of raw data blocks used to store the data.
     * Stores the sectors of the file.
    */
    struct Inode {
        uint32_t Valid; ///> Whether or not inode is valid
        uint32_t Size; ///> The logical size of the file in bytes
        std::array<BlockPointer, POINTERS_PER_INODE> Direct; ///> 5 direct pointers to data blocks
        BlockPointer Indirect; ///> One indirect pointer

        void clear() {
            Valid = Size = 0;
            std::fill(Direct.begin(), Direct.end(), 0);
            Indirect = 0;
        }
        // Pointer means the number of the block where the data can be found
        // 0 indicated null block pointer
    };

    const constexpr uint32_t INODES_PER_BLOCK =
            BLOCK_SIZE / sizeof(Inode); ///> Number of Inodes which can be contained in a block
    static_assert(INODES_PER_BLOCK * sizeof(Inode) == BLOCK_SIZE);

    /**
     * SuperBlock structure.
     *
     * It is the first block in any disk.
     * It's main function is to help validating the disk.
     * Contains metadata of the disk.
    */
    struct SuperBlock {
        uint32_t MagicNumber{}; ///> File system magic number
        uint32_t Blocks{}; ///> Number of blocks in file system
        uint32_t InodeBlocks{}; ///> Number of blocks reserved for inodes
        ///> These are stored at the start of the disk, after the SuperBlock
        ///> After the inode blocks there are the free regions
        uint32_t Inodes{}; ///> Number of inodes in file system
        uint32_t DirBlocks{}; ///> Number of blocks reserved for directories

        uint32_t dataStart{}; ///> The block where the data blocks begin
        uint32_t dataEnd{}; ///> The block after the data blocks end

        uint32_t dirStart{}; ///> The block where the directory entries start
        ///> Directories end at the end of the disk
        ///> They are stored in reverse order, actually starting from the end of the disk

        SuperBlock() = default;

        explicit SuperBlock(uint32_t blocks) {
            this->MagicNumber = MAGIC_NUMBER;
            this->Blocks = blocks;
            this->InodeBlocks = this->Blocks / 10; // approximately 1/10th of blocks
            this->Inodes = this->InodeBlocks * INODES_PER_BLOCK;
            this->DirBlocks = this->Blocks / 100; // approximately 1/100th of blocks

            this->dataStart = this->InodeBlocks + 1;
            this->dataEnd = this->Blocks - this->DirBlocks;

            this->dirStart = this->Blocks - this->DirBlocks; ///> These are stored starting from the end of the disk
        }

        bool operator==(const SuperBlock &other) const {
            return (MagicNumber == other.MagicNumber) &&
                   (Blocks == other.Blocks) &&
                   (InodeBlocks == other.InodeBlocks) &&
                   (Inodes == other.Inodes) &&
                   (DirBlocks == other.DirBlocks);
        }
    };

    /* One thing missing in SimpleFS is the free block bitmap.
     * A real filesystem would keep a free block bitmap on disk, recording one bit for each block that was available or in use.
     * This bitmap would be consulted and updated every time the filesystem needed to add or remove a data block from an inode.
     * We will keep the bitmap in memory and scan the disk each time the file system is mounted
     *
     * Each file is identified by an integer inode number, all further references are made using the inode number
     */

    /**
     * @brief Directory Entry - dentry
     * Contains necessary fields to locate the file and directory
     * Consumes 64 KB per object. Used to store information about a directory
     */

    constexpr const bool FILE_TYPE = true;
    constexpr const bool DIR_TYPE = false;

    struct Dirent {
        bool isFile; ///>  type = 1 for file, type = 0 for directory
        bool valid; ///>  valid bit to check if the entry is valid
        uint32_t inum; ///>  inum for Inodes or offset for dir
        char Name[NAME_SIZE]{}; ///> File/Directory Name

        Dirent() : isFile(false), valid(false), inum(0) {
            for (auto &it: Name)
                it = 0;
        }

        vfs::file toFile() {
            return {Name, isFile, inum};
        }
    };

    static_assert(sizeof(Dirent) == 24); // TODO


    const constexpr uint32_t ENTRIES_PER_DIR = 7; /// Number of Files/Directory entries within a Directory
    /**
     * @brief Directory Structure.
     * Contains a table of directory entries for storing hierarchy.
     * Also contains fields for Size and Valid bits.
     * Name is matched against the one in Directory entry.
     * Also, it is allocated from end for effectively using disk space.
     */
    struct Directory {
        bool Valid; ///> Valid bit for validation
        uint32_t inum; ///> inum = block_num * DIR_PER_BLOCK + offset
        char Name[NAME_SIZE]{}; ///> Directory Name
        std::array<Dirent, ENTRIES_PER_DIR> Table{}; ///> Each Table by default contains 2 entries, "." and ".."

        Directory() : Valid(false), inum(-1) {
            for (auto &it: Name)
                it = 0;
            std::fill(Table.begin(), Table.end(), Dirent{});
        }
    };

    const constexpr uint32_t DIR_PER_BLOCK = BLOCK_SIZE / sizeof(Directory); /// Number of Directories per block

    /**
     * @brief Block Union
     * Corresponds to one block of disk of size BLOCK_SIZE
     * Can be used as a SuperBlock, Inode, Pointers block, or raw Data block.
    */
    union Block {
        SuperBlock super; ///> SuperBlock
        Inode inodes[INODES_PER_BLOCK]; ///> Inode block
        uint32_t pointers[POINTERS_PER_BLOCK]; ///> Contains indexes of Direct Blocks, 0 if null
        uint8_t data[BLOCK_SIZE]; ///> Data block
        Directory Directories[DIR_PER_BLOCK]; ///> Directory blocks

        Block() : super{} {
            for (auto &it: data)
                it = 0;
        }

        void clear() {
            for (auto &it: data)
                it = 0;
        }
    };

    static_assert(sizeof(Block) == BLOCK_SIZE);
}