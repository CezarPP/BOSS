global start
extern long_mode_start

section .text
bits 32
start:
	mov esp, stack_top ; set up the stack
	push eax ; push the multiboot magic number
	push ebx ; push the pointer to the multiboot structure

	call check_multiboot
	call check_cpuid
	call check_long_mode

	call setup_page_tables
	call enable_paging

	lgdt [gdt64.pointer]
	jmp gdt64.kernel_code:long_mode_start

	hlt

check_multiboot:
	cmp eax, 0x36d76289
	jne .no_multiboot
	ret
.no_multiboot:
	mov al, "M"
	jmp error

check_cpuid:
	pushfd
	pop eax
	mov ecx, eax
	xor eax, 1 << 21
	push eax
	popfd
	pushfd
	pop eax
	push ecx
	popfd
	cmp eax, ecx
	je .no_cpuid
	ret
.no_cpuid:
	mov al, "C"
	jmp error

check_long_mode:
	mov eax, 0x80000000
	cpuid
	cmp eax, 0x80000001
	jb .no_long_mode

	mov eax, 0x80000001
	cpuid
	test edx, 1 << 29
	jz .no_long_mode
	
	ret
.no_long_mode:
	mov al, "L"
	jmp error

; Here we identify map the first 4MB of memory to be sure
; The kernel is loaded at the first MB initially by the bootloader

setup_page_tables:
	mov eax, page_table_l3
	or eax, 0b11 ; present, writable page
	mov [page_table_l4], eax ; first entry in lvl 4 table
	
	mov eax, page_table_l2
	or eax, 0b11 ; present, writable
	mov [page_table_l3], eax

	mov ecx, 0 ; counter
.loop:

	mov eax, 0x200000 ; 2MiB page
	mul ecx
	or eax, 0b10000011 ; present, writable, huge page
	mov [page_table_l2 + ecx * 8], eax ; place in lvl 2 table

	inc ecx ; increment counter
	cmp ecx, 4 ; checks if the whole table is mapped
	           ; only map the first 8 MB using identity paging
	jne .loop ; if not, continue

	ret

enable_paging:
	; pass page table location to cpu
	mov eax, page_table_l4
	mov cr3, eax

	; enable PAE, necessary for 64-bit paging
	mov eax, cr4
	or eax, 1 << 5
	mov cr4, eax

	; enable long mode
	mov ecx, 0xC0000080
	rdmsr ; read model specific register
	or eax, 1 << 8 ; long mode flag
	wrmsr ; write model specific register

	; enable paging
	mov eax, cr0
	or eax, 1 << 31 ; set paging bit
	mov cr0, eax

	ret

error:
	; print "ERR: X" where X is the error code
	mov dword [0xb8000], 0x4f524f45
	mov dword [0xb8004], 0x4f3a4f52
	mov dword [0xb8008], 0x4f204f20
	mov byte  [0xb800a], al
	hlt

; Page tables and stack
; Each of the page tables is 4KB, 4 * 4KB = 16KB total

; Each table entry is 8 bytes long, there are 512 entries => 512 * 8 = 4096 bytes = 4KB
section .bss
align 4096
page_table_l4:
	resb 4096 ; root page table
page_table_l3:
	resb 4096
page_table_l2:
	resb 4096
stack_bottom:
	resb 4096 * 2 ; 8KB stack
stack_top:

section .rodata
gdt64:
	dq 0 ; gdt must begin with zero entry
; Kernel Code Segment Descriptor
.kernel_code: equ $ - gdt64
	dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) ; code segment
; Kernel Data Segment Descriptor
.kernel_data:
    dq (1 << 43) | (1 << 47) | (1 << 53) ; data segment, read/write, ring 0
; User Code Segment Descriptor
.user_code:
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) | (3 << 45) ; code segment, read/execute, non-conforming, ring 3
; User Data Segment Descriptor
.user_data:
    dq (1 << 43) | (1 << 47) | (1 << 53) | (3 << 45) ; data segment, read/write, ring 3
.pointer:
	dw $ - gdt64 - 1 ; length
	dq gdt64 ; address