/*
 * simple_fs_dirs.cpp
 *
 *  Created on: 6/13/24.
 *      Author: Cezar PP
 */

#include "fs/simple_fs.h"
#include "std/cstring.h"
#include "util/console_printer.h"

namespace simple_fs {
    Directory SimpleFS::add_dir_entry(Directory dir, uint32_t inum, uint32_t type, const char name[]) {
        Directory tempdir = dir;

        /// Find a free Dirent in the Table to be modified
        uint32_t idx = 0;
        for (; idx < ENTRIES_PER_DIR; idx++)
            if (tempdir.Table[idx].valid == 0) {
                break;
            }

        /// If No Dirent Found, Error
        if (idx == ENTRIES_PER_DIR) {
            kPanic("[SIMPLE_FS] Directory entry limit reached..");
            tempdir.Valid = false;
            return tempdir;
        }

        ///  Free Dirent found, add the new one to the table
        tempdir.Table[idx].inum = inum;
        tempdir.Table[idx].type = type;
        tempdir.Table[idx].valid = true;
        strcpy(tempdir.Table[idx].Name, name);

        return tempdir;
    }


    Directory SimpleFS::read_dir_from_offset(uint32_t offset) {
        /**-   Sanity Check  */
        if (offset >= ENTRIES_PER_DIR || (curr_dir.Table[offset].valid == 0) || (curr_dir.Table[offset].type != 0)) {
            Directory temp;
            temp.Valid = false;
            return temp;
        }

        /// Get offsets and indexes
        uint32_t inum = curr_dir.Table[offset].inum;
        uint32_t block_idx = (inum / DIR_PER_BLOCK);
        uint32_t block_offset = (inum % DIR_PER_BLOCK);

        /// Read block
        Block block{};
        disk_->read(MetaData.Blocks - 1 - block_idx, block.data);
        return block.Directories[block_offset];
    }

    void SimpleFS::write_dir_back(Directory dir) {
        /// Get block offset and index
        uint32_t block_idx = (dir.inum / DIR_PER_BLOCK);
        uint32_t block_offset = (dir.inum % DIR_PER_BLOCK);

        /// Read block
        Block block{};
        disk_->read(MetaData.Blocks - 1 - block_idx, block.data);
        block.Directories[block_offset] = dir;

        /// Write the dirBlock
        disk_->write(MetaData.Blocks - 1 - block_idx, block.data);
    }

    int SimpleFS::dir_lookup(Directory dir, const char name[]) {
        /// Search the curr_dir.Table for name
        for (uint32_t offset = 0; offset < ENTRIES_PER_DIR; offset++)
            if (dir.Table[offset].valid == 1 && strcmp(dir.Table[offset].Name, name) == 0)
                return offset;

        return -1;
    }

    bool SimpleFS::ls_dir(const char name[]) {
        checkFsMounted();

        /// Get the directory entry offset
        int offset = dir_lookup(curr_dir, name);
        if (offset == -1) {
            Console::instance().println("No such directory");
            return false;
        }

        /// Read directory from block
        Directory dir = read_dir_from_offset(offset);

        /// Sanity checks
        if (dir.Valid == 0) {
            Console::instance().println("Invalid directory");
            return false;
        }

        /// Print Directory Data
        Console::instance().println("   inum    |       name       | type");
        for (auto temp: dir.Table) {
            if (temp.valid) {
                if (temp.type == 1)
                    Console::instance().println("%-10d | %-16s | %-5s", temp.inum, temp.Name, "file");
                else
                    Console::instance().println("%-10d | %-16s | %-5s", temp.inum, temp.Name, "dir");
            }
        }
        return true;
    }

    bool SimpleFS::mkdir(const char name[NAME_SIZE]) {
        checkFsMounted();

        /// Find empty dirBlock
        uint32_t block_idx = 0;
        for (; block_idx < MetaData.DirBlocks; block_idx++)
            if (dir_counter[block_idx] < DIR_PER_BLOCK)
                break;

        if (block_idx == MetaData.DirBlocks) {
            Console::instance().println("Directory limit reached");
            return false;
        }

        /// Read empty dirBlock
        Block block;
        disk_->read(MetaData.Blocks - 1 - block_idx, block.data);

        /// Find empty directory in dirBlock
        uint32_t offset = 0;
        for (; offset < DIR_PER_BLOCK; offset++)
            if (block.Directories[offset].Valid == 0)
                break;

        kAssert(offset < DIR_PER_BLOCK, "[SIMPLE_FS] We know this dirBlock not to be full");
        if (offset == DIR_PER_BLOCK)
            return false;

        /// Create new directory
        Directory new_dir, temp;
        memset(&new_dir, 0, sizeof(Directory));
        new_dir.inum = block_idx * DIR_PER_BLOCK + offset;
        new_dir.Valid = true;
        strcpy(new_dir.Name, name);

        /// Create 2 new entries for "." and ".."
        char tstr1[] = ".", tstr2[] = "..";
        temp = new_dir;
        temp = add_dir_entry(temp, temp.inum, 0, tstr1);
        temp = add_dir_entry(temp, curr_dir.inum, 0, tstr2);

        if (temp.Valid == 0) {
            Console::instance().println("Error creating new directory");
            return false;
        }
        new_dir = temp;

        /// Add new entry to the curr_dir
        temp = add_dir_entry(curr_dir, new_dir.inum, 0, new_dir.Name);
        if (temp.Valid == 0) {
            Console::instance().println("Error adding new directory");
            return false;
        }
        curr_dir = temp;

        /// Write the new directory back to the disk
        write_dir_back(new_dir);

        /// Write the curr_dir back to the disk
        write_dir_back(curr_dir);

        /// Increment the counter
        dir_counter[block_idx]++;

        return true;
    }

