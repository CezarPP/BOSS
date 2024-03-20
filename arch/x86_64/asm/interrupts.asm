
; section .text

; bits 64

; Loads interrupts descriptor table
global idtLoad
extern IDT_ptr

idtLoad:
   lidt [IDT_ptr]
   ret

; Just for testing purposes
global asmInterrupt
asmInterrupt:
    int 10

; ISRs are associated with both hardware and software interrupts directly managed by the CPU
; IRQs specifically refer to hardware interrupt requests from peripheral devices

; Takes a RegistersState parameter
; Declared as extern "C" in interrupts.cpp
; Calls the handler for the specific interrupt
extern isrHandler

; Takes a RegistersState parameter
; Declared as extern "C" in interrupts.cpp
; Calls the handler and also aknowledges the interrupt to the PIC
extern irqHandler

; Default interrupt handler
extern defHandler

; Push all registers onto the stack, no arguments
%macro pushaq 0
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro


; Pop all registers from the stack, no arguments
%macro popaq 0
   	pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro


; Defines an interrupt service routine
; Takes one argument, the interrupt number
%macro ISR_NOERRCODE 1
  global isr%1
  isr%1:
  	cli
  	push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1

  global isr%1
  isr%1:
    cli
    ;interrupt verctor number pushed by cpu
    push byte %1
    jmp isr_common_stub
%endmacro

%macro IRQ 2
  global irq%1
  irq%1:
  	cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

global defaultIRQ

defaultIRQ:
	cli
	call defHandler
	iretq

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47

isr_common_stub:
	pushaq

    mov rdi,rsp
    call isrHandler

    popaq

    add rsp, 16
    sti
    iretq

irq_common_stub:
    pushaq

    mov rdi,rsp
    call irqHandler

    popaq

    add rsp, 16
    sti
    iretq
