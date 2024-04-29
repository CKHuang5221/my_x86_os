#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(struct idtr_desc* ptr);

void idt_no_0(){
    print("Divide by zero error\n");
}

void idt_set(int interrupt_no, void* address){  //set interrup for ring3
    struct idt_desc* desc = &idt_descriptors[interrupt_no];
    desc->offset_1 = (uint32_t)address & 0x0000ffff;
    desc->selector = KERNEL_CODE_SELECTOR;  //0x08
    desc->zero = 0x00;  //default = 0
    desc->type_attr = 0xEE;    //low 44bits 1110 for 32bit interrupt gate, high 4bits 1110 for P(1),DPL(11, ring3),Storage Segment(0)
    desc->offset_2 =  ( (uint32_t)address >> 16 );

}

void idt_init(){
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    idt_set(0, idt_no_0);   // set idt_descriptors[interrupt_no] -> idt_no_0

    //load interrupt descriptor table
    idt_load(&idtr_descriptor);
}
