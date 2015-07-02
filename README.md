ThreadPool in C++
=================
## Description
This is a simple ThreadPool implementation in C++. 
This implementation provides the following functionalities: 
* Configurable thread count per pool instance.
* Dynamic thread count modification `SetThreadCount(size_t)`
* Two ThreadPool stopping modes: Synchronous, and Asynchronous

A sample code using this ThreadPool has been provided in `main.cpp`.

##Content
* `Makefile`: makefile to compile the library 
* `main.cpp`: sample code using this ThreadPool library
* `Task.h`: Abstract interface of the task that will be executed by thread
* `ThreadPoolTask.cpp` : Implementation of threadPool library
* `ThreadPoolTask.h` : API of threadPool library

##Requirements
* Boost 1.48.0 (for boost::functions)
* GNU Make 3.81 or higher
* G++ 4.7.2


