#include "idt.h"
#include "config.h"
#include "kernel.h"
#include "memory/memory.h"
#include "io/io.h"
#include "task/task.h"
#include "status.h"
#include "task/process.h"

struct idt_desc idt_descriptors[PEACHOS_TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

static ISR80H_COMMAND isr80h_commands[PEACHOS_MAX_ISR80H_COMMANDS];

static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[PEACHOS_TOTAL_INTERRUPTS];

extern void idt_load(struct idtr_desc* ptr);
extern void no_interrupt();
extern void isr80h_wrapper();
extern void* interrupt_pointer_table[PEACHOS_TOTAL_INTERRUPTS];

void no_interrupt_handler(){
    outb(PIC1_COMMAND, PIC_EOI);   //tell pic we are done handling
}

void interrupt_handler(int interrupt, struct interrupt_frame* frame)
{
    kernel_page();
    if (interrupt_callbacks[interrupt] != 0)
    {
        task_current_save_state(frame);
        interrupt_callbacks[interrupt](frame);
    }

    task_page();

    outb(PIC1_COMMAND, PIC_EOI);   //tell pic we are done handling
}

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


void idt_handle_exception()
{
    process_terminate(task_current()->process);
    task_next();
}

void idt_clock()
{
    outb(0x20, 0x20);

    // Switch to the next task
    task_next();
}

void idt_init(){
    memset(idt_descriptors, 0, sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    for(int i = 1; i<PEACHOS_TOTAL_INTERRUPTS; i++){
        idt_set(i, interrupt_pointer_table[i]);
    }

    idt_set(0, idt_no_0);   // set idt_descriptors[interrupt_no] -> idt_no_0
    idt_set(0x80, isr80h_wrapper);

    for (int i = 0; i < 0x20; i++)
    {
        idt_register_interrupt_callback(i, idt_handle_exception);
    }

    idt_register_interrupt_callback(0x20, idt_clock);

    //load interrupt descriptor table
    idt_load(&idtr_descriptor);
}

int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    if (interrupt < 0 || interrupt >= PEACHOS_TOTAL_INTERRUPTS)
    {
        return -EINVARG;
    }

    interrupt_callbacks[interrupt] = interrupt_callback;
    return 0;
}

void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
    if (command_id < 0 || command_id >= PEACHOS_MAX_ISR80H_COMMANDS)
    {
        panic("The command is out of bounds\n");
    }

    if (isr80h_commands[command_id])
    {
        panic("Your attempting to overwrite an existing command\n");
    }

    isr80h_commands[command_id] = command;
}

void* isr80h_handle_command(int command, struct interrupt_frame* frame)
{
    void* result = 0;

    if(command < 0 || command >= PEACHOS_MAX_ISR80H_COMMANDS)
    {
        // Invalid command
        return 0;
    }

    ISR80H_COMMAND command_func = isr80h_commands[command];
    if (!command_func)
    {
        return 0;
    }

    result = command_func(frame);
    return result;
}

void* isr80h_handler(int command, struct interrupt_frame* frame)
{
    void* res = 0;
    kernel_page();
    task_current_save_state(frame);
    res = isr80h_handle_command(command, frame);
    task_page();
    return res;
}
