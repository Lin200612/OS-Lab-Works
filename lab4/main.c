#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_THREADS 10
#define QUEUE_SIZE 30

typedef struct {
    void (*function)(void* p);
    void* data;
} task_t;

typedef struct {
    task_t queue[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    sem_t sem;
    int shutdown_flag;
} thread_pool_t;

thread_pool_t pool;
pthread_t threads[MAX_THREADS];
int thread_count = 0;

int enqueue(task_t t) {
    pthread_mutex_lock(&pool.mutex);
    if (pool.count == QUEUE_SIZE) {
        pthread_mutex_unlock(&pool.mutex);
        return 1;
    }
    pool.queue[pool.tail] = t;
    pool.tail = (pool.tail + 1) % QUEUE_SIZE;
    pool.count++;
    pthread_mutex_unlock(&pool.mutex);
    sem_post(&pool.sem);
    return 0;
}

int dequeue(task_t *t) {
    pthread_mutex_lock(&pool.mutex);
    if (pool.count == 0) {
        pthread_mutex_unlock(&pool.mutex);
        return 1;
    }
    *t = pool.queue[pool.head];
    pool.head = (pool.head + 1) % QUEUE_SIZE;
    pool.count--;
    pthread_mutex_unlock(&pool.mutex);
    return 0;
}

void* worker(void* arg) {
    while (1) {
        sem_wait(&pool.sem);

        pthread_mutex_lock(&pool.mutex);
        if (pool.shutdown_flag && pool.count == 0) {
            pthread_mutex_unlock(&pool.mutex);
            break;
        }
        pthread_mutex_unlock(&pool.mutex);

        task_t task;
        if (dequeue(&task) == 0) {
            task.function(task.data);
        }
    }
    return NULL;
}

int pool_init() {/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

#define TRUE 1

// this represents work that has to be 
// completed by a thread in the pool
typedef struct 
{
    void (*function)(void *p);
    void *data;
}
task;

// the work queue
task worktodo;

// the worker bee
pthread_t bee;

// insert a task into the queue
// returns 0 if successful or 1 otherwise, 
int enqueue(task t) 
{
    return 0;
}

// remove a task from the queue
task dequeue() 
{
    return worktodo;
}

// the worker thread in the thread pool
void *worker(void *param)
{
    // execute the task
    execute(worktodo.function, worktodo.data);

    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    worktodo.function = somefunction;
    worktodo.data = p;

    return 0;
}

// initialize the thread pool
void pool_init(void)
{
    pthread_create(&bee,NULL,worker,NULL);
}

// shutdown the thread pool
void pool_shutdown(void)
{
    pthread_join(bee,NULL);
}

    pool.head = 0;
    pool.tail = 0;
    pool.count = 0;
    pool.shutdown_flag = 0;

    if (pthread_mutex_init(&pool.mutex, NULL) != 0)
        return 1;
    if (    0, 0) != 0) {
        pthread_mutex_destroy(&pool.mutex);
        return 1;
    }

    thread_count = MAX_THREADS;
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&threads[i], NULL, worker, NULL) != 0) {
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
                pthread_join(threads[j], NULL);
            }
            pthread_mutex_destroy(&pool.mutex);
            sem_destroy(&pool.sem);
            return 1;
        }
    }
    return 0;
}

int pool_submit(void (*func)(void*), void* arg) {
    task_t task;
    task.function = func;
    task.data = arg;
    if (enqueue(task) != 0)
        return 1;
    sem_post(&pool.sem);
    return 0;
}

void pool_shutdown() {
    pthread_mutex_lock(&pool.mutex);
    pool.shutdown_flag = 1;

    for (int i = 0; i < thread_count; i++) {
        sem_post(&pool.sem);
    }
    pthread_mutex_unlock(&pool.mutex);

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&pool.mutex);
    sem_destroy(&pool.sem);
}

void my_task(void* arg) {
    int id = *(int*)arg;
    printf("Thread %lu выполняет задачу %d\n", (unsigned long)pthread_self(), id);
    sleep(1);
}

int main() {
    int num_tasks = 20;

    if (pool_init() != 0) {
        fprintf(stderr, "Ошибка инициализации пула потоков\n");
        return 1;
    }

    int* tasks = malloc(num_tasks * sizeof(int));
    if (!tasks) {
        pool_shutdown();
        fprintf(stderr, "Ошибка выделения памяти\n");
        return 1;
    }

    for (int i = 0; i < num_tasks; i++) {
        tasks[i] = i;
        if (pool_submit(my_task, &tasks[i]) != 0) {
            fprintf(stderr, "Не удалось отправить задачу %d\n", i);
        }
    }

    // Ждём, чтобы задачи успели выполниться
    sleep(5);

    pool_shutdown();

    free(tasks);
    return 0;
}
