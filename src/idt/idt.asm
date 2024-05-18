;load idtr so that processor knows where is idt

section .asm

extern int21h_handler ;define in C
extern no_interrupt_handler ;define in C
extern isr80h_handler   ;define in C

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load:
    push ebp
    mov ebp, esp
    mov ebx, [ebp+8]
    lidt[ebx]
    pop ebp
    ret

int21h:
    cli
    pushad
    call int21h_handler
    popad
    sti
    iret

no_interrupt:
    cli
    pushad
    call no_interrupt_handler
    popad
    sti
    iret

isr80h_wrapper:
    ; INTERRUPT FRAME START
    ; ALREADY "AUTO" PUSHED TO US BY THE PROCESSOR UPON ENTRY TO THIS INTERRUPT
    ; uint32_t ip
    ; uint32_t cs;
    ; uint32_t flags
    ; uint32_t sp;
    ; uint32_t ss;

    pushad ; Pushes the general purpose registers "manually" to the stack  <--|   
    ;                                                                         |
    ; INTERRUPT FRAME END                                                     |
    ;                                                                         |
    ; Push the stack pointer so that we are pointing to the interrupt frame __|
    push esp

    push eax ; EAX holds our command lets push it to the stack for isr80h_handler

    call isr80h_handler
    mov dword[tmp_res], eax ;store return value incase we messup eax later
    add esp, 8  ;change esp to point to INTERRUPT FRAME START since we push esp, push eax before

; Restore general purpose registers for user land
    popad
    mov eax, dword[tmp_res]
    iretd

section .data
; Inside here is stored the return result from isr80h_handler
tmp_res: dd 0