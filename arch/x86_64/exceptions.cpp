/*
 * exceptions.cpp
 *
 *  Created on: 3/16/24.
 *      Author: Cezar PP
 */
#include "../../include/arch/x86_64/exceptions.h"
#include "arch/x86_64/logging.h"


void kPanicAt(const char *msg, const char *file, const unsigned int line) {
    disableInterrupts();

    uint64_t rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp, r8, r9, r10, r11, r12, r13,
            r14, r15, cr0, cr2, cr3; //rip, rflags

    Logger::instance().println("I am panicking my friend.");
    Logger::instance().println(msg);
    Logger::instance().println("In file %s, at line %d", file, line);

    asm("mov %%rax, %0;\
		 	mov %%rbx, %1;\
		 	mov %%rcx, %2;\
			mov %%rdx, %3;\
		 	mov %%rdi, %4;\
		 	mov %%rsi, %5;\
		 	mov %%rbp, %6;\
		 	mov %%rsp, %7;\
		 	mov %%r8, %8;\
		 	mov %%r9, %9;\
		 	mov %%r10, %10;\
			mov %%r11, %11;\
			mov %%r12, %12;\
			mov %%r13, %13;\
			mov %%r14, %14;\
			mov %%r15, %15;\
			mov %%cr0, %16;\
			mov %%cr2, %17;\
			mov %%cr3, %18;\
		 	"
            : "=g"(rax),
    "=g"(rbx),
    "=g"(rcx),
    "=g"(rdx),
    "=g"(rdi),
    "=g"(rsi),
    "=g"(rbp),
    "=g"(rsp),
    "=g"(r8),
    "=g"(r9),
    "=g"(r10),
    "=g"(r11),
    "=g"(r12),
    "=g"(r13),
    "=g"(r14),
    "=g"(r15),
    "=g"(cr0),
    "=g"(cr2),
    "=g"(cr3));
/*    printf(stdOut, "--> KERNEL PANIC at file: %s, in line: %d <--\n", file, line);
    printf(stdOut, "--> MSG=%s <--\n", msg);
    printf(stdOut, "--> General register state: <--\n");
    REGDUMP2(stdOut, "RAX", rax, "RBX", rbx);
    REGDUMP2(stdOut, "RCX", rcx, "RDX", rdx);
    REGDUMP2(stdOut, "RSI", rsi, "RSP", rsp);
    REGDUMP2(stdOut, "RDI", rdi, "RBP", rbp);
    NEWLINE(stdOut);
    REGDUMP2(stdOut, "R8 ", r8, "R9 ", r9);
    REGDUMP2(stdOut, "R10", r10, "R11", r11);
    REGDUMP2(stdOut, "R12", r13, "R13", r13);
    REGDUMP2(stdOut, "R14", r14, "R15", r15);
    NEWLINE(stdOut);
    REGDUMPB(stdOut, "CR0", cr0);
    REGDUMPB(stdOut, "CR2", cr2);
    REGDUMPB(stdOut, "CR3", cr3);*/

    haltCpu();
}
