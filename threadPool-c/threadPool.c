#include "threadPool.h"
//线程工作函数
void *work(void *arg)
{
    thread_pool *pool = (thread_pool *)arg;
    struct task *p;
    while(1)
    {
        pthread_mutex_lock(&pool->lock);

        //如果当前没有任务且线程池未关闭，进入条件变量等待队列
        while(pool->waiting_tasks == 0 && pool->isdestroy == 0)
        {
            pthread_cond_wait(&pool->cond, &pool->lock);
        }
        //当前没有任务,且线程池关闭标志置1，释放互斥锁并退出
        if(pool->waiting_task == 0 && pool->isdestroy == 1)
        {
            pthread_mutex_unlock();
            pthread_exit(NULL);
        }
        //取出任务队列中的任务并执行
        p = pool->task_list->next;
        pool->task_list->next = p->next;
        pool->waiting_tasks--;

        //释放互斥锁，其他线程可以访问任务队列
        pthread_mutex_unlock(&pool->lock);

        //执行任务
        (p->task)(p->arg);

        //释放完后任务空间
        free(p);
        
    }
    pthread_exit(NULL);
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

//向线程池任务队列中添加任务
int add_task(thread_pool *pool, void *(*task)(void *arg), void *arg)
{
    struct task *new_task = (struct task *)malloc(sizeof(struct task));
    if(new_task == NULL)
    {
        perror("allocate memory error");
        return 0;
    }
    new_task->task = task;
    new_task->arg = arg;
    new_task->nex = NULL;

    //访问任务队列前加锁
    pthread_mutex_lock(&pool->lock);
    if(pool->waiting_task >= MAX_WAITING_TASKS)
    {
        //最大任务数已达到，不能添加新任务
        pthread_mutex_unlock(&pool->lock);
        fprintf(stderr, "too many tasks.\n");
        free(new_task);
        return 0;
    }

    struct task * temp = pool->task_list;
    while(temp->next != NULL)
        temp = temp->next;

    temp->next = new_task;
    pool_waiting_tasks++;

    //添加新任务后释放互斥锁，并唤醒一个条件变量等待队列中的线程
    pthread_mutex_unlock(&pool->lock);
    pthread_cond_signal(&pool->cond);

    return 1;
}

//向线程池中添加线程，返回实际添加数，出错返回-1
int add_thread(thread_pool *pool, unsigned int adding_threads_number)
{
    if(adding_threads_number = 0)
        return 0;

    unsigned int total_threads_number = pool->active_threads + adding_threads_number;
    unsigned int actual_add_number = 0; //实际添加的线程数
    for(int i=pool->active_threads; i<total_threads_number && i<MAX_ACTIVE_THREADS; i++)
    {
        if(pthread_create(&((pool->tids)[i]), NULL, work, (void *)pool) !=0)
        {
            perror("add threads error");
            if(actual_add_number == 0)
                return -1;
            break;
        }
        actual_add_number++;

    }

    pool->active_threads += actual_add_number;
    return actual_add_number;   
}

//从线程池中移除线程
int remove_thread(thread_pool *pool,unsigned int removing_threads_number)
{
    if(removing_threads_number == 0)
        return 0;
    int remain_threads = pool->active_threads - removing_threads;
    remian_threads = remain_threads > 0 ? remain_threads : 1;
    //循环移除线程
    for(int i = pool_active_threads-1; i>remain_threads-1; i--)
    {
        errno = pthread_cancel(pool->tids[i]);
        if(error != 0)
            break;
    }

    if(i == pool->active_threads-1)
        return -1;
    else
    {
        pool->active_threads = i+1;
        return i+1;
    }
}

//销毁线程池
int destroy_pool(thread_pool *pool)
{
    pool->isdestroy = 1;
    pthread_cond_broadcast(&pool->cond);

    for(int i=0; i<pool->active_threads; i++)
    {
        errno = pthread_join(pool->tids[i],NULL);
        if(errno != 0)
        {
            printf("join tids[%d] error: %s\n",i,strerror(errno));
        }
    }

    free(pool->task_list);
    free(pool->tids);
    free(pool);
    return 1;
}