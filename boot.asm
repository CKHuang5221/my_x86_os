ORG 0
BITS 16

_start: ;create fake BPB for our bootloader so that BIOS wont mess the bootloader
    jmp short start
    nop
times 33 db 0


start:
    jmp 0x7c0:step2 ;set code segment to 0x7c0

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

    mov ah, 2
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov bx, buffer
    int 0x13
    jc error    ; if failed

    mov si, buffer
    call print

    jmp $

error:
    mov si, error_message
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

error_message: db 'failed to load sector'

times 510 - ($ - $$) db 0   ; $ means current addr, $$ means segment addr
;dw 0xAA55   ;remember intel use little endian
db 0x55
db 0xAA


buffer:
