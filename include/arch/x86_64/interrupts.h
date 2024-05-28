#pragma once

#include "std/extra_std.h"
#include "util/types.h"

inline __attribute__((always_inline)) void enableInterrupts() {
    asm volatile("sti");
}

inline __attribute__((always_inline)) void disableInterrupts() {
    asm volatile("cli");
}

inline __attribute__((always_inline)) void haltCpu() {
    asm volatile("hlt");
}


struct RegistersState {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t int_no, err_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
}__attribute__((packed));
static_assert(AssertSize<RegistersState, 176>());

typedef void (*InterruptHandler)();

void setupInterrupts();
void setInterruptHandler(Byte num, InterruptHandler handler);