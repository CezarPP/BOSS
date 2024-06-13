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

namespace vfs {
    class FileSystem {
    protected:
        Disk *disk_;
    public:
        explicit FileSystem(Disk *disk) : disk_(disk) {}

        virtual ~FileSystem() = default;

        virtual void init() {}

        // virtual size_t read(const Path &file_path, char *buffer, size_t count, size_t offset, size_t &read) = 0;

        // virtual size_t write(const Path &file_path, const char *buffer, size_t count, size_t offset, size_t &written) = 0;

        // virtual size_t clear(const Path &file_path, size_t count, size_t offset, size_t &written) = 0;

        // virtual size_t get_file(const Path &file_path, vfs::file &file) = 0;

        // virtual size_t ls(const Path &file_path, std::vector<vfs::file> &contents) = 0;

        // virtual size_t touch(const Path &file_path) = 0;

        // virtual size_t mkdir(const Path &file_path) = 0;

        // virtual size_t rm(const Path &file_path) = 0;
    };
}