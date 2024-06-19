/*
 * simple_fs_files.cpp
 *
 *  Created on: 6/13/24.
 *      Author: Cezar PP
 */

#include "fs/simple_fs.h"
#include "util/console_printer.h"
#include "std/expected.h"

namespace simple_fs {
    Directory SimpleFS::rm_helper(Directory dir, const char name[NAME_SIZE]) {
        checkFsMounted();

        /// Get the offset for removal
        int offset = dir_lookup(dir, name);
        if (offset == -1) {
            Console::instance().println("No such file/directory");
            dir.Valid = false;
            return dir;
        }

        ///   Check if directory
        if (dir.Table[offset].isFile == 0) {
            return rmdir_helper(dir, name);
        }

        /// Get inumber
        uint32_t inum = dir.Table[offset].inum;

        // printf("%u\n", inum);

        /// Remove the inode
        if (!remove(inum)) {
            Console::instance().println("Failed to remove Inode");
            dir.Valid = false;
            return dir;
        }

        /// Remove the entry
        dir.Table[offset].valid = false;

        /// Write back the changes
        write_dir_back(dir);
        return dir;
    }

    bool SimpleFS::touch(const char name[NAME_SIZE]) {
        checkFsMounted();

        /// Check if such file exists
        for (auto &file: curr_dir.Table)
            if (file.valid && strcmp(file.Name, name) == 0) {
                // Console::instance().printStr("File already exists");
                return false;
            }

        /// Allocate new inode for the file
        const auto new_node_idx = create();
        Logger::instance().println("[SIMPLE_FS] Creating file with inode number %X", new_node_idx);
        if (new_node_idx == -1) {
            Console::instance().println("Error creating new inode\n");
            return false;
        }

        /// Add the directory entry in the curr_directory
        Directory temp = add_dir_entry(curr_dir, new_node_idx, 1, name);
        if (temp.Valid == 0) {
            Console::instance().println("Error adding new file");
            return false;
        }
        curr_dir = temp;

        /// Write back the changes
        write_dir_back(curr_dir);

        return true;
    }

    std::expected<uint32_t> SimpleFS::getInode(const char name[NAME_SIZE]) {
        checkFsMounted();

        /// Check if such file exists
        for (auto &file: curr_dir.Table)
            if (file.valid && strcmp(file.Name, name) == 0) {
                return file.inum;
            }
        Logger::instance().println("[SIMPLE_FS] File does not exist!");

        return std::make_unexpected<uint32_t>((size_t) 0);
    }

    bool SimpleFS::cd(const char name[NAME_SIZE]) {
        checkFsMounted();

        int offset = dir_lookup(curr_dir, name);
        if ((offset == -1) || (curr_dir.Table[offset].isFile == 1)) {
            Console::instance().println("No such directory");
            return false;
        }

        /// Read the dirBlock from the disk
        Directory temp = read_dir_from_offset(offset);
        if (temp.Valid == 0) {
            return false;
        }
        curr_dir = temp;
        return true;
    }

    bool SimpleFS::ls(std::vector<vfs::file> &contents) {
        checkFsMounted();

        char name[] = ".";
        return ls_dir(name, contents);
    }

    bool SimpleFS::rm(const char name[]) {
        Directory temp = rm_helper(curr_dir, name);
        if (temp.Valid == 1) {
            curr_dir = temp;
            return true;
        }
        return false;
    }
}