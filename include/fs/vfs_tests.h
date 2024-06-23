/*
 * vfs_tests.cpp
 *
 *  Created on: 6/19/24.
 *      Author: Cezar PP
 */


#include "fs/vfs.h"
#include "fs/simple_fs_structures.h"

namespace vfs {
    constexpr const auto BLOCK_SIZE = simple_fs::BLOCK_SIZE;

    void test_create_file() {
        const char *fileName = "new_file4";
        auto fd = vfs::open(fileName, OPEN_CREATE);
        kAssert(fd, "[VFS] Failed to open new file!");

        auto readBytes = vfs::stat(fd.value());
        kAssert(readBytes && *readBytes == 0, "[VFS] New file should be empty");

        vfs::close(fd.value());
    }

    void test_write_to_file() {
        const char *fileName = "test_file1";
        auto fd = vfs::open(fileName, OPEN_CREATE);
        kAssert(fd, "[VFS] Failed to open file for writing!");

        constexpr const size_t SIZE_TO_WRITE = BLOCK_SIZE;
        uint8_t data[SIZE_TO_WRITE] = {1, 2, 3, 4, 5};
        auto writtenBytes = vfs::write(fd.value(), data, SIZE_TO_WRITE, 0);
        kAssert(writtenBytes && *writtenBytes == SIZE_TO_WRITE, "[VFS] Write operation failed");

        uint8_t buffer[BLOCK_SIZE] = {};
        auto readBytes = vfs::read(fd.value(), buffer, SIZE_TO_WRITE, 0);
        kAssert(readBytes && *readBytes == SIZE_TO_WRITE && std::equal(data, data + SIZE_TO_WRITE, buffer),
                "[VFS] Data mismatch on read back");

        vfs::close(fd.value());
    }

    void test_create_directory() {
        const char *dirName = "new_directory1";
        auto result = vfs::mkdir(dirName);
        kAssert(result, "[VFS] Failed to create directory");

        // Verify the directory creation by attempting to change into it
        auto cdResult = vfs::cd(dirName);
        kAssert(cdResult, "[VFS] Failed to change into new directory");
    }

    void test_remove_directory() {
        const char *dirName = "to_remove_dir2";
        auto made = vfs::mkdir(dirName);

        kAssert(made, "[VFS] Failed to make dir!");

        auto rmDirResult = vfs::rmDir(dirName);
        kAssert(rmDirResult, "[VFS] Failed to remove directory");
    }

    void test_change_directory() {
        const char *dirName = "test_dir2";
        auto madeDir = vfs::mkdir(dirName);
        kAssert(madeDir, "[VFS] Failed to make dir");

        auto cdResult = vfs::cd(dirName);
        kAssert(cdResult, "[VFS] Failed to change directory");

        cdResult = vfs::cd("..");
        kAssert(cdResult, "[VFS] Failed to change back to parent directory");
    }

    void test_list_directory() {
        auto created = vfs::mkdir("test_dir3");

        kAssert(created, "[VFS] Failed to create directory");

        auto changedDir = vfs::cd("test_dir3");

        kAssert(changedDir, "[VFS] Failed to change directory");

        auto opened = vfs::open("known_file3", OPEN_CREATE);

        kAssert(opened, "[VFS] Failed to create file with open");

        std::vector<vfs::file> contents;
        auto listed = vfs::ls(contents);
        kAssert(listed, "[VFS] Failed to list directory contents");

        bool found = std::any_of(contents.begin(), contents.end(), [](const vfs::file &file) {
            return file.fileName == "known_file3" && file.isFile;
        });
        kAssert(found, "[VFS] Known file not found during ls");
    }

    void test() {
        Logger::instance().println("[VFS] Starting tests...");


        Logger::instance().println("[VFS] Testing file creation...");
        test_create_file();

        Logger::instance().println("[VFS] Testing file write...");
        test_write_to_file();

        Logger::instance().println("[VFS] Testing directory creation...");
        test_create_directory();

        Logger::instance().println("[VFS] Testing directory removal...");
        test_remove_directory();

        Logger::instance().println("[VFS] Testing directory change...");
        test_change_directory();

        Logger::instance().println("[VFS] Testing directory list...");
        test_list_directory();

        Logger::instance().println("[VFS] All tests passed successfully!");
    }
}