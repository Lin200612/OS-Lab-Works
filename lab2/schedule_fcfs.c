#include <stddef.h>    // NULL
#include <stdlib.h>    // malloc, exit
#include <string.h>    // strdup
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
    append(&head, t);
}

void schedule() {
    while (head) {
        Task *t = head->task;
        delete(&head, t);
        run(t, t->burst);
    }
}