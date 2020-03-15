#ifndef __IO_HPP
#define __IO_HPP

#include "head.hpp"
#define DEFAULT_THREAD_NUM 100  
/* thread task*/
typedef struct
{
    void *(*function)(void *); //task function
    void *argc;                // function arguments

} threadpool_task_t;

/*manage thread pool*/
struct threadpool_t
{
    pthread_mutex_t lock;           //lock the whole struct
    pthread_mutex_t thread_number;  //lock numbers used for the busy thread
    pthread_cond_t queue_not_full;  //conditional variable for thread queue is not full
    pthread_cond_t queue_not_empty; //conditional variable for thread queue is not empty

    pthread_t *threads;            //store all thread pid. worker thread array
    pthread_t admin_tid;           // thread pid of administrator
    threadpool_task_t *task_queue; //task queue

    /*thread poole information*/
    int min_thread_num;       // the minimum numbers in the thread pool
    int max_thread_num;       // the maximum number int the thread pool
    int live_thread_num;      // number of threads alive in the thread pool
    int busy_thread_num;      // working thread
    int wait_exit_thread_num; // thread is needed to destoryed

    /* task queue*/
    int queue_front; // queue front
    int queue_rear;
    int queue_size;
    int queue_max_size; // queue maximun number

    /* thread pool status */
    int shutdown; // true is closed
};

/* worker thread*/
void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;
    /*detach thread*/
    pthread_detach(pthread_self());
    while (1)
    {
        pthread_mutex_lock(&(pool->lock));
        /*if no task,blocking in queue_not_empty*/
        while ((pool->queue_size == 0) && (!pool->shutdown))
        {
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));
            /*clear thread*/
            if (pool->wait_exit_thread_num > 0)
            {
                pool->wait_exit_thread_num--;
                /*determines whether the number of threads in the thread pool is greater than the minimum number of threads,if so,ends the current thread*/
                if (pool->live_thread_num > pool->min_thread_num)
                {
                    pool->live_thread_num--;
                    //在线程函数中添加pthread_detach函数，使线程退出时自动进行回收。但是这样有一个问题，
                    //就是pthread_kill使用pthread_detach后的id会导致进程崩溃。解决方法是在pool->threads中查找相同的id，之后进程清0操作。
                    for (int i = 0; i < pool->max_thread_num; i++)
                    {
                        if (pthread_equal(pool->threads[i], pthread_self()))
                        {
                            memset(pool->threads + i, 0, sizeof(pthread_t));
                            break;
                        }
                    }
                    //pthread_exit(NULL);
                    pthread_mutex_unlock(&(pool->lock));
                }
            }
        }
        /* thread poll,if thread closed,exit this thread*/
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&(pool->lock));
            //pthread_exit(NULL);
        }

        /*the thread pull out task*/
        task.function = pool->task_queue[pool->queue_front].function;
        task.argc = pool->task_queue[pool->queue_front].argc;
        pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
        pool->queue_size--;

        /**broadcast to add new task and active it*/
        pthread_cond_broadcast(&(pool->queue_not_full));
        pthread_mutex_unlock(&(pool->lock));

        /*perform the task feched before*/
        pthread_mutex_lock(&(pool->thread_number));
        pool->busy_thread_num++;
        pthread_mutex_unlock(&(pool->thread_number));
        (*(task.function))(task.argc);

        /*end of task*/
        pthread_mutex_lock(&(pool->thread_number));
        pool->busy_thread_num--;
        pthread_mutex_unlock(&(pool->thread_number));
    }
    pthread_exit(NULL);
    return NULL;
}

/* add task to thread pool*/
int threadpool_add_task(threadpool_t *pool, void *(*funtion)(void *argc), void *argc)
{
    pthread_mutex_lock(&(pool->lock));
    /*if queue is full,block*/
    while((pool->queue_size == pool->queue_max_size) && (!pool->shutdown))
        pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));

    if(pool->shutdown)
    {
        pthread_mutex_unlock(&(pool->lock));
        return -1;
    }    
    /*clear arguments of worker thread*/
    if(pool->task_queue[pool->queue_rear].argc != NULL)
    {
        free(pool->task_queue[pool->queue_rear].argc);
        pool->task_queue[pool->queue_rear].argc = NULL;        
    }
    /*add task to task queue*/
    pool->task_queue[pool->queue_rear].function = funtion;
    pool->task_queue[pool->queue_rear].argc = argc;
    pool->queue_rear = (pool->queue_rear + 1) % pool->max_thread_num;
    pool->queue_size++;

    /*now queue is not empty,wake up a thread from thread pool*/
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

int is_thread_alive(pthread_t tid) 
{ 
    int kill_rc = pthread_kill(tid, 0); 
    if (kill_rc == ESRCH)
        return false; 
    return true; 
}

