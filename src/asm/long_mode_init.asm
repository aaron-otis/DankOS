global long_mode_start
extern kernel_main

section .text
bits 64
long_mode_start:
    ; Load 0 into all data segment registers.
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp kernel_main         ; Jump to main function.

    ; Print `OKAY` to screen.
    mov rax, 0x2f592f412f4b2f4f
    mov qword [0xb8000], rax
    hlt
