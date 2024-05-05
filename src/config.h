#ifndef CONFIG_H
#define CONFIG_H

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

#define TOTAL_INTERRUPTS 512

//100mb heap size
#define HEAP_SIZE_BYTES 104857600
#define HEAP_BLOCK_SIZE 4096
#define HEAP_ADDRESS 0x01000000
#define HEAP_TABLE_ADDRESS 0x00007E00
#define SECTOR_SIZE 512

#endif