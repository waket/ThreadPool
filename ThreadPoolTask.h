/**
 * @file ThreadPoolTask.h\
 * 
 * The ThreadPool is an implementation of Task which owns a set of threads and
 * accepts jobs to be enqueued to the thread pool.  Once a thread becomes available,
 * the next piece of task will be executed in a FIFO order.
 *
 * The actual thread that executes the task is not granateed and may change across
 * different copies of an identical job.
 */
#pragma once

#include "Task.h"
#include <pthread.h>
#include <deque>
#include <set>

#define DEFAULT_THREAD_COUNT 4

class Mutex {
public:

    Mutex() {
        pthread_mutex_init(&mutxLock, NULL);
        isLocked = false;
    }
    ~Mutex() {
        // It shall be safe to destroy an initialized mutex that is unlocked.
        // Attempting to destroy a locked mutex results in undefined behavior.
        if (isLocked) {
            unlock();
        }
        pthread_mutex_destroy(&mutxLock);
    }
    void lock() {
        pthread_mutex_lock(&mutxLock);
        isLocked = true;
    }
    void unlock() {
        isLocked = false;
        pthread_mutex_unlock(&mutxLock);
    }
    // Used by wait condition
    pthread_mutex_t* mutexPtr() {
        return &mutxLock;
    }
private:
    pthread_mutex_t mutxLock;
    volatile bool isLocked;
};

class Condition {
public:

    Condition() {
        pthread_cond_init(&condition, NULL);
    }

    ~Condition() {
        pthread_cond_destroy(&condition);
    }
    // The pthread_cond_wait() functions shall block on a condition variable. It
    // shall be called with mutex locked by the calling thread or undefined
    // behavior results.
    void wait(pthread_mutex_t* mutex) {
        pthread_cond_wait(&condition, mutex);
    }
    // The pthread_cond_signal() function shall unblock at least one of the
    // threads that are blocked on the specified condition variable cond (if any
    // threads are blocked on cond).
    void signal() {
        pthread_cond_signal(&condition);
    }
    // The pthread_cond_broadcast() function shall unblock all threads currently
    // blocked on the specified condition variable cond.
    void broadcast() {
        pthread_cond_broadcast(&condition);
    }
private:
    pthread_cond_t condition;
};

class ThreadPoolTask : public Task
{
public:
   /**
    * Constructor
    *
    * @note uses a default thread count
    */
   ThreadPoolTask() {workerCount = DEFAULT_THREAD_COUNT; isStarted = false; isShutdown = false; isStopWorker = false;}
   ThreadPoolTask(size_t c) { workerCount = c; isStarted = false; isShutdown = false; isStopWorker = false;}
   /**
    * Destructor
    */
   virtual ~ThreadPoolTask();
private:
   ThreadPoolTask(const ThreadPoolTask&);
   ThreadPoolTask& operator=(const ThreadPoolTask&);

public:
   // --- Task Implementation
    bool start();
    bool stop();
    bool started() const;
    bool doTask(const TaskItem& t);
   // --- ThreadPoolTask Specific Methods
   /**
    * Waits (block the caller) until the ThreadPool is stopped
    *
    * @return true if a block occurred, false if not
    * @note if !started, can return immediately
    */
   bool wait(void);
   /**
    * Requests an asynchronous stop
    *
    * @return true if requested, false if not
    * @note will not block until stop completes
    * @note you must call wait() at some point to wait for the stop to complete
    */
   bool requestStop(void);
   /**
    * Start worker as thread
    *
    */
   void startWorker();
   /**
    * Dynamic worker count modification
    *
    */
   bool setWorkerCount(size_t d);
private:
   // Count of workers
   size_t workerCount;
   // Tasks queue 
   std::deque<TaskItem> tasksDeque;
   // Set of workers' ids(i.e thread id)
   std::set<pthread_t> workersSet;
   // Mutex between workers (sharing some common data such as tasksDeque,...)
   Mutex tasksMutex;
   // Waiting for task availability or stopping pool Condition
   Condition tasksCond;
   // Mutex between stop() requestStop() and wait()
   Mutex stopMutex;
   // Waiting for stopping pool Condition
   Condition stopCond;
   // Flag for stopping worker and exiting
   bool isStarted;
   // Flag for releasing wait() locked by requestStop()
   bool isShutdown;
   // Flag for stopping some workers (for dynamic worker count)
   bool isStopWorker;
   // Mutex start workers
   Mutex startWorkersMutex;
   // Condition start workers
   Condition startWorkersCond;
   // Mutex stopping worker for dynamic worker count
   Mutex stopWorkersMutex;
   // Condition stopping worker for dynamic worker count
   Condition stopWorkersCond;
};


