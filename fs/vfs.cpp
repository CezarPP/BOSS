/*
 * vfs.cpp
 *
 *  Created on: 6/7/24.
 *      Author: Cezar PP
 */

#include "fs/vfs.h"
#include "arch/x86_64/exceptions.h"
#include "arch/x86_64/logging.h"
#include "fs/simple_fs.h"

namespace {
    constexpr const size_t OPEN_CREATE = 0x1;

    std::string partitionTypeToString(vfs::PartitionType type) {
        switch (type) {
            case vfs::PartitionType::SIMPLE_FS:
                return "SimpleFS";
            case vfs::PartitionType::UNKNOWN:
                return "Unknown";
            default:
                return "Invalid Type";
        }
    }

    std::vector<vfs::MountedFS> mount_point_list;

    void mountRoot(Disk *disk) {
        auto success = mount(vfs::PartitionType::SIMPLE_FS, "/", disk);
        kAssert(success, "Error mounting SIMPLE_FS");
    }

    Path getPath(const char *file_path) {
        Path p(file_path);
        if (!p.is_valid())
            return p;

        kAssert(!p.is_relative(), "Can't support relative paths at the moment...");
        return p;
    }

    vfs::MountedFS &getFs(const Path &base_path) {
        size_t best = 0;
        size_t best_match = 0;

        if (base_path.is_root()) {
            for (auto &mp: mount_point_list) {
                if (mp.mount_point.is_root()) {
                    return mp;
                }
            }
        }

        for (size_t i = 0; i < mount_point_list.size(); ++i) {
            auto &mp = mount_point_list[i];

            bool match = true;
            for (size_t j = 0; j < mp.mount_point.size() && j < base_path.size(); ++j) {
                if (mp.mount_point[j] != base_path[j]) {
                    match = false;
                    break;
                }
            }

            if (match && mp.mount_point.size() > best) {
                best = mp.mount_point.size();
                best_match = i;
            }
        }

        return mount_point_list[best_match];
    }

    vfs::FileSystem *getNewFs(vfs::PartitionType type, Disk *disk) {
        switch (type) {
            case vfs::PartitionType::SIMPLE_FS:
                return new simple_fs::SimpleFS(disk);
            default:
                kPanic("Unknown FS type");
                return nullptr;
        }
    }

}

std::expected<void> vfs::mount(PartitionType type, const char *mount_point, Disk *disk) {
    Path mpPath(mount_point);

    auto fs = getNewFs(type, disk);

    if (!fs) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_SYSTEM);
    }

    mount_point_list.emplace_back(type, mpPath, fs);

    Logger::instance().println("[VFS] Mounted file system at %s", mpPath.string());

    return {};
}

void vfs::init(Disk *disk) {
    mountRoot(disk);

    // Mount and test all file systems
    for (auto &mp: mount_point_list) {
        mp.file_system->mount();
        mp.file_system->test();
    }
}

std::expected<vfs::fd_t> vfs::open(const char *filePath, size_t flags) {
    const auto base_path = getPath(filePath);

    if (!base_path.is_valid()) {
        return std::make_unexpected<fd_t>(std::ERROR_INVALID_FILE_PATH);
    }

    auto &fs = getFs(base_path);
    // Touch in case it does not exist
    if (flags & OPEN_CREATE)
        fs.file_system->touch(filePath);

    auto inodeExpected = fs.file_system->getInode(filePath);

    if (inodeExpected)
        return handles::register_new_handle(inodeExpected.value());

    return std::make_unexpected<vfs::fd_t>(std::ERROR_UNKNOWN);
}

void vfs::close(fd_t fd) {
    if (handles::has_handle(fd)) {
        handles::release_handle(fd);
    }
}

std::expected<void> vfs::mkdir(const char *file_path) {
    auto base_path = getPath(file_path);

    if (!base_path.is_valid()) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_PATH);
    }

    auto &fs = getFs(base_path);

    bool success = fs.file_system->mkdir(file_path);
    if (success)
        return std::make_expected();

    Logger::instance().println("[VFS] Error mkdir");
    return std::make_unexpected<void>(std::ERROR_UNKNOWN);
}

std::expected<void> vfs::rm(const char *file_path) {
    auto base_path = getPath(file_path);

    if (!base_path.is_valid()) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_PATH);
    }

    auto &fs = getFs(base_path);

    bool success = fs.file_system->rm(file_path);
    if (success)
        return std::make_expected();

    Logger::instance().println("[VFS] Error rm");
    return std::make_unexpected<void>(std::ERROR_UNKNOWN);
}

std::expected<void> vfs::rmDir(const char *file_path) {
    auto base_path = getPath(file_path);

    if (!base_path.is_valid()) {
        return std::make_unexpected<void>(std::ERROR_INVALID_FILE_PATH);
    }

    auto &fs = getFs(base_path);

    bool success = fs.file_system->rmdir(file_path);
    if (success)
        return std::make_expected();

    Logger::instance().println("[VFS] Error rm");
    return std::make_unexpected<void>(std::ERROR_UNKNOWN);
}

std::expected<size_t> vfs::read(fd_t fd, uint8_t *buffer, size_t count, size_t offset) {
    if (!handles::has_handle(fd)) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    fd_t inode = handles::get_handle(fd);

    auto &fs = getFs(Path{"/"});

    ssize_t result = fs.file_system->read(inode, buffer, count, offset);

    if (result < 0) {
        Logger::instance().println("[VFS] Error read, result is %X", result);
        return std::make_unexpected<size_t>(std::ERROR_UNKNOWN);
    }
    if (result != count) {
        Logger::instance().println("[VFS] Error read 2, result is %X", result);
        return std::make_unexpected<size_t>((size_t) result);
    }
    return result;
}

std::expected<size_t> vfs::write(fd_t fd, const uint8_t *buffer, size_t count, size_t offset) {
    if (!handles::has_handle(fd)) {
        return std::make_unexpected<size_t>(std::ERROR_INVALID_FILE_DESCRIPTOR);
    }

    fd_t inode = handles::get_handle(fd);

    auto &fs = getFs(Path("/"));

    ssize_t result = fs.file_system->write(inode, buffer, count, offset);

    if (result < 0) {
        Logger::instance().println("[VFS] Error write, result is %X", result);
        return std::make_unexpected<size_t>(std::ERROR_UNKNOWN);
    }

    return result;
}