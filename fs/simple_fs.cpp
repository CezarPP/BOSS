/*
 * simple_fs.cpp
 *
 *  Created on: 6/10/24.
 *      Author: Cezar PP
 */


#include "fs/simple_fs.h"

namespace simple_fs {
    inline void SimpleFS::checkDiskNotMounted() const {
        kAssert(!disk_->isMounted(), "[SIMPLE_FS] Disk should not yet be mounted");
    }

    inline void SimpleFS::checkFsMounted() const {
        kAssert(isMounted, "[SIMPLE_FS] The file system should be mounted at this point!");
    }

    void SimpleFS::debug() {
        Block block{};

        /// Read superBlock
        disk_->read(0, block.data);

        Logger::instance().println("[SIMPLE_FS] DEBUG");
        Logger::instance().println("[SIMPLE_FS] Magic number is %x", block.super.MagicNumber);

        kAssert(block.super.MagicNumber == MAGIC_NUMBER, "[SIMPLE_FS] Magic number is invalid");

        Logger::instance().println("Blocks: %d, inode blocks: %d, inodes: %d", block.super.Blocks,
                                   block.super.InodeBlocks, block.super.Inodes);

        /// reading the inode blocks
        int ii = 0;

        for (uint32_t i = 1; i <= block.super.InodeBlocks; i++) {
            disk_->read(i, block.data);

            for (auto &inode: block.inodes) {
                if (inode.Valid) {
                    Logger::instance().println("Inode %d:\n", ii);
                    Logger::instance().println("    file size: %d bytes\n", inode.Size);
                    Logger::instance().println("    direct blocks:");

                    for (auto directPtr: inode.Direct)
                        if (directPtr)
                            Logger::instance().println(" %d", directPtr);

                    if (inode.Indirect) {
                        Logger::instance().println("    indirect block: %d\n    indirect data blocks:",
                                                   inode.Indirect);
                        Block IndirectBlock;
                        disk_->read(inode.Indirect, IndirectBlock.data);
                        for (auto indirectPtr: IndirectBlock.pointers) {
                            if (indirectPtr)
                                Logger::instance().println(" %d", indirectPtr);
                        }
                    }
                }

                ii++;
            }
        }

        Logger::instance().println("[SIMPLE_FS] Finished debugging!");
    }

    void SimpleFS::format() {
        Logger::instance().println("[SIMPLE_FS] Formatting disk...");
        checkDiskNotMounted();

        Block superBlock{};

        Logger::instance().println("[SIMPLE_FS] The disk has %X sectors.", disk_->size());
        superBlock.super = SuperBlock(disk_->size());

        disk_->write(0, superBlock.data);


        Block emtpyBlock{};
        memset(&emtpyBlock, 0, sizeof(emtpyBlock));

        /// Clear all other blocks
        Logger::instance().println("[SIMPLE_FS] Clearing inode blocks...");
        for (uint32_t i = 1; i <= superBlock.super.InodeBlocks; i++) {
            disk_->write(i, emtpyBlock.data);
        }

        Logger::instance().println("[SIMPLE_FS] Clearing data blocks...");

        /// Free Data Blocks
        for (uint32_t i = superBlock.super.dataStart; i < superBlock.super.dataEnd; i++) {
            disk_->write(i, emtpyBlock.data);
        }

        Logger::instance().println("[SIMPLE_FS] Clearing directory blocks...");
        // Free Directory Blocks

        Directory emptyDir{};
        emptyDir.inum = -1;
        for (auto &dir: emtpyBlock.Directories)
            dir = emptyDir;
        for (uint32_t i = superBlock.super.dirStart; i < superBlock.super.Blocks; i++) {
            disk_->write(i, emtpyBlock.data);
        }

        emtpyBlock.clear();

        // Create Root directory

        Logger::instance().println("[SIMPLE_FS] Creating root directory...");
        Directory root{};
        strcpy(root.Name, "/");
        root.inum = 0;
        root.Valid = true;

        // Create table entries for "." and ".."

        // Both . and .. should also point to the root

        Dirent temp{};
        temp.inum = 0;
        temp.isFile = false;
        temp.valid = true;
        char dot[] = ".";
        char dotDot[] = "..";
        strcpy(temp.Name, dot);
        memcpy(&(root.Table[0]), &temp, sizeof(Dirent));
        strcpy(temp.Name, dotDot);
        memcpy(&(root.Table[1]), &temp, sizeof(Dirent));

        // Empty the directories

        Block dirBlock{};
        dirBlock.Directories[0] = root;
        // memcpy(&(dirBlock.Directories[0]), &root, sizeof(root));
        disk_->write(superBlock.super.Blocks - 1, dirBlock.data);

        Logger::instance().println("[SIMPLE_FS] Finished formatting disk!");
    }

