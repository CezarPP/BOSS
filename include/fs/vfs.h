/*
 * vfs.h
 *
 *  Created on: 5/4/24.
 *      Author: Cezar PP
 */


#pragma once


#include "../std/expected.h"

#include "../include/fs/handles.h"
#include "../include/fs/fs_errors.h"
#include "drivers/ata.h"
#include "file_system.h"

/*
 * ALL THE FOLLOWING REFER TO LINUX
 * The VFS is object-oriented. It has 4 primary object types: superBlock, inode, dentry, file
 * An operation object is stored for each of these primary objects
 * SuperBlock - filesystem control information
 * super_operations: e.g. write_inode(), sync_fs()
 *
 * inode (index node) - file metadata
 * inode_operations: e.g. create(), link()
 *
 * dentry - directory entry, a single component of a path, might be a file
 * dentries might also include mount points. In the path /mnt/cdrom/foo, the components /, mnt, cdrom, and foo are all dentry objects.
 * dentries do not correspond to any on-disk data structure, they are created on the fly
 * dentry_operations: e.g. d_compare(), d_delete()
 *
 * file object
 * file_operations: e.g. read(), write()
 * Each mount point has a vfsmount structure
 *
 * ABOUT VIRTUAL FILE SYSTEMS IN GENERAL:
 * https://wiki.osdev.org/VFS
 * We are going to use a Mount Point List
 */


namespace vfs {
    using fd_t = size_t;

    /**
     * Enumeration for all supported partition types
    */
    enum class PartitionType {
        SIMPLE_FS = 1, /// SimpleFS
        UNKNOWN = 100 ///< Unknown file system
    };


    struct MountedFS {
        vfs::PartitionType fs_type{PartitionType::UNKNOWN};
        Path mount_point;
        vfs::FileSystem *file_system{};

        MountedFS(vfs::PartitionType type, Path mp, vfs::FileSystem *fs)
                : fs_type(type), mount_point(std::move(mp)), file_system(fs) {
            // Nothing else to init
        }
    };

    void init(Disk *disk);

    constexpr const size_t OPEN_CREATE = 0x1;
    std::expected<fd_t> open(const char *filePath, size_t flags);

    void close(fd_t fd);

    std::expected<void> mkdir(const char *file);

    std::expected<void> rm(const char *file);

    std::expected<void> rmDir(const char* file);

    std::expected<size_t> read(fd_t fd, uint8_t *buffer, size_t count, size_t offset = 0);

    std::expected<size_t> write(fd_t fd, const uint8_t *buffer, size_t count, size_t offset = 0);

    std::expected<void> ls(std::vector<file>& contents);

    std::expected<void> mount(PartitionType type, const char *mount_point, Disk *disk);

    // vfs should not have cd, it should be independent
    std::expected<void> cd(const char* dir);

    std::expected<ssize_t> stat(fd_t fd);
}