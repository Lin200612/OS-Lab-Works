#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

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
        Task *minT = p->task;
        for (; p; p = p->next) {
            if (p->task->burst < minT->burst)
                minT = p->task;
        }
        delete(&head, minT);
        run(minT, minT->burst);
    }
}
