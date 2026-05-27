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
    append(&head, t);
}

void schedule() {
    while (head) {
        Task *t = head->task;
        delete(&head, t);
        if (t->burst > QUANTUM) {
            run(t, QUANTUM);
            t->burst -= QUANTUM;
            append(&head, t);
        } else {
            run(t, t->burst);
        }
    }
}
