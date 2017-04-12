section .multiboot_header
header_start:
    dd 0xE85250D6                   ; Magic number.
    dd 0                            ; Intel architecture (4 for MIPS).
    dd header_end - header_start    ; Header length.

    ; Checksum, including stupid hack to avoid compiler warning.
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; Optional multiboot tabs go here:

    ; Required end tags go here:
    dw 0    ; Type.
    dw 0    ; Flags.
    dd 8    ; Size.
header_end:
