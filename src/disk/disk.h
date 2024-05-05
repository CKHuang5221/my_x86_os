#ifndef DISK_H
#define DISJ_H


typedef unsigned int DISK_TYPE;

#define DISK_TYPE_REAL 0    //repersent a real hard disk


struct disk
{
    DISK_TYPE type;
    int sector_size;
};


void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_blocks(struct disk* idisk, unsigned int lba, int total, void* buf);

#endif