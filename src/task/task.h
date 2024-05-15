#ifndef TASK_H
#define TASK_H

#include "config.h"
#include "memory/paging/paging.h"

struct registors{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

struct task{
    //page directory of the task
    struct paging_4gb_chunk* page_directory;

    //the registors of task when the task get interrupt
    struct registors registors;

    //double link list
    struct task* next;
    struct task* prev;
};

struct task* task_new();
struct task* task_current();
struct task* task_get_next();
int task_free(struct task* task);

#endif
