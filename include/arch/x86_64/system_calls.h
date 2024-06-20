/*
 * system_calls.h
 *
 *  Created on: 6/15/24.
 *      Author: Cezar PP
 */


#pragma once

#include "util/types.h"

namespace sys_calls {
    inline uint64_t issueSyscall(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
        uint64_t result;
        Logger::instance().println("[SYSCALL] Issuing system call with nr %X, rdi %X, rsi %X, rdx %X",
                                   syscall_number, arg1, arg2, arg3);

        asm volatile (
                "mov %1, %%rax\n"          // System call number in RAX
                "mov %2, %%rdi\n"          // First argument in RDI
                "mov %3, %%rsi\n"          // Second argument in RSI
                "mov %4, %%rdx\n"          // Third argument in RDX
                "int $0x80\n"              // Trigger the interrupt
                "mov %%rax, %0\n"          // Store the result from RAX
                : "=r"(result)             // Output to variable result
                : "r"(syscall_number), "r"(arg1), "r"(arg2), "r"(arg3) // Input
                : "rax", "rdi", "rsi", "rdx" // Clobbered registers
                );
        return result;
    }
}