/*
 * system_calls.cpp
 *
 *  Created on: 6/19/24.
 *      Author: Cezar PP
 */

#include "fs/vfs.h"
#include "arch/x86_64/system_calls.h"

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

    void sc_open(SystemCallRegisters *regs) {
        auto file = reinterpret_cast<char *>(regs->rbx);
        auto flags = regs->rcx;

        auto status = vfs::open(file, flags);
        regs->rax = expected_to_i64(status);
    }

    void sc_close(SystemCallRegisters *regs) {
        auto fd = regs->rbx;

        vfs::close(fd);
    }

    void sc_read(SystemCallRegisters *regs) {
        auto fd = regs->rbx;
        auto buffer = reinterpret_cast<uint8_t *>(regs->rcx);
        auto max = regs->rdx;
        auto offset = regs->rsi;

        auto status = vfs::read(fd, buffer, max, offset);
        regs->rax = expected_to_i64(status);
    }

    void sc_write(SystemCallRegisters *regs) {
        auto fd = regs->rbx;
        auto buffer = reinterpret_cast<uint8_t *>(regs->rcx);
        auto max = regs->rdx;
        auto offset = regs->rsi;

        auto status = vfs::write(fd, buffer, max, offset);
        regs->rax = expected_to_i64(status);
    }

    void sc_pwd(SystemCallRegisters *regs) {
        auto p = vfs::pwd();

        auto buffer = reinterpret_cast<char *>(regs->rbx);
        std::copy(p.begin(), p.end(), buffer);
        buffer[p.size()] = '\0';
    }

    void sc_cwd(SystemCallRegisters *regs) {
        auto p = reinterpret_cast<const char *>(regs->rbx);

        auto status = vfs::cd(p);
        regs->rax = expected_to_i64(status);
    }

    void sc_mkdir(SystemCallRegisters *regs) {
        auto file = reinterpret_cast<char *>(regs->rbx);

        auto status = vfs::mkdir(file);
        regs->rax = expected_to_i64(status);
    }

    void sc_rm(SystemCallRegisters *regs) {
        auto file = reinterpret_cast<char *>(regs->rbx);

        auto status = vfs::rm(file);
        regs->rax = expected_to_i64(status);
    }

    void sc_rmdir(SystemCallRegisters *regs) {
        auto dir = reinterpret_cast<char *>(regs->rbx);

        auto status = vfs::rmDir(dir);
        regs->rax = expected_to_i64(status);
    }
}