#include "kernel.h"
#include <stdint.h> //for uint16_t
#include <stddef.h> //for size_t
#include "idt/idt.h"
#include "io/io.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "disk/streamer.h"
#include "fs/file.h"
#include "memory/memory.h"
#include "gdt/gdt.h"
#include "config.h"
#include "task/tss.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "isr80h/isr80h.h"
#include "keyboard/keyboard.h"

uint16_t* video_mem = 0;
int terminal_col = 0;
int terminal_row = 0;


// Function to convert integer to string
char* itoa(int num, char* str, int base) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly, otherwise empty string is printed
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // Handle negative numbers
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}

uint16_t terminal_make_char(char c, char color){
    return (color << 8) | c;    //combine character and color to 16 bits
}

void terminal_put_char(int x, int y, char c, char color){
    video_mem[ (y * VGA_WIDTH) + x ] = terminal_make_char(c, color);
}

void terminal_write_char(char c, char color){
    if( c == '\n'){
        terminal_row = terminal_row + 1; //next line
        terminal_col = 0;
        return;
    }
    
    terminal_put_char(terminal_col, terminal_row, c, color);
    terminal_col = terminal_col + 1 ;

    if(terminal_col >= VGA_WIDTH){  //next line
        terminal_col = 0;
        terminal_row = terminal_row + 1;
    }
}


void terminal_init(){   //clear screen to all blank
    terminal_col = 0;
    terminal_row = 0;
    video_mem = (uint16_t*)(0xB8000);

    for(int y = 0; y < VGA_HEIGHT; y++){
        for(int x = 0; x < VGA_WIDTH; x++){
            terminal_put_char(x, y, ' ', 0 );
        }
    }

}

void print(const char* str){
    int len = strlen(str);
    for(int i = 0; i<len; i++){
        terminal_write_char(str[i], 3);
    }
}

static struct paging_4gb_chunk* kernel_chunk  = 0;

void panic(const char* msg){
    print(msg);
    while(1){}
}

void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_chunk);
}

struct tss tss;
struct gdt gdt_real[PEACHOS_TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[PEACHOS_TOTAL_GDT_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},            // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},              // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},             // User data segment
    {.base = (uint32_t)&tss, .limit=sizeof(tss), .type = 0xE9}      // TSS Segment
};

void kernel_main(){
    
    terminal_init();
    print("Welcome!\n");

    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, PEACHOS_TOTAL_GDT_SEGMENTS);

    // Load the gdt
    gdt_load(gdt_real, sizeof(gdt_real));

    //init the heap
    kheap_init();

    //init filesystem(fat16)
    fs_init();

    //search and init disks
    disk_search_and_init();

    //initialize the interrup descriptor table
    idt_init();

    // Setup the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;    //kernel stack
    tss.ss0 = KERNEL_DATA_SELECTOR;

    // Load the TSS
    tss_load(0x28);

    //1. setup paging
    kernel_chunk =  paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    //2. switch to kernel chunk
    paging_switch(kernel_chunk);
    //3. enable paging
    enable_paging();

    // Register the kernel commands
    isr80h_register_commands();

    // Initialize all the system keyboards
    keyboard_init();

    struct process* process = 0;
    int res = process_load("0:/blank.bin", &process);
    if(res!= PEACHOS_ALL_OK){
        panic("failed to load blank.bin");
    }

    task_run_first_ever_task();

    while(1) {}
}