    Directory SimpleFS::rmdir_helper(Directory parent, const char name[]) {
        /// Initialization
        Directory dir, temp;
        uint32_t inum, blk_idx, blk_off;
        Block blk;

        checkFsMounted();

        /// Get offset of the directory to be removed
        int offset = dir_lookup(parent, name);
        if (offset == -1) {
            dir.Valid = false;
            return dir;
        }

        /// Get block
        inum = parent.Table[offset].inum;
        blk_idx = inum / DIR_PER_BLOCK;
        blk_off = inum % DIR_PER_BLOCK;
        disk_->read(MetaData.Blocks - 1 - blk_idx, blk.data);

        /// Check Directory
        dir = blk.Directories[blk_off];
        if (dir.Valid == 0) {
            return dir;
        }

        /// Check if it is root directory
        if (strcmp(dir.Name, curr_dir.Name) == 0) {
            Console::instance().println("Current Directory cannot be removed.\n");
            dir.Valid = false;
            return dir;
        }

        /// Remove all Dirent in the directory to be removed
        for (uint32_t ii = 0; ii < ENTRIES_PER_DIR; ii++) {
            if ((ii > 1) && (dir.Table[ii].valid == 1)) {
                temp = rm_helper(dir, dir.Table[ii].Name);
                if (temp.Valid == 0)
                    return temp;
                dir = temp;
            }
            dir.Table[ii].valid = false;
        }

        /// Read the block again, because the block may have changed by Dirent
        disk_->read(MetaData.Blocks - 1 - blk_idx, blk.data);

        /// Write it back
        dir.Valid = false;
        blk.Directories[blk_off] = dir;
        disk_->write(MetaData.Blocks - 1 - blk_idx, blk.data);

        /// Remove it from the parent
        parent.Table[offset].valid = false;
        write_dir_back(parent);

        /// Update the counter
        dir_counter[blk_idx]--;

        return parent;
    }

    bool SimpleFS::rmdir(char name[NAME_SIZE]) {
        Directory temp = rmdir_helper(curr_dir, name);
        if (temp.Valid == 0) {
            curr_dir = temp;
            return true;
        }
        return false;
    }


    void SimpleFS::stat() {
        checkFsMounted();

        /// Read Super Block and print MetaData
        Block blk;
        disk_->read(0, blk.data);
        Console::instance().println("Total Blocks : %d", blk.super.Blocks);
        Console::instance().println("Total Directory Blocks : %d", blk.super.DirBlocks);
        Console::instance().println("Total Inode Blocks : %d", blk.super.InodeBlocks);
        Console::instance().println("Total Inode : %d", blk.super.Inodes);

        Console::instance().println("Max Directories per block : %d", DIR_PER_BLOCK);
        Console::instance().println("Max Namsize : %d", NAME_SIZE);
        Console::instance().println("Max Inodes per block : %d", INODES_PER_BLOCK);
        Console::instance().println("Max Entries per directory : %d", ENTRIES_PER_DIR);

        /// Read directory blocks
        for (uint32_t blk_idx = 0; blk_idx < MetaData.DirBlocks; blk_idx++) {
            disk_->read(MetaData.Blocks - 1 - blk_idx, blk.data);
            Console::instance().println("Block %d", blk_idx);

            /// Read Directories in each directory block
            for (uint32_t offset = 0; offset < DIR_PER_BLOCK; offset++) {
                Directory dir = blk.Directories[offset];
                if (dir.Valid) {
                    Console::instance().println("    Offset %d: Directory Name - \"%s\"", offset, dir.Name);

                    /// Read Table Entries for each directory
                    for (uint32_t tbl_idx = 0; tbl_idx < ENTRIES_PER_DIR; tbl_idx++) {
                        Dirent ent = dir.Table[tbl_idx];
                        if (ent.valid) {
                            Console::instance().println(
                                    "        tbl_idx %d: Entry Name - \"%s\", type - %d, inum - %d\n", tbl_idx,
                                    ent.Name,
                                    ent.type, ent.inum);
                        }
                    }
                }
            }
        }
    }
}