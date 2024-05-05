#include "io/io.h"
#include "disk/disk.h"
#include "memory/memory.h"
#include "config.h"
#include "status.h"

//as our primary hard disk
struct disk disk;

int disk_read_sector(int lba, int total, void* buf){
    outb(0x1F6, (lba >> 24) | 0xE0 );
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char)(lba & 0xff) );
    outb(0x1F4, (unsigned char)(lba >> 8) );
    outb(0x1F5, (unsigned char)(lba >> 16) );
    outb(0x1F7, 0x20 );

    unsigned short* ptr = (unsigned short*) buf;    //use shor because we read two bytes each time
    for(int b = 0; b < total; b++){
        char c = insb(0x1F7);

        //wait for the buffer to be ready
        while ( !(c & 0x08) )
        {
            c = insb(0x1F7);
        }

        //read
        for(int i = 0; i < 256; i++){
            *ptr = insw(0x1F0); //insw read two bytes
            ptr++;
        }
        
    }
    return 0;
}


//init disk struct:
void disk_search_and_init(){
    memset(&disk, 0, sizeof(disk));
    disk.type = DISK_TYPE_REAL;
    disk.sector_size = SECTOR_SIZE;
}

//get disk strcuct by index, if we have many disk
struct disk* disk_get(int index){
    if(index != 0){
        return 0;
    }

    return &disk;
}

int disk_read_blocks(struct disk* idisk, unsigned int lba, int total, void* buf){
    if(idisk != &disk){
        return -EIO;
    }

    return disk_read_sector(lba, total, buf);
}

