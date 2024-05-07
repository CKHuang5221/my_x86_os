section .asm

global insb
global insw
global outb
global outw

insb:
    push ebp
    mov ebp, esp

    xor eax, eax    ; clear eax to zero
    mov edx, [ebp+8]    ;get port number to edx
    in al, dx   ;read from port(dx) and return value(ax) will in eax by default (and we only return 1 byte since it's insb) 

    pop ebp
    ret

insw:
    push ebp
    mov ebp, esp

    xor eax, eax    ; clear eax to zero
    mov edx, [ebp+8]    ;get port number to edx
    in ax, dx   ;read from port and return value will in eax by default (and we only return 2 byte since it's insw) 

    pop ebp
    ret

outb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+12]
    mov edx, [ebp+8]
    out dx, al

    pop ebp
    ret

outw:
    push ebp
    mov ebp, esp

    mov eax, [ebp+12]
    mov edx, [ebp+8]
    out dx, ax

    pop ebp
    ret

