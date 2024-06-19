/*
 * file_system.h
 *
 *  Created on: 5/7/24.
 *      Author: Cezar PP
 */


#pragma once

#include "path.h"
#include "file.h"
#include "drivers/disk_driver.h"
#include "std/expected.h"

namespace vfs {
    class FileSystem {
    protected:
        Disk *disk_;
    public:
        explicit FileSystem(Disk *disk) : disk_(disk) {}

        virtual ~FileSystem() = default;

        virtual void mount() = 0;

        virtual ssize_t read(size_t inum, uint8_t *buffer, int length, size_t offset) = 0;

        virtual ssize_t write(size_t inumber, const uint8_t *data, int length, size_t offset) = 0;

        virtual bool ls(std::vector<vfs::file> &contents) = 0;

        virtual bool touch(const char *file_path) = 0;

        virtual bool mkdir(const char name[]) = 0;

        virtual bool rm(const char name[]) = 0;

        virtual std::expected<size_t> getInode(const char *name) = 0;

        virtual bool rmdir(const char name[]) = 0;

        virtual bool cd(const char name[]) = 0;

        virtual void test() = 0;
    };
}