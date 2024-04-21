ORG 0x7c00
BITS 16

start:
    mov si, message 
    call print
    jmp $

print:
    mov bx, 0 

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

