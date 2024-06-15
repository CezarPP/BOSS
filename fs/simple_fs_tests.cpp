/*
 * simple_fs_tests.cpp
 *
 *  Created on: 6/15/24.
 *      Author: Cezar PP
 */

#include "fs/simple_fs.h"

namespace simple_fs {
    void test_create_file(SimpleFS &fs) {
        bool touchSucceeded = fs.touch("new_file");
        kAssert(touchSucceeded, "[SIMPLE_FS] Failed to create new file!");

        auto file_offset = fs.dir_lookup(fs.curr_dir, "new_file");
        kAssert(file_offset != -1, "[SIMPLE_FS] Lookup failed!");

        Inode node{};
        bool loaded = fs.load_inode(fs.curr_dir.Table[file_offset].inum, &node);
        kAssert(loaded, "[SIMPLE_FS] Failed to load inode of new file");

        // Assuming Inode structure has a size attribute that should initially be 0
        kAssert(node.Size == 0, "[SIMPLE_FS] New file size should be zero");
    }

    void test_write_to_file(SimpleFS &fs) {
        bool touchSucceeded = fs.touch("test_file");
        kAssert(touchSucceeded, "[SIMPLE_FS] Failed to create file!");

        auto fileOffset = fs.dir_lookup(fs.curr_dir, "test_file");
        auto inodeNumber = fs.curr_dir.Table[fileOffset].inum;

        constexpr const auto SIZE_TO_READ = BLOCK_SIZE;
        const uint8_t data[SIZE_TO_READ] = {1, 2, 3, 4, 5};
        const auto bytes_written = fs.write(inodeNumber, data, SIZE_TO_READ, 0);
        kAssert(bytes_written == SIZE_TO_READ, "[SIMPLE_FS] Failed to write correct amount of bytes");

        uint8_t buffer[BLOCK_SIZE] = {};
        auto bytes_read = fs.read(inodeNumber, buffer, SIZE_TO_READ, 0);

        kAssert(bytes_read == SIZE_TO_READ, "[SIMPLE_FS] Failed to read back correct amount of bytes");
        kAssert(std::equal(data, data + SIZE_TO_READ, buffer), "[SIMPLE_FS] Data mismatch on read back");
    }

    void test_create_directory(SimpleFS &fs) {
        bool created = fs.mkdir("new_directory");
        kAssert(created, "[SIMPLE_FS] Failed to create directory");

        // Assuming directories are listed as a special inode entry or separate structure
        int dir_idx = fs.dir_lookup(fs.curr_dir, "new_directory");
        kAssert(dir_idx >= 0, "[SIMPLE_FS] Directory not found in current directory listing");
    }

    void test_remove_directory(SimpleFS &fs) {
        const char *toRemoveName = "to_remove_dir";
        bool created = fs.mkdir(toRemoveName);
        kAssert(created, "[SIMPLE_FS] Failed to create directory");

        bool removed = fs.rmdir(toRemoveName);
        kAssert(removed, "[SIMPLE_FS] Failed to remove directory");

        int dir_idx = fs.dir_lookup(fs.curr_dir, "to_remove_dir");
        kAssert(dir_idx == -1, "[SIMPLE_FS] Directory still exists after removal");
    }

    void SimpleFS::test() {
        Logger::instance().println("[SIMPLE_FS] Testing...");

        Logger::instance().println("[SIMPLE_FS] Testing file creation...");
        test_create_file(*this);

        Logger::instance().println("[SIMPLE_FS] Testing writing to file...");
        test_write_to_file(*this);

        Logger::instance().println("[SIMPLE_FS] Testing directory creation...");
        test_create_directory(*this);

        Logger::instance().println("[SIMPLE_FS] Testing directory removal...");
        test_remove_directory(*this);

        Logger::instance().println("[SIMPLE_FS] Testing succeeded!");
    }
}