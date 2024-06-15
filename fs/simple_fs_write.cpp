/*
 * simple_fs_write.cpp
 *
 *  Created on: 6/13/24.
 *      Author: Cezar PP
 */
#include "fs/simple_fs.h"

namespace simple_fs {
    ssize_t SimpleFS::write_ret(size_t inumber, Inode *node, int ret) {
        checkFsMounted();

        /// Find index of inode in the inode table
        int i = inumber / INODES_PER_BLOCK;
        int j = inumber % INODES_PER_BLOCK;

        /// Store the node into the block
        Block block{};
        disk_->read(i + 1, block.data);
        block.inodes[j] = *node;
        disk_->write(i + 1, block.data);

        return (size_t) ret;
    }

    void SimpleFS::read_buffer(int offset, int *read, int length, const uint8_t *data, uint32_t blockNum) {
        checkFsMounted();

        Block block;

        /// Read data into ptr and change pointers accordingly
        for (int i = offset; i < (int) BLOCK_SIZE && *read < length; i++) {
            block.data[i] = data[*read];
            *read = *read + 1;
        }
        disk_->write(blockNum, block.data);
    }


    ssize_t SimpleFS::write(size_t inumber, const uint8_t *data, int length, size_t offset) {
        checkFsMounted();

        Inode node{};
        Block indirect;
        int read = 0;
        int orig_offset = offset;

        /// Insufficient size
        if (length + offset > (POINTERS_PER_BLOCK + POINTERS_PER_INODE) * BLOCK_SIZE) {
            return -1;
        }

        /**- if the inode is invalid, allocate inode.
         *  need not write to disk right now; will be taken care of in write_ret()
         */
        if (!load_inode(inumber, &node)) {
            node.Valid = true;
            node.Size = length + offset;
            for (uint32_t ii = 0; ii < POINTERS_PER_INODE; ii++) {
                node.Direct[ii] = 0;
            }
            node.Indirect = 0;
            inode_counter[inumber / INODES_PER_BLOCK]++;
            occupied_block[inumber / INODES_PER_BLOCK + 1] = true;
        } else {
            /// Set size of the node
            node.Size = std::max((int) node.Size, length + (int) offset);
        }

        /// Check if the offset is within direct pointers
        if (offset < POINTERS_PER_INODE * BLOCK_SIZE) {
            /// Find the first node to start writing at and change offset accordingly
            int direct_node = offset / BLOCK_SIZE;
            offset %= BLOCK_SIZE;

            /// Check if the node is valid; if invalid; allocates a block and if no block is available, returns false
            if (!check_allocation(&node, read, orig_offset, node.Direct[direct_node], false, indirect)) {
                return write_ret(inumber, &node, read);
            }
            /// Read from data buffer
            read_buffer(offset, &read, length, data, node.Direct[direct_node++]);

            /// Enough data has been read from data buffer
            if (read == length)
                return write_ret(inumber, &node, length);

            /**- store in direct pointers till either one of the two things happen:
            * 1. all the data is stored in the direct pointers
            * 2. the data is stored in indirect pointers
            */
            /// Start writing into direct nodes
            for (int i = direct_node; i < (int) POINTERS_PER_INODE; i++) {
                /// Check if the node is valid; if invalid; allocates a block and if no block is available, returns false
                if (!check_allocation(&node, read, orig_offset, node.Direct[direct_node], false, indirect)) {
                    return write_ret(inumber, &node, read);
                }
                read_buffer(0, &read, length, data, node.Direct[direct_node++]);

                /// Enough data has been read from data buffer
                if (read == length)
                    return write_ret(inumber, &node, length);
            }

            /// Check if the indirect node is valid
            if (node.Indirect)
                disk_->read(node.Indirect, indirect.data);
            else {
                /// Check if the node is valid; if invalid; allocates a block and if no block is available, returns false
                if (!check_allocation(&node, read, orig_offset, node.Indirect, false, indirect)) {
                    return write_ret(inumber, &node, read);
                }
                disk_->read(node.Indirect, indirect.data);

                /// Initialise the indirect nodes
                for (auto &indirectPtr: indirect.pointers)
                    indirectPtr = 0;
            }

            /// Write into indirect nodes
            for (unsigned int &indirectPtr: indirect.pointers) {
                /// Check if the node is valid; if invalid; allocates a block and if no block is available, returns false
                if (!check_allocation(&node, read, orig_offset, indirectPtr, true, indirect)) {
                    return write_ret(inumber, &node, read);
                }
                read_buffer(0, &read, length, data, indirectPtr);

                /// Enough data has been read from data buffer
                if (read == length) {
                    disk_->write(node.Indirect, indirect.data);
                    return write_ret(inumber, &node, length);
                }
            }

            /// Space exhausted
            disk_->write(node.Indirect, indirect.data);
            return write_ret(inumber, &node, read);
        } else {
            /// Offset begins in indirect blocks
            /// Find the first indirect node to write into and change offset accordingly
            offset -= (BLOCK_SIZE * POINTERS_PER_INODE);
            int indirect_node = offset / BLOCK_SIZE;
            offset %= BLOCK_SIZE;

            /// Check if the indirect node is valid
            if (node.Indirect)
                disk_->read(node.Indirect, indirect.data);
            else {
                /// Check if the node is valid; if invalid; allocates a block and if no block is available, returns false
                if (!check_allocation(&node, read, orig_offset, node.Indirect, false, indirect)) {
                    return write_ret(inumber, &node, read);
                }
                disk_->read(node.Indirect, indirect.data);

                /// Initialise the indirect nodes
                for (auto &indirectPtr: indirect.pointers) {
                    indirectPtr = 0;
                }
            }

            /// Check if the node is valid; if invalid; allocates a block and if no block is available, returns false
            if (!check_allocation(&node, read, orig_offset, indirect.pointers[indirect_node], true, indirect)) {
                return write_ret(inumber, &node, read);
            }
            read_buffer(offset, &read, length, data, indirect.pointers[indirect_node++]);

            /// Enough data has been read from data buffer
            if (read == length) {
                disk_->write(node.Indirect, indirect.data);
                return write_ret(inumber, &node, length);
            }

            /// Write into indirect nodes
            for (int j = indirect_node; j < (int) POINTERS_PER_BLOCK; j++) {
                /// Check if the node is valid; if invalid; allocates a block and if no block is available, returns false
                if (!check_allocation(&node, read, orig_offset, indirect.pointers[j], true, indirect)) {
                    return write_ret(inumber, &node, read);
                }
                read_buffer(0, &read, length, data, indirect.pointers[j]);

                /// Enough data has been read from data buffer
                if (read == length) {
                    disk_->write(node.Indirect, indirect.data);
                    return write_ret(inumber, &node, length);
                }
            }

            /// space exhausted
            disk_->write(node.Indirect, indirect.data);
            return write_ret(inumber, &node, read);
        }
    }
}