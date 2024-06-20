/*
 * system_calls.h
 *
 *  Created on: 6/15/24.
 *      Author: Cezar PP
 */


#pragma once

#include "util/types.h"
#include "std/array.h"

namespace sys_calls {
    inline uint64_t issueSyscall(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
        uint64_t result;
        Logger::instance().println("[SYSCALL] Issuing system call with nr %X, rdi %X, rsi %X, rdx %X",
                                   syscall_number, arg1, arg2, arg3);

        asm volatile (
                "mov %1, %%rax\n"          // System call number in RAX
                "mov %2, %%rdi\n"          // First argument in RDI
                "mov %3, %%rsi\n"          // Second argument in RSI
                "mov %4, %%rdx\n"          // Third argument in RDX
                "mov %5, %%r10\n"          // Fourth argument in R10
                "int $0x80\n"              // Trigger the interrupt
                "mov %%rax, %0\n"          // Store the result from RAX
                : "=r"(result)             // Output to variable result
                : "r"(syscall_number), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4) // Input
                : "rax", "rdi", "rsi", "rdx", "r10" // Clobbered registers
                );
        return result;
    }

    void sc_pwd(SystemCallRegisters *regs);

    void sc_open(SystemCallRegisters *regs);

    void sc_close(SystemCallRegisters *regs);

    void sc_read(SystemCallRegisters *regs);

    void sc_write(SystemCallRegisters *regs);

    void sc_cwd(SystemCallRegisters *regs);

    void sc_mkdir(SystemCallRegisters *regs);

    void sc_rm(SystemCallRegisters *regs);

    void sc_rmdir(SystemCallRegisters *regs);

    typedef void (*SyscallHandlerType)(SystemCallRegisters *);

    static std::array<SyscallHandlerType, 256> sysCallArray{};

    inline void populateSyscallArray() {
        sysCallArray[0x00] = sc_read;
        sysCallArray[0x01] = sc_write;
        sysCallArray[0x02] = sc_open;
        sysCallArray[0x03] = sc_close;

        sysCallArray[0x4A] = sc_pwd;
        sysCallArray[0x4B] = sc_cwd;
        sysCallArray[0x4E] = sc_mkdir;
        sysCallArray[0x4F] = sc_rmdir;

        sysCallArray[0xAA] = sc_rm;
    }

    inline void doSystemCall(SystemCallRegisters *regState) {
        kAssert(sysCallArray[regState->rax] != nullptr, "[SYSCALL] Unknown system call!");

        sysCallArray[regState->rax](regState);
    }
}