    void SimpleFS::mount() {
        Logger::instance().println("[SIMPLE_FS] Mounting...");
        /// Sanity check
        checkDiskNotMounted();

        /// Read superBlock
        Block block;
        disk_->read(0, block.data);

        /// Check superBlock is valid
        kAssert(block.super.MagicNumber == MAGIC_NUMBER, "[SIMPLE_FS] Magic Number is invalid");
        SuperBlock validSuperBlock{block.super.Blocks};
        kAssert(block.super == validSuperBlock, "[SIMPLE_FS] SuperBlock is invalid");
        Logger::instance().println("[SIMPLE_FS] SuperBlock is valid");

        disk_->mount();

        /// Copy metadata
        MetaData = block.super;

        /// Allocate free block bitmap
        Logger::instance().println("[SIMPLE_FS] Allocating block bitmap...");
        occupied_block.resize(MetaData.Blocks);
        std::fill(occupied_block.begin(), occupied_block.end(), false);

        /// Allocate inode counter
        Logger::instance().println("[SIMPLE_FS] Allocating inode counter...");
        inode_counter.resize(MetaData.InodeBlocks);
        std::fill(inode_counter.begin(), inode_counter.end(), 0);

        /// Setting free bit map node 0 to true for superBlock
        occupied_block[0] = true;

        /// Read inode blocks
        for (uint32_t i = 1; i <= MetaData.InodeBlocks; i++) {
            disk_->read(i, block.data);

            for (auto &inode: block.inodes) {
                if (inode.Valid) {
                    inode_counter[i - 1]++;

                    /// Set free bit map for inode blocks
                    if (inode.Valid)
                        occupied_block[i] = true;

                    /// Set free bit map for direct pointers
                    for (uint32_t k = 0; k < POINTERS_PER_INODE; k++) {
                        if (inode.Direct[k]) {
                            kAssert(inode.Direct[k] < MetaData.dataEnd,
                                    "[SIMPLE_FS] Data pointer out of bounds!");
                            occupied_block[inode.Direct[k]] = true;
                        }
                    }

                    /// Set free bit map for indirect pointers
                    if (inode.Indirect) {
                        kAssert(inode.Indirect < MetaData.dataEnd,
                                "[SIMPLE_FS] Indirect pointer out of bounds!");
                        /// Mark indirect block as occupied
                        occupied_block[inode.Indirect] = true;
                        /// Read indirect block
                        Block indirect;
                        disk_->read(inode.Indirect, indirect.data);
                        /// Mark indirect pointer blocks as occupied
                        for (auto pointer: indirect.pointers) {
                            kAssert(pointer < MetaData.dataEnd,
                                    "[SIMPLE_FS] Indirect pointer out of bounds!");
                            occupied_block[pointer] = true;
                        }
                    }
                }
            }
        }

        /// Allocate dir_counter
        dir_counter.resize(MetaData.DirBlocks);
        std::fill(dir_counter.begin(), dir_counter.end(), 0);

        /// Iterate through the directories
        Block dirBlock;
        for (uint32_t dirs = 0; dirs < MetaData.DirBlocks; dirs++) {
            /// Read directory
            disk_->read(MetaData.Blocks - 1 - dirs, dirBlock.data);
            /// Increment dir counter for subdirectories
            for (auto &dir: dirBlock.Directories) {
                if (dir.Valid == 1) {
                    dir_counter[dirs]++;
                }
            }
            /// First directory is the root
            if (dirs == 0) {
                curr_dir = dirBlock.Directories[0];
            }
        }

        isMounted = true;
        Logger::instance().println("[SIMPLE_FS] Finished mount!");
    }

