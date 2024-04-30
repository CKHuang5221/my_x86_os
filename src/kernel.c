#include "kernel.h"
#include <stdint.h> //for uint16_t
#include <stddef.h> //for size_t
#include "idt/idt.h"
#include "io/io.h"

uint16_t* video_mem = 0;
int terminal_col = 0;
int terminal_row = 0;


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

size_t strlen( const char* str ){
    size_t len = 0;

    while( str[len] ){
        len++;
    }

    return len;
}

void print(const char* str){
    int len = strlen(str);
    for(int i = 0; i<len; i++){
        terminal_write_char(str[i], 1);
    }
}

void kernel_main(){
    
    terminal_init();
    print("Welcome!\n");

    //initialize the interrup descriptor table
    idt_init();

}