## Project 7/cs452-cthreads-library

* Author: Jonathan Porter, Sharon Yang
* Class: CS452 Section #1

## Overview

This program is a user level implementation of a C thread library. This library allows the user to make threads and employ synchronization and concurrency principles to ensure accurate data management. 

## Manifest

A listing of source files and other non-generated files and a brief (one line)
explanation of the purpose of each file.

README.md - This file.

cthreads-test1.c - Test 1.

cthreads-test2.c - Test 2.

cthreads-test3.c - Test 3.

cthreads-test4.c - Test 4.

cthreads-test5.c - Test 5.

cthreads-test6.c - Test 6.

cthreads-test7.c - Test 7.

cthreads-test8.c - Test 8.

cthreads.c - User level thread library.

cthreads.h - Headers and struct definitions for cthreads.c.

Makefile - Tool for automatically building the project.

README.orig - Instructions.


## Building the project

To build the project, cd into the directory containing all the files in the manifest and build the code by running make with root privileges:

$ sudo make

A cthreads executable will be made, along with executable for all the tests. 

## Features and Usage

As mentioned in the overview, this library is an implementation of a thread library for C. A user of this library will be able to create, join, and exit threads and employ synchronization and concurrency via mutex locks and semaphore to prevent race conditions from occurring, making the state of data shared between threads logged correctly.

A thread can be created with a thread id selected but a counter. This counter is then stored in a integer reference passed into create. When a thread is made, the new thread will know the id of the parent thread that made it, and have its context saved. The new thread will also invoke the start routine by calling makecontext.

While the created thread is running, the parent thread can wait on the child thread to finish before continuing through a call to join. Join makes the parent thread wait until the child thread completes execution before continuing. In join, we define wait as making the computer do something else if the child thread is not finished. Exit will notify the parent thread that its finished, and thus will continue once the scheduler reaches the parent thread.

Exit chooses a thread in the ready_queue to work on since the current thread is finished. The exiting thread is marked as exited, not put back into the ready_queue and is disregarded.

Mutex init makes a lock that is unlocked by default. This lock could then be used in cthread_mutex_lock and cthread_mutex_unlock. Lock makes the locking thread have restricted access to the code that is after the call to lock. If a thread calls lock and the mutex is locked, it must wait (have the computer do something else) while the lock is in the locked state. Unlock simply just sets the lock back to zero, unlocked state.

Semaphore init makes a semaphore which has its own thread queue separate from the ready queue, it’s own mutex, and a counter. Post checks if the semaphore queue is not empty, it it’s not empty, move the top thread from the semaphore queue to the ready_queue and swap to the next thread. Otherwise, increment semaphore count. Wait, checks if the semaphore count is less than or equal to zero, and adds the thread that called wait to the semaphore queue, then swaps to another thread. Otherwise, semaphore count is decremented and the lock is released.

## Testing
First, we built threads in part 1, once we completed init(), create_thread(), join(), exit(), and schedule()  we set no_schedule = 1 in most functions and ran the cthreads.test[1-3]. Next, set lock in part 2, we did mutex_init(), lock(), unlock() and ran the cthreads.tests[4-6]. Finally, in part 3, we added semaphores and used cthreads.test[7-8] to test the init(), wait(), and load() functions for it.  

Test 1 can be ran with: 

$ ./cthreads-test1

Test 2 can be ran with:

$ ./cthreads-test2

Test 3 can be ran with:

$ ./cthreads-test3 <number of threads> <iterations>

Test 4 can be ran with:

$ ./cthreads-test4 <number of threads> <iterations>

Test 5 can be ran with:

$ ./cthreads-test5

Test 6 can be ran with:

$ ./cthreads-test6

Test 7 can be ran with:

$ ./cthreads-test7

Test 8 can be ran with:

$ ./cthreads-test8

Our cthreads library makes all eight of these tests exhibit their expected behavior as stated in the handout.


## Known Bugs

The cthreads.test4 4 100000000 takes 10 seconds, but it’s unclear if this is the cost of using locks or inefficiencies in our scheduling logic. We believe it’s the former and thus likely not a bug.

## Reflection and Self Assessment

This program was a good exercise in developing code in stages. We started by implementing more basic functionality of the threads library such as init, create, join, exit, and schedule and later on implemented more complex features of the library such as locks and semaphores. This project gave us some amount of experience as to what developing a practical library in industry is like, where in the beginning we focus on implementing basic functionality of that library, and expand the possibilities as to what that library is capable of doing as we grow confident in the functionality it already implements.

As for implementing each part of the program, there were parts we were able to go through with relative ease, and parts that took some amount of time to work through. In part 1, we focused mainly on understanding on how set, get, swap, and make context work. After quite a bit of upfront studying with the context examples listed on the course home page, we were able to implement part 1 with relative ease. We had some minor bugs in cycling through the ready_queue properly but those bugs exhibited non-volatile behavior and were thus quite easy to fix. The majority of our time debugging was spent in the ladder two parts debugging synchronization and concurrency. 

In part 2,  when we went back to the test ./cthreads-test4 4 100000000 after getting ./cthreads-test6 passed. We sometimes got the expected results, sometimes seemingly were caught by an infinite loop making this bug non-volatile and much difficult to resolve than any bug we faced in part. In debugging this, we focused on trying to recreate the infinite loop and once the infinite loop was made, we needed to work out what loop it was stuck in. After some careful placement of breakpoints, we found out that the program was stuck in a loop where the library cycles through the ready_queue and finds a next thread to work on. However we forgot to consider a case where every thread in the ready_queue was waiting on another thread. We had to code this in as a special case for finding the next thread, where if every thread in the ready_queue is waiting, we simply just continue the current thread. We found this incorrect state of the ready_queue to be present in every infinite loop we ran into for part 2. 

In part 3, when we ran the test8 at the first attempt, we were seemingly held in an infinite loop. Later we found that was caused by missing setting no_schedule = 0 after getting the next thread run. Once that missing piece was added, we got the expected results for test8. 

## Sources Used

Part 1 user context from IBM articles: https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-getcontext-get-user-context

https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-makecontext-modify-user-context

https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-setcontext-restore-user-context

https://www.ibm.com/docs/en/zos/2.3.0?topic=functions-swapcontext-save-restore-user-context

Part 2 lock：https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf

Part 3 semaphores: http://cs.boisestate.edu/~jxiao/cs452/semaphore_implementation.pdf

