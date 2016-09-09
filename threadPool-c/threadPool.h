#ifndef    _THREADPOOL_H_
#define    _THREADPOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <pthread.h>

#define MAX_WAITING_TASKS 1000
#define MAX_ACTIVE_THREADS 20

struct task
{
    void *(*task)(void *arg);
    void *arg;

    struct task *next;
}; 

typedef  struct thread_pool
{
    pthread_mutex_t lock;//操作任务队列互斥锁
    pthread_cond_t  cond;//条件变量，同步所有线程
    
    struct task *task_list;//任务队列

    pthread_t *tids;//线程ID

    unsigned  int waiting_tasks;//任务队列中等待任务数
    unsigned int active_threads;//当前活跃线程数

    int isdestroy;//线程池销毁标记
}thread_pool;
//初始化线程池
int int_pool(thread_pool *pool,unsigned int thread_numbers);
//添加任务
int add_task(thread_pool *pool. void *(*task)(void *arg),void *arg);
//添加线程
int add_thread(thread_pool *pool, unsigned int adding_threads_number);
//移除线程
int remove_thread(thread_pool *pool, unsigned int removing_threads_number);
//销毁线程池
int destroy_pool(thread_pool *pool)
//线程工作函数
void *work(void *arg);




























#endif 
