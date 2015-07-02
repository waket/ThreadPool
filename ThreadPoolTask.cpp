/**
 * @file ThreadPoolTask.cpp
 * @author Wafa ketata
 *
 * implementation of ThreadPoolTask.
 */

#include "ThreadPoolTask.h"
#include <errno.h>
#include <string.h>
#include <iostream>
#include <unistd.h>


using namespace std;

ThreadPoolTask::~ThreadPoolTask() {
    if (!isShutdown && started())
        stop();
}

//Wrapper for start() to run it as thread
void* start_worker(void* arg) {
    ThreadPoolTask* tp = (ThreadPoolTask*) arg;
    tp->startWorker();
    return NULL;
}

bool ThreadPoolTask::start() {
    startWorkersMutex.lock();
    isStarted = true;
    isStopWorker = false;
    for (int i = 0; i < workerCount; i++) {
        pthread_t pth;
        if (pthread_create(&pth, NULL, start_worker, (void*) this) != 0) {
            cout << "\tFailed to create new worker" << endl;
            isStarted = false;
            startWorkersMutex.unlock();
            return false;
        }
    }
    // Lock and wait till all the workers start
    cout << "\tLock and wait for All workers to get started" << endl;
    startWorkersCond.wait(startWorkersMutex.mutexPtr());
    startWorkersMutex.unlock();

    return true;
}

void ThreadPoolTask::startWorker() {
    pthread_t id = pthread_self();
    cout << "\tStart the worker: " << id << endl;

    startWorkersMutex.lock();
    workersSet.insert(id);
    // Check if all the workers have started then unblock startWorkersCond
    if (workersSet.size() == workerCount) {
        cout << "\t" << id << " All workers started wakeup startWorkersCond" << endl;
        startWorkersCond.signal();
    }
    startWorkersMutex.unlock();

    while (true) {
        cout << "\t" << id << " waiting for the release of the lock" << endl;
        tasksMutex.lock();
        cout << "\t" << id << " is locked " <<tasksDeque.size() << " " << isStarted << " " << isStopWorker << endl;
        // Wait if no task to do and the thread pool is not stopped 
        while (tasksDeque.empty() && isStarted && !isStopWorker) {
            // The wait will be released due to availability of a new task 
            // (signal() from doTask()) or due to the stooping of the
            // pool (broadcast() from stop())
            // The Wait will release the unlock of the mutex tasksMutex and lock it
            // back as soon as it is signaled
            cout << "\t" << id << " is waiting" << endl;
            tasksCond.wait(tasksMutex.mutexPtr());
            cout << "\t" << id << " is signaled " << isStarted << " " << isStopWorker << endl;
        }

        // Check if the thread pool is still on
        if (!isStarted) {
            cout << "\t" << id << " exit (thread pool is stopped) " << workersSet.size() << endl;
            tasksMutex.unlock();
            pthread_exit(NULL);
        }

        // Check for dynamic count worker
        if (isStopWorker) {
            assert(workersSet.size() != workerCount);
            workersSet.erase(id);
            tasksCond.signal();
            if (workersSet.size() == workerCount) {
                isStopWorker = false;
                cout << "\t" << id << " is the last exiting worker " << workersSet.size() << " " << isStopWorker << endl;
                // Unblock setWorkerCount()
                stopWorkersCond.signal();
            }
            cout << "\t" << id << " exit (reduce the worker count) " << workersSet.size() << endl;
            tasksMutex.unlock();
            pthread_exit(NULL);
        }

        assert(!tasksDeque.empty());

        // pick a task
        TaskItem task = tasksDeque.front();
        tasksDeque.pop_front();
        cout << "\t" << id << " pop. Size of the task queue is: " << tasksDeque.size() << endl;
        tasksMutex.unlock();
        // do the task
        (task) ();
    }
}