/*create thread*/
int thread_create(struct threadpool_t *pool)
{
    pthread_mutex_lock(&(pool->lock));
    int queue_szie = pool->queue_size;
    int max_thread_num = pool->max_thread_num;
    int live_thread_num = pool->live_thread_num;
    pthread_mutex_unlock(&(pool->lock));

    /*create new thread: when the number of suriviving threads are less than  queue_szie and max_thread_num*/
    if((live_thread_num < queue_szie) && (live_thread_num < max_thread_num))
    {
        pthread_mutex_lock(&(pool->lock));
        /*create 100 threads*/
        for(int i = 0, add = 0; i < max_thread_num && add  < DEFAULT_THREAD_NUM; i++)
        {
            if(pool->live_thread_num > pool->max_thread_num)
                break;
            /* if thread pid is invalid or not exist, create it*/
            if(pool->threads[i] == 0 || is_thread_alive(pool->threads[i]))
            {
                pthread_create(&(pool->threads[i]), NULL, threadpool_thread,(void*)pool);
                add++;
                pool->live_thread_num++;
            }
        }
        pthread_mutex_unlock(&(pool->lock));
    }
    return 0;
}

/* destory thread*/
int thread_destory(struct threadpool_t *pool)
{
    pthread_mutex_lock(&(pool->lock));
    int queue_szie = pool->queue_size;
    int min_thread_num = pool->min_thread_num;
    int live_thread_num = pool->live_thread_num;
    pthread_mutex_unlock(&(pool->lock));

    pthread_mutex_lock(&(pool->thread_number));
    int busy_thread_num = pool->busy_thread_num;
    pthread_mutex_unlock(&(pool->thread_number));
    
    /*destroy additional threads*/
	/* live > min 是为了保证线程池最少有min个线程living */
    if((busy_thread_num * 2) < live_thread_num && live_thread_num > min_thread_num)
    {   
        pthread_mutex_lock(&(pool->lock));
        pool->wait_exit_thread_num= DEFAULT_THREAD_NUM;
        pthread_mutex_unlock(&(pool->lock));

        for(int i = 0; i < DEFAULT_THREAD_NUM; i++)
        {
            /*notifies idle thread to commit suicide*/
            pthread_cond_signal(&(pool->queue_not_empty));
        }
    }
    return 0;
}

/*administrator thread*/
void *admin_thread(void *threadpool)
{
    pthread_detach(pthread_self());
    threadpool_t *pool = (threadpool_t*)threadpool;
    while(!pool->shutdown){
        if(thread_create(pool) < 0)
            return NULL;
        if(thread_destory(pool) < 0)
            return NULL;
    }
    return NULL;
}

/*free thread pool*/
int threadpool_free(struct threadpool_t* pool)
{
    if(pool == NULL)
        return -1;
    if(pool->task_queue)
        free(pool->task_queue);
    if(pool->threads)
    {
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_number));
        pthread_mutex_destroy(&(pool->thread_number));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    free(pool);
    pool = NULL;
    return 0;
}

/*create thread pool*/
/*
    min_thread_num: the minimum numbers of thread
    max_thread_num: the maximun numbers of thread
    queue_max_size: the maximun numbers of task
*/
threadpool_t *threadpool_create(int min_thread_num, int max_thread_num, int queue_max_size)
{
    threadpool_t *pool = NULL;
    do
    {
        if ((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL)
        {
            perror("Threadpool_t Malloc Error");
            break;
        }
        pool->min_thread_num = min_thread_num;
        pool->max_thread_num = max_thread_num;
        pool->live_thread_num = 0;
        pool->busy_thread_num = 0;
        pool->wait_exit_thread_num = 0;
        pool->queue_front = 0;
        pool->queue_rear = 0;
        pool->queue_size = 0;
        pool->queue_max_size = queue_max_size;
        pool->shutdown = false;

        /*According to the maximum number of threads,  allocate the space of the worker thread array and clear 0,*/
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_thread_num);
        if (pool->threads == NULL)
        {
            perror("Pthread_t Malloc Error.");
            break;
        }
        memset(pool->threads, 0, sizeof(pthread_t) * max_thread_num);

        /*According to the maximum number of queue,  allocate the space of the work queue and clear 0*/
        pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_max_size);
        if (pool->task_queue == NULL)
        {
            perror("Threadpool_task_t Malloc Error.");
            break;
        }
        memset(pool->task_queue, 0, sizeof(threadpool_t) * queue_max_size);

        /*Initializes mutexes and condition variables*/
        if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
            pthread_mutex_init(&(pool->thread_number), NULL) != 0 ||
            pthread_cond_init(&(pool->queue_not_full), NULL) != 0 ||
            pthread_cond_init(&(pool->queue_not_empty), NULL) != 0)
        {
            perror("Initoalize Lock Error.");
            break;
        }

        /*start min_thread_num thread*/
        for (int i = 0; i < min_thread_num; i++)
        {
            pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
        }
        /*start adminstrator thread*/
        pthread_create(&(pool->admin_tid), NULL, admin_thread, (void *)pool);
        return pool;
        
    } while (1);
    threadpool_free(pool);
    return NULL;
}

/*destory thread pool*/
int threadpool_destory(struct threadpool_t* pool)
{
    if(pool == NULL)
        return -1;
    pool->shutdown = true;
    /*destory admin thread*/
    pthread_join(pool->admin_tid, NULL);

    /*notifies all of threads to end themselves*/
    for(int i = 0; i < pool->live_thread_num; i++)
        pthread_cond_broadcast(&(pool->queue_not_empty));

    for(int i = 0; i < pool->live_thread_num; i++)
        pthread_join(pool->threads[i], NULL);
        
    threadpool_free(pool);
    return 0;
}

#endif // ! __IO_HPP
