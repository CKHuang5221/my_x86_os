ORG 0
BITS 16

_start: ;create fake BPB for our bootloader so that BIOS wont mess the bootloader
    jmp short start
    nop
times 33 db 0


start:
    jmp 0x7c0:step2 ;set code segment to 0x7c0

my_int0:    ;create our own interrupt
    mov ah, 0eh
    mov al, 'A'
    mov bx, 0x00
    int 0x10
    iret

step2:
    ;Dont rely BIOS set 0x7c0 for us
    cli ;disable  interrups
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax  
    mov sp, 0x7c00
    sti ;enable interrups

    mov word[ss:0x00], my_int0
    mov word[ss:0x02], 0x7c0

    int 0

    mov si, message 
    call print
    jmp $

print:
    mov bx, 0 ; position , where to display

.loop:
    lodsb ; load addr to 'al' where 'si' pointing and increase si
    cmp al, 0; compare al == 0 ?
    je .done    ; jump equal
    call print_char
    jmp .loop
.done:
    ret

print_char:

    mov ah, 0eh
    int 0x10 ; calling BIOS interrupt
    ret

message: db 'Hello World!' , 0 ;use 'db' for char or string

times 510 - ($ - $$) db 0   ; $ means current addr, $$ means segment addr
;dw 0xAA55   ;remember intel use little endian
db 0x55
db 0xAA

