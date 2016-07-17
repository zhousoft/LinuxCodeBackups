#ifndef     _THREADPOOL_H_
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

    struct task  *next;
}; 

typedef  struct thread_pool
{
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    
    struct task *task_list;

     pthread_t *tids;

     unsigned  int waiting_tasks;
     unsigned int active_threads;

     int isdestroy;
}thread_pool;
//初始化线程池
int int_pool(thread_pool *pool,unsigned int thread_numbers);
//添加任务
int add_task(thread_pool *pool. void *(*task)(void *arg),void *arg);
//添加线程
int add_thread(thread_pool *pool, unsigned int addied_threads_numbers);
//移除线程
int remove_thread(thread_pool *pool, unsigned int removed_threads_number);
//销毁线程池
int destroy_pool(thread_pool *pool)
//线程服务函数
void *thread_server(void *arg);




























#endif 