    ssize_t SimpleFS::create() {
        checkFsMounted();

        /// Read the superBlock
        Block block;
        disk_->read(0, block.data);

        /// Locate free inode in inode table
        for (uint32_t i = 1; i <= MetaData.InodeBlocks; i++) {
            /// Check if inode block is full
            if (inode_counter[i - 1] == INODES_PER_BLOCK)
                continue;
            /// Inode block is not full
            disk_->read(i, block.data);

            /// Find the first empty inode
            for (uint32_t j = 0; j < INODES_PER_BLOCK; j++) {
                /// Set the inode to default values
                if (!block.inodes[j].Valid) {
                    block.inodes[j].clear();
                    block.inodes[j].Valid = true;
                    occupied_block[i] = true;
                    inode_counter[i - 1]++;

                    disk_->write(i, block.data);

                    return (((i - 1) * INODES_PER_BLOCK) + j);
                }
            }
        }

        Logger::instance().println("[SIMPLE_FS] Failed to create inode!");
        return -1;
    }

    bool SimpleFS::load_inode(size_t inumber, Inode *node) {
        checkFsMounted();

        if ((inumber > MetaData.Inodes)) {
            Logger::instance().println("[SIMPLE_FS] Invalid inode! %X", inumber);
            return false;
        }

        Block block;

        /// Find index of inode in the inode table
        size_t i = inumber / INODES_PER_BLOCK;
        size_t j = inumber % INODES_PER_BLOCK;

        /// Load the inode into Inode *node
        if (inode_counter[i]) {
            disk_->read(i + 1, block.data);
            if (block.inodes[j].Valid) {
                *node = block.inodes[j];
                return true;
            }
            Logger::instance().println("[SIMPLE_FS] Inode is invalid!");
        }

        Logger::instance().println("[SIMPLE_FS] Inode counter is wrong or inode is invalid!");
        return false;
    }

    bool SimpleFS::remove(size_t inumber) {
        checkFsMounted();

        Inode node{};

        /// Check if the node is valid; if yes, then load the inode
        if (load_inode(inumber, &node)) {
            node.Valid = false;
            node.Size = 0;

            /**- Decrement the corresponding inode block in inode counter
             * if the inode counter decreases to 0, then set the free bit map to false */
            if (!(--inode_counter[inumber / INODES_PER_BLOCK])) {
                occupied_block[inumber / INODES_PER_BLOCK + 1] = false;
            }

            /// Free direct blocks
            for (uint32_t i = 0; i < POINTERS_PER_INODE; i++) {
                occupied_block[node.Direct[i]] = false;
                node.Direct[i] = 0;
            }

            /// Free indirect blocks
            if (node.Indirect) {
                Block indirect;
                disk_->read(node.Indirect, indirect.data);
                occupied_block[node.Indirect] = false;
                node.Indirect = 0;

                for (auto indirectPtr: indirect.pointers) {
                    if (indirectPtr)
                        occupied_block[indirectPtr] = false;
                }
            }

            Block block;
            disk_->read(inumber / INODES_PER_BLOCK + 1, block.data);
            block.inodes[inumber % INODES_PER_BLOCK] = node;
            disk_->write(inumber / INODES_PER_BLOCK + 1, block.data);

            return true;
        }

        return false;
    }

    ssize_t SimpleFS::stat(size_t inumber) {
        checkFsMounted();

        Inode node{};

        /// Load inode; if valid, return its size
        if (load_inode(inumber, &node))
            return node.Size;

        return -1;
    }

    void SimpleFS::read_helper(uint32_t blockNum, int offset, int *length, uint8_t **data, uint8_t **ptr) {
        /// Read the block from disk and change the pointers accordingly
        disk_->read(blockNum, *ptr);
        *data += offset;
        *ptr += BLOCK_SIZE;
        *length -= (BLOCK_SIZE - offset);
    }

