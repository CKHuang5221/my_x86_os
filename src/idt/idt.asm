;load idtr so that processor knows where is idt

section .asm

global idt_load

idt_load:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    lidt[ebx]
    pop ebp
    ret
