#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "config.h"
#include "task.h"

struct process
{
    //process id
    uint16_t id;

    char filename[PEACHOS_MAX_PATH];

    //the main process task
    struct task* task;

    //for kernel to track memory allocation in every process
    void* allocations[PEACHOS_MAX_PROGRAM_ALLOCATIONS];

    //physical pointer to the process memory
    void* ptr;

    //physical pointer to the stack memory
    void* stack;

    //the size of the data pointed to by "ptr"
    uint32_t size;

    struct keyboard_buffer
    {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;

};

int process_load_for_slot(const char* filename, struct process** process, int process_slot);
int process_load(const char* filename, struct process** process);
struct process* process_current();
struct process* process_get(int process_id);

int process_switch(struct process* process);
int process_load_switch(const char* filename, struct process** process);

#endif