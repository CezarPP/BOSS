/*
 * system_calls.cpp
 *
 *  Created on: 6/19/24.
 *      Author: Cezar PP
 */

#include "fs/vfs.h"
#include "arch/x86_64/system_calls.h"
#include "console/console_printer.h"

namespace sys_calls {
    int64_t expected_to_i64(const std::expected<void> &status) {
        if (status) {
            return 0;
        } else {
            return -status.error();
        }
    }

    int64_t expected_to_i64(const std::expected<size_t> &status) {
        if (status) {
            return *status;
        } else {
            return -status.error();
        }
    }

    /*
     * Calling convention, syscall number through rax
     * Parameters in rdi, rsi, rdx, r10, r8, r9
     */

    void sc_open(SystemCallRegisters *regs) {
        auto file = reinterpret_cast<char *>(regs->rdi);
        auto flags = regs->rsi;

        auto status = vfs::open(file, flags);
        regs->rax = expected_to_i64(status);
    }

    void sc_close(SystemCallRegisters *regs) {
        auto fd = regs->rdi;

        vfs::close(fd);
    }

    void sc_read(SystemCallRegisters *regs) {
        auto fd = regs->rdi;
        auto buffer = reinterpret_cast<uint8_t *>(regs->rsi);
        auto max = regs->rdx;
        auto offset = regs->r10;

        auto status = vfs::read(fd, buffer, max, offset);
        regs->rax = expected_to_i64(status);
    }

    void sc_write(SystemCallRegisters *regs) {
        auto fd = regs->rdi;
        auto buffer = reinterpret_cast<uint8_t *>(regs->rsi);
        auto max = regs->rdx;
        auto offset = regs->r10;

        auto status = vfs::write(fd, buffer, max, offset);
        regs->rax = expected_to_i64(status);
    }

    void sc_pwd(SystemCallRegisters *regs) {
        auto p = vfs::pwd();

        auto buffer = reinterpret_cast<char *>(regs->rdi);
        std::copy(p.begin(), p.end(), buffer);
        buffer[p.size()] = '\0';
    }

    void sc_cwd(SystemCallRegisters *regs) {
        auto p = reinterpret_cast<const char *>(regs->rdi);

        auto status = vfs::cd(p);
        regs->rax = expected_to_i64(status);
    }

    void sc_mkdir(SystemCallRegisters *regs) {
        auto file = reinterpret_cast<char *>(regs->rdi);

        auto status = vfs::mkdir(file);
        regs->rax = expected_to_i64(status);
    }

    void sc_rm(SystemCallRegisters *regs) {
        auto file = reinterpret_cast<char *>(regs->rdi);

        auto status = vfs::rm(file);
        regs->rax = expected_to_i64(status);
    }

    void sc_rmdir(SystemCallRegisters *regs) {
        auto dir = reinterpret_cast<char *>(regs->rdi);

        auto status = vfs::rmDir(dir);
        regs->rax = expected_to_i64(status);
    }

    void sc_ls(SystemCallRegisters *regs) {
        Logger::instance().println("[SYSCALL] In ls syscall...");
        std::vector<vfs::file> contents;
        auto status = vfs::ls(contents);

        Console::instance().println("Listing current directory...");
        for (const auto &it: contents)
            if (it.isFile)
                Console::instance().println("File %s, inum: %X", it.fileName.c_str(), it.inum);
            else
                Console::instance().println("Dir %s", it.fileName.c_str());

        regs->rax = expected_to_i64(status);
    }
}