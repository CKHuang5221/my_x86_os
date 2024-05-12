#ifndef FAT16_H
#define FAT16_H

#include "file.h"

struct filesystem* fat16_init();
int fat16_resolve(struct disk* disk);
void* fat16_open();

#endif