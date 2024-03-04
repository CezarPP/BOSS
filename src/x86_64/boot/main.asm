; entry point into OS
global start

section .text
bits 32 ; still in 32 bit mode
start:
  ; print 'OK'
  ; write to video memory directly
  mov dword [0xb8000], 0x2f4b2f4f
  hlt