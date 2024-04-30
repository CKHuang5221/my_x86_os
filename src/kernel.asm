[BITS 32]

global _start

extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ;enable A20 line
    in al, 0x92
    or al, 2
    out 0x92, al

    ;Remap the master PIC
    mov al, 00010001b
    out 0x20, al    ;tell master pic

    mov al, 0x20    ;tell master pic interrupt 0x20 is where master isr should start
    out 0x21, al

    mov al, 00000001b
    out 0x21, al
    ;end of remap master pic

    ;enable interrupt   ???
    sti

    call kernel_main
    
    jmp $

    times 512 - ($ - $$) db 0 
