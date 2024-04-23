ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

_start: ;create fake BPB for our bootloader so that BIOS wont mess the bootloader
    jmp short start
    nop
times 33 db 0


start:
    jmp 0:step2 ;set code segment to 0x7c0

step2:
    ;Dont rely BIOS set 0x7c0 for us
    cli ;disable  interrups
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax  
    mov sp, 0x7c00
    sti ;enable interrups

.load_protected:
    cli
    lgdt[gdt_descripter]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

;GDT
gdt_start:
gdt_null:   ;64bits of 0
    dd 0x0
    dd 0x0

;offset 0x8 from gdt_start since gdt_null is 8bytes
gdt_code:   ;CS should point to this
    dw 0xffff   ;segment limit first 0~15 bits
    dw 0    ;base first 0~15bits
    db 0    ;base 16~23 bits
    db 0x9a ;access byte
    db 11001111b    ;high 4 bits flags and low 4 bits flag
    db 0    ;base 24~31bits

;offset 0x10
gdt_data:   ; link to DS SS ES FS GS
    dw 0xffff   ;segment limit first 0~15 bits
    dw 0    ;base first 0~15bits
    db 0    ;base 16~23 bits
    db 0x92 ;access byte
    db 11001111b    ;high 4 bits flags and low 4 bits flag
    db 0    ;base 24~31bits
gdt_end:

gdt_descripter:
    dw gdt_end - gdt_start-1
    dd gdt_start

[BITS 32]
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ebp, 0x00200000
    mov esp, ebp
    jmp $

times 510 - ($ - $$) db 0   ; $ means current addr, $$ means segment addr
;dw 0xAA55   ;remember intel use little endian
db 0x55
db 0xAA
