// schedule_priority_rr.c

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

#define QUANTUM 10

static struct node *head = NULL;

void add(char *name, int priority, int burst) {
    Task *t = malloc(sizeof(Task));
    if (!t) exit(1);
    t->name = strdup(name);
    t->tid = 0;
    t->priority = priority;
    t->burst = burst;
    insert(&head, t);
}

void schedule() {
    while (head) {
        struct node *p = head;
        Task *best = p->task;
        for (; p; p = p->next) {
            if (p->task->priority > best->priority)
                best = p->task;
        }
        delete(&head, best);
        if (best->burst > QUANTUM) {
            run(best, QUANTUM);
            best->burst -= QUANTUM;
            insert(&head, best);
        } else {
            run(best, best->burst);
        }
    }
}
