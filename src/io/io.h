#ifndef IO_H
#define IO_H

unsigned char insb(unsigned short port);    //design function to read from port 1 byte each times
unsigned char insw(unsigned short port);    //design function to read from port 2 bytes each times

void outb(unsigned short port, unsigned char val);  // design function to send val to port 1 byte each times
void outw(unsigned short port, unsigned char val);  // design function to send val to port 2 bytes each times
#endif