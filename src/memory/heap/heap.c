#include "heap.h"
#include "kernel.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "status.h"

// check if table is set correctly
static int heap_validate_table(void* ptr, void* end, struct heap_table* table){
    int res = 0;

    size_t table_size = (size_t)(end - ptr);
    size_t total_blocks = table_size / HEAP_BLOCK_SIZE;

    if(table->total != total_blocks){   //if table total is not correct
        res = -EINVARG;
        goto out;
    }

out:
    return res;

}

static bool heap_validate_alignment(void* ptr){
    return ((unsigned int)ptr % HEAP_BLOCK_SIZE ) == 0;
}

//init for our heap in kheap.c
int heap_create(struct heap* heap, void* ptr, void* end, struct heap_table* table){
    
    int res = 0;

    if(!heap_validate_alignment(ptr) || !heap_validate_alignment(end)){
        res = -EINVARG;
        goto out;
    }
    memset(heap, 0, sizeof(struct heap));
    heap->saddr = ptr;
    heap->table = table;

    res = heap_validate_table(ptr, end, table);
    if( res < 0 ){
        goto out;
    }

    //init every entries of table
    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);//set every entries of table to free

out:
    return res;
}

//calculate how many size we need to give user
static uint32_t heap_align_value_to_upper(uint32_t val){    
    if( (val % HEAP_BLOCK_SIZE) == 0 ){
        return val;
    }

    val = ( val - (val % HEAP_BLOCK_SIZE)) + HEAP_BLOCK_SIZE;

    return val;
}

static int heap_get_entry_type(HEAP_BLOCK_TABLE_ENTRY entry){
    return entry & 0x0f;    //low 4 bits for entries type
}

int heap_get_start_block(struct heap* heap, uint32_t total_blocks){
    struct heap_table* table = heap->table;
    int bc = 0; //blocks current index
    int bs = -1;    //start blocks

    //traverse heap table
    for(size_t i = 0; i<table->total; i++){ 
        if (heap_get_entry_type(table->entries[i]) != HEAP_BLOCK_TABLE_ENTRY_FREE){
            bc = 0;
            bs = -1;
            continue;
        }

        //once we find first free entries
        if(bs == -1){
            bs = i;
        }
        bc++;
        if(bc == total_blocks){
            break;
        }
    }

    //if no free memory to malloc
    if(bs == -1){
        return -ENOMEM;
    }

    return bs;  //start which entries we can start to malloc 
}

void* heap_block_to_address(struct heap* heap, int block){
    return  heap->saddr + (block * HEAP_BLOCK_SIZE);
}

void heap_mark_blocks_taken(struct heap* heap, int start_block, int total_blocks){
    int end_block = (start_block + total_blocks) - 1;

    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;

    if(total_blocks > 1){
        entry |= HEAP_BLOCK_HAS_NEXT;
    }

    for(int i = start_block; i <= end_block; i++){
        heap->table->entries[i] = entry;
        entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;

        if(i != end_block-1){   //if it is not the last entry
            entry |= HEAP_BLOCK_HAS_NEXT;
        }

    }

}

void* heap_malloc_blocks(struct heap* heap, uint32_t total_blocks){    //find free blocks for user
    void* address = 0;

    int start_block = heap_get_start_block(heap, total_blocks); // search in heap_table
    if( start_block < 0 ){
        goto out;
    }

    address = heap_block_to_address(heap, start_block);  //convert block number to address

    //mark entries as taken:
    heap_mark_blocks_taken(heap, start_block, total_blocks);

out:
    return address;
}

void heap_mark_blocks_free(struct heap* heap, int starting_blocks){
    struct heap_table* table = heap->table;

    //traverse heap table
    for(int i = starting_blocks; i<(int)table->total; i++){
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;

        //if it is the last entry to free
        if( !(entry & HEAP_BLOCK_HAS_NEXT) ){
            break;
        }
    }
}

int heap_address_to_block(struct heap* heap, void* address){
    return ( (int)(address - heap->saddr) ) / HEAP_BLOCK_SIZE;
}

void* heap_malloc(struct heap* heap, size_t size){
    size_t aligned_size = heap_align_value_to_upper(size);
    uint32_t total_blocks = aligned_size / HEAP_BLOCK_SIZE; //how many free blocks we need to find
    return heap_malloc_blocks(heap, total_blocks);
}

void heap_free(struct heap* heap, void* ptr ){
    heap_mark_blocks_free( heap, heap_address_to_block(heap, ptr) );
}