bool ThreadPoolTask::stop() {
    //Lock by mutex before changing the value of isStarted
    tasksMutex.lock();
    isStarted = false;
    tasksMutex.unlock();

    //Lock by mutex for thread safe stop(), wait() and requestStop()
    stopMutex.lock();
    if (isShutdown) {
        stopMutex.unlock();
        return true;
    }
    bool ret = true;
    cout << "\tWakeup all the workers waiting for task" << endl;
    tasksCond.broadcast(); // Sometime calling broadcast() is not enough to wakeup all the worker
    cout << "\t" << tasksDeque.size() << " task will be ignored" << endl;

    for (std::set<pthread_t>::const_iterator it = workersSet.begin() ; it != workersSet.end(); it++) {
        if (pthread_join(*(it), NULL) != 0) {
            cout << "\tFailed to stop the worker " << *(it) << endl;
            ret = false;
        }
        tasksCond.broadcast(); // Keep waking up the workers, because of the lock "tasksMutex"
        cout << "\tStop worker " <<*(it) << " DONE" << endl;
    }
    //Remove all the task from the queue
    tasksDeque.clear();
    //Unlock wait()
    stopCond.signal();
    isShutdown = true;
    stopMutex.unlock();
    return ret;
}

bool ThreadPoolTask::doTask(const TaskItem& task) {
    //Lock by mutex before getting the value of isShutdown
    stopMutex.lock();
    if (isShutdown ) {
        stopMutex.unlock();
        return false;
    }
    
    // TO ALLOW ADDING TASK EVEN IF IT IS NOT STARTED YET
    // Comment the next 4 lines.
    if (!started()) {
        stopMutex.unlock();
        return false;
    }
    stopMutex.unlock();

    //Lock by mutex before queuing the task
    tasksMutex.lock();
    tasksDeque.push_back(task);
    cout << "\tadd. Size of the task queue is: " << tasksDeque.size() << endl;
    // wakeup one worker that is waiting for a task to be available
    tasksCond.signal();
    tasksMutex.unlock();
    return true;
}

bool ThreadPoolTask::started() const {
    return isStarted;
}

bool ThreadPoolTask::wait() {
    //Lock by mutex before getting  the value of isShutdown
    stopMutex.lock();
    if(isShutdown) { //Don't wait. The Thread pool is already stopped
        assert(!isStarted);
        stopMutex.unlock();
        return true;
    }
     //Wait til the thread pool stops all the workers
    stopCond.wait(stopMutex.mutexPtr());
    stopMutex.unlock();
    return true;
}

//Wrapper for stop() to be run as thread
void* stop_workers(void* arg) {
    ThreadPoolTask* tp = (ThreadPoolTask*) arg;
    tp->stop();
    return NULL;
}

bool ThreadPoolTask::requestStop() {
    //Lock by mutex to avoid scheduling successive requestStop
    stopMutex.lock();
    if(isShutdown) { //Nothing to do. The thread pool is already stopped
        assert(!isStarted);
        stopMutex.unlock();
        return true;
    }
    stopMutex.unlock();
    //Stop all workers by creating new thread for asynchronous purpose
    pthread_t pth;
    if (pthread_create(&pth, NULL, stop_workers, (void*) this) != 0) {
        cout << "\tFailed to create thread to stop all the workers" << endl;
        return false;
    }
    return true;
}

bool ThreadPoolTask::setWorkerCount(size_t dynWorkerCount) {
    //Lock by mutex to avoid scheduling successive requestStop
    stopMutex.lock();
    if(isShutdown || dynWorkerCount == workersSet.size()) { //Nothing to do
        assert(!isStarted);
        stopMutex.unlock();
        return true;
    }

    if (dynWorkerCount > workersSet.size()) {
        workerCount = dynWorkerCount;
        startWorkersMutex.lock();
        //Start new worker by creating new thread
        for (unsigned int i = 0; i < dynWorkerCount - workersSet.size(); i++) {
            pthread_t pth;
            if (pthread_create(&pth, NULL, start_worker, (void*) this) != 0) {
                cout << "\tFailed to create new worker" << endl;
                stopMutex.unlock();
                startWorkersMutex.unlock();
                return false;
            }
        }
        startWorkersCond.wait(startWorkersMutex.mutexPtr());
        startWorkersMutex.unlock();
    } else {
        workerCount = dynWorkerCount;
        isStopWorker = true;
        // wakeup a worker to exit
        tasksCond.signal();
        stopWorkersMutex.lock();
        // wait till all unnecessary workers exit
        stopWorkersCond.wait(stopWorkersMutex.mutexPtr());
        stopWorkersMutex.unlock();
    }
    assert(workerCount == workersSet.size());
    stopMutex.unlock();
    return true;
}