    ssize_t SimpleFS::read(size_t inumber, uint8_t *data, int length, size_t offset) {
        checkFsMounted();

        /// IMPORTANT: start reading from index = offset
        auto size_inode = stat(inumber);

        /**- if offset is greater than size of inode, then no data can be read
         * if length + offset exceeds the size of inode, adjust length accordingly
        */
        if ((int) offset >= size_inode)
            return 0;
        else if (length + (int) offset > size_inode)
            length = size_inode - (int) offset;

        Inode node{};

        /// Data is head; ptr is tail
        uint8_t *ptr = data;
        int to_read = length;

        /// Load inode; if invalid, return error
        if (!load_inode(inumber, &node))
            return -1;

        /// The offset is within direct pointers
        if (offset < POINTERS_PER_INODE * BLOCK_SIZE) {
            /// Calculate the node to start reading from
            uint32_t direct_node = offset / BLOCK_SIZE;
            offset %= BLOCK_SIZE;

            /// Check if the direct node is valid
            if (!node.Direct[direct_node]) {
                /// Inode has no stored data
                return 0;
            }

            read_helper(node.Direct[direct_node++], offset, &length, &data, &ptr);

            /// Read the direct blocks
            while (length > 0 && direct_node < POINTERS_PER_INODE && node.Direct[direct_node]) {
                read_helper(node.Direct[direct_node++], 0, &length, &data, &ptr);
            }

            /// If length <= 0, then enough data has been read
            if (length <= 0)
                return to_read;

            /// More data is to be read

            /// Check if all the direct nodes have been read completely and if the indirect pointer is valid
            if (direct_node == POINTERS_PER_INODE && node.Indirect) {
                Block indirect;
                disk_->read(node.Indirect, indirect.data);

                /// Read the indirect nodes
                for (auto pointer: indirect.pointers) {
                    if (pointer && length > 0) {
                        read_helper(pointer, 0, &length, &data, &ptr);
                    } else break;
                }

                /// If length <= 0, then enough data has been read
                if (length <= 0)
                    return to_read;
                /// Data exhausted, but the length requested was bigger
                /// Logically, this should never happen
                return (to_read - length);
            } else {
                /// Data exhausted, but the length requested was bigger
                /// Logically, this should never happen
                return (to_read - length);
            }
        } else {
            /// Offset begins in the indirect block
            /// Check if the indirect node is valid

            if (!node.Indirect) {
                /// The indirect node is invalid
                return 0;
            }

            /// Change offset accordingly and find the indirect node to start reading from
            offset -= (POINTERS_PER_INODE * BLOCK_SIZE);
            uint32_t indirect_node = offset / BLOCK_SIZE;
            offset %= BLOCK_SIZE;

            Block indirect;
            disk_->read(node.Indirect, indirect.data);

            if (indirect.pointers[indirect_node] && length > 0) {
                read_helper(indirect.pointers[indirect_node++], offset, &length, &data, &ptr);
            }

            /// Iterate through the indirect nodes
            for (uint32_t i = indirect_node; i < POINTERS_PER_BLOCK; i++) {
                if (indirect.pointers[i] && length > 0) {
                    read_helper(indirect.pointers[i], 0, &length, &data, &ptr);
                } else break;
            }

            /// If length <= 0, then enough data has been read
            if (length <= 0)
                return to_read;

            /// Data exhausted, but the length requested was bigger
            /// Logically, this should never happen
            return (to_read - length);
        }
    }

    uint32_t SimpleFS::allocate_block() {
        checkFsMounted();

        /// Iterate through the free bit map and allocate the first free block
        for (uint32_t i = MetaData.dataStart; i < MetaData.dataEnd; i++) {
            if (!occupied_block[i]) {
                occupied_block[i] = true;
                return i;
            }
        }

        /// Disk is full
        return 0;
    }


    bool SimpleFS::check_allocation(Inode *node, int read, int orig_offset, uint32_t &blockNum, bool write_indirect,
                                    Block indirect) {
        checkFsMounted();

        /// If blockNum is 0, then allocate a new block
        if (!blockNum) {
            blockNum = allocate_block();
            /// Set size of node and write back to disk if it is an indirect node
            if (!blockNum) {
                node->Size = read + orig_offset;
                if (write_indirect)
                    disk_->write(node->Indirect, indirect.data);
                return false;
            }
        }
        return true;
    }
}