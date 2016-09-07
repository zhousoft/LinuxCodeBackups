#include "threadPool.h"
//线程工作函数
void *work(void *arg)
{
    while(1)
    {

    }
}

//初始化线程池
int init_pool(thread_pool *pool, unsigned int threads_numbers)
{
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond, NULL);

    pool->isdestroy = 0;

    pool->task_list = (struct task*)malloc(sizeof(struct task));
    pool->tids = (pthread_t *)malloc(sizeof(pthread_t) * MAX_ACTIVE_THREADS);

    if(pool->task_list == NULL || pool->tids == NULL)
    {
        perror("cant allocate memory");
        return 0;
    }

    pool->task_list->next = NULL;
    pool->waiting_tasks = 0;
    pool->active_threads = threads_numbers;

    //创建线程
    for(int i=0; i<threads_numbers; i++)
    {
        if(pthread_create(&((pool->tids)[i]), NULL, work, (void *)pool) !=0)
        {
            perror("create threads error");
            return 0;
        }
    }

    return 1;

}