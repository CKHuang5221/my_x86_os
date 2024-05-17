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

};

int process_load_for_slot(const char* filename, struct process** process, int process_slot);


#endif