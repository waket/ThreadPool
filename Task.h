/**
 * @file Task.h
 * 
 * The worker is an abstract interface to in some way execute a piece of
 * task on behalf of the caller.  The task is simply a boost bound function
 * that can do whatever the caller needs done.
 *
 * This interface does not enforce the exact mechanism that is used to execute
 * the task, which is exactly why it is an interface.  You should use a concrete
 * implementation of the worker that satisfies the task execution discipline
 * that is needed by the caller (for example deferring to another thread for
 * execution).
 */
#pragma once

#include <boost/function.hpp>

class Task
{
public:
   /**
    * A task item that the worker can execute
    *
    * Bind this to a function that does some task to allow it to execute
    */
   typedef boost::function<void()> TaskItem;

protected:
   /**
    * Constructor
    */
   Task() {};
public:
   /**
    * Destructor
    */
   virtual ~Task() {};
private:
   Task(const Task&);
   Task& operator=(const Task&);

public:
   // --- Interface Methods
   /**
    * Starts the worker to accept new task
    *
    * @return true on success, false on failure
    * @note must be thread safe to be called with stop, started, and doTask
    * @note when called concurrently, must run as though it were being run
    *       without the concurrent call, and run safely
    * @note the order that concurrent calls must execute are not defined
    */
   virtual bool start(void)=0;
   /**
    * Stops the worker from accepting new task and stops executing any backlogged task
    *
    * @return true on success, false on failure
    * @note must be thread safe to be called with start, started, and doTask
    * @note when called concurrently, must run as though it were being run
    *       without the concurrent call, and run safely
    * @note the order that concurrent calls must execute are not defined
    * @note if task is currently executing, it must finish to completion before this call
    *       returns.
    * @note must block until the stop is complete
    * @note must support multiple concurrent calls to stop
    */
   virtual bool stop(void)=0;
   /**
    * Determines if the worker is started (can accept new task and/or is executing)
    *
    * @return true if started, false if not
    * @note must be thread safe to be called with start, stop, and doTask
    */
   virtual bool started(void) const=0;
   /**
    * Requests that the passed task be executed
    *
    * @paramt t the task to execute
    * @return true if the task was accepted for execution, false if not
    * @note must be thread safe to be called with start, stop, and started
    * @note true does not guarantee that the task will be executed, simply that
    *       is has been accepted for execution (i.e. may have been enqueued)
    * @note if the object is stop()'ped before the task is executed, it may not
    *       actually be executed
    */
   virtual bool doTask(const TaskItem& t)=0;
};


