# Overview

In this assignment, we will write a user-level thread library called cadillac-threads, or cthreads. Note this is NOT a kernel project, and you should just develop your code on onyx, not in your virtual machine. Submissions that fail to compile or run on onyx will not be graded.

## Learning Objectives

- Creating a large piece of system software in stages.
- Gaining a deep understanding of user-level thread libraries.
- Understanding how to implement a round robin scheduler.
- Practicing on managing queue data structures.
- Learning how to implement locks.
- Learning how to implement semaphores.

## Book References

- [Concurrency and Threads](http://pages.cs.wisc.edu/~remzi/OSTEP/threads-intro.pdf).
This chapter explains how multiple threads programs look different from single thread programs.

- [Threads API](http://pages.cs.wisc.edu/~remzi/OSTEP/threads-api.pdf).
This chapter describes the APIs provided by the pthread library, your cthreads library will provide very similar APIs, with some slight differences.

- [Locks](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf).
The test-and-set example (figure 28.3) described in this chapter is directly related to this assignment and you should use it.

- [Semaphores](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-sema.pdf).
This chapter explains what semaphores are, how we can use semaphores in concurrent programs, and how semaphores can be implemented.

- [CPU Scheduling](https://pages.cs.wisc.edu/~remzi/OSTEP/cpu-sched.pdf).
This chapter has more explanation about the round robin scheduling policy, as well as the concept of time slicing.

## Background

### user-level thread library vs kernel-level thread library

In previous assignments we used the pthreads library, which allows us to run multiple threads concurrently. The pthreads library is supported by the Linux kernel, and each pthread is mapped into one kernel thread, and the kernel manages to schedule these threads as if each thread is a separate process. This suggests such threads are visible to the kernel, and that's why when you run *ps -eLf*, you can see multiple threads of the same process.

```console
(base) [jidongxiao@onyx ~]$ ps -eLf | grep test-mergesort
jidongx+ 3622074 3618225 3622074 97    7 18:14 pts/1    00:00:35 ./test-mergesort 100000000 2 1234
jidongx+ 3622074 3618225 3622087  0    7 18:14 pts/1    00:00:00 ./test-mergesort 100000000 2 1234
jidongx+ 3622074 3618225 3622088  0    7 18:14 pts/1    00:00:00 ./test-mergesort 100000000 2 1234
jidongx+ 3622074 3618225 3622089  0    7 18:14 pts/1    00:00:00 ./test-mergesort 100000000 2 1234
jidongx+ 3622074 3618225 3622090  0    7 18:14 pts/1    00:00:00 ./test-mergesort 100000000 2 1234
jidongx+ 3622074 3618225 3622091  0    7 18:14 pts/1    00:00:00 ./test-mergesort 100000000 2 1234
jidongx+ 3622074 3618225 3622092  0    7 18:14 pts/1    00:00:00 ./test-mergesort 100000000 2 1234
```

Note here there is only one process, which is launched by this command:

```console
(base) [jidongxiao@onyx p1]$ ./test-mergesort 100000000 2 1234
Serial Sorting 100000000 elements took 26.80 seconds.
Parallel(2 levels) Sorting 100000000 elements took 8.50 seconds.
```

In this assignment, we aim to implement a user-level thread library which does not require that much support from the kernel, and that means our threads are not visible to the kernel, and therefore they will be collectively treated as one process and the kernel will allocate time slices to this one single process. Inside this process, it is our responsibility to allocate time slices to each thread, and switch between our threads, so that every thread of our process will have a chance to run. Such a model determines that we will not be able to take advantage of multiprocessing. However, user-level threads are still expected to be fast, because they require fewer context switches between user mode and kernel mode.

## The Starter Code

The starter code looks like this.

```console
(base) [jidongxiao@onyx cs452-cthreads-library]$ ls
cthreads.c  cthreads-test1.c  cthreads-test3.c  cthreads-test5.c  cthreads-test7.c  Makefile   README.template
cthreads.h  cthreads-test2.c  cthreads-test4.c  cthreads-test6.c  cthreads-test8.c  README.md
```

You will be completing the cthreads.c file. You are not allowed to modify the cthreads.h file.

8 testing programs are provided in the starter code. They are cthreads-test[1-8].c. See their description in the [Testing](#testing) section of this README file.

## Specification

You are required to implement the following functions:

### part 1

```c
int cthread_create(cthread_t *thread, void *(*start_routine) (void *), void *arg);
```

This function creates a new thread. The new thread starts execution by invoking *start_routine*(); *arg* is passed as the sole argument of *start_routine*(). Before returning, a successful call to *cthread_create*() stores the ID of the new thread in the address pointed to by *thread* - *cthread_t* is just an integer type, and thus *thread* is an integer type pointer. The user who uses your library is responsible for allocating memory for the address pointed to by *thread*. It is also the user's responsibility to define the *start_routine*() and pass the correct *arg*.

```c
static int cthread_init();
```

This function is not provided for the user, but you should call it in your *cthread_create*() to initialize your library.

```c
void cthread_exit(void *retval);
```

This function exits the current thread. In this assignment, we do not intend to use the argument *retval*.

```c
int cthread_join(cthread_t thread, void **retval);
```

This function lets the current thread wait for the exit of another thread. In this assignment, we do not intend to use the argument *retval*.

```c
static void cthread_schedule(int sig);
```

This function implements the round robin scheduling. In this assignment, we do not intend to use the argument *sig*.

**Warning**: Move on to part 2 only if part 1 is implemented and you have passed *cthreads-test[1-3]*.

### part 2

```c
int cthread_mutex_init(cthread_mutex_t *mutex);
int cthread_mutex_lock(cthread_mutex_t *mutex);
int cthread_mutex_unlock(cthread_mutex_t *mutex);
```

The user of your library calls these 3 functions to initialize a lock, grab a lock, release a lock, respectively. Read the chapter [Locks](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf) to have a better understanding of how these functions should be implemented.

**Warning**: Move on to part 3 only if part 2 is implemented and you have passed *cthreads-test[4-6]*.

### part 3

```c
void cthread_sem_init(cthread_sem_t *sem, int value);
void cthread_sem_wait(cthread_sem_t *sem);
void cthread_sem_post(cthread_sem_t *sem);
```

The user of your library calls these 3 functions to initialize a semaphore, lock a semaphore, unlock a semaphore, respectively. Read the chapter [Semaphores](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-sema.pdf) to have a better understanding of how these functions should behave.

## Predefined Data Structures and Global Variables

### data structures for thread control blocks

A global struct data structure is defined in cthreads.h:

```c
typedef struct {
        int exited;     // is this thread exited?
        int waiting;    // who are you (as a parent) waiting for. your waiting would be -1 if you're not waiting for anyone.
        cthread_t parent;       // parent thread id
        ucontext_t ctx;  // user thread context
        char stack[STACK_SIZE]; // each thread has its own private stack
} thread_control_block;
```

We use each instance of *thread_control_block* to represent one thread. A global array which contains 64 elements is defined to represent all the 64 thread control blocks.

```c
static thread_control_block tcbs[MAX_NUM_THREADS];
```

Here *MAX_NUM_THREADS* is defined as 64 (in cthreads.h), and in this assignment we assume users will run no more than 64 threads.

Among all the fields in *thread_control_block*, *ucontext_t ctx* is the field which enables you to switch the context from one thread to another thread. Read the [APIs](#apis) section of this README for more details.

### data structures to track active thread IDs

A global queue named *ready_queue* is defined as well as initialized like this:

```c
struct Queue ready_queue = {.front = 0, .rear = MAX_NUM_THREADS - 1, .size = 0, .tids = {[0 ... (MAX_NUM_THREADS-1)] = -1}};
```

This queue stores thread IDs for all ready threads. The queue itself is an instance of *struct Queue*, which is defined as following in *cthread.h*:

```c
/* a structure to represent a queue */
struct Queue {
    /* front and rear are both indexes, indexes of the array. */
    int front, rear, size;
    /* we use this queue to store tids. */
    int tids[MAX_NUM_THREADS];
};
```

More specifically, the array *tids[]* stores tids. For the *ready_queue*, the above initialization code shows all of its 64 elements in *tids*[] are initialized to -1.

Note that we can not take advantage of multiple processors, thus at any given moment, only one of our threads will be running, all the other active threads will be in a ready state, i.e., their IDs will be stored in this *ready_queue*. Or, in the semaphore case, their IDs may be stored in a semaphore's waiting queue.

The above approach to initialize a struct is known as using a **designated initializer**. You are recommended to use this same approach to initialize the queue of your semaphore.

### data structures for your locks

We define *cthread_mutex_t* in cthreads.h.

```c
typedef struct {
        unsigned int lock;
} cthread_mutex_t;
```

It only has one field - *lock*. You can use it like this: when *lock* is 1, it means the lock is held; when *lock* is 0, it means the lock is available.

### data structures for your semaphores

We define *cthread_sem_t* in cthreads.h.

```c
typedef struct {
    /* this can be negative, and when it's negative, 
     * it is equal to the number of waiting threads.
     * */
    int count;
    cthread_mutex_t mutex;
    /* if you can't get the semaphore, then put yourself in this waiting queue. */
    struct Queue queue;
} cthread_sem_t;
```

Here the field *mutex* is used to protect the semaphore itself, in other words, when you access *count* or *queue*, you want to use *mutex* to protect them.

### global variables

```c
int initialized = FALSE;
```

You can use this global variable to track if *cthread_init*() is called already or not. *cthread_init*() should only be called once, for each application.

```c
static cthread_t current_tid;
```

You should initialize *current_tid* to the tid of the main thread. Anytime a context switch is about to occur, we update this variable to store the tid of the thread that is chosen to run.

```c
/* a global variable, we increment this by one every time we create a thread */
cthread_t tid_idx = 0;
```

As the comment suggests, we use this global variable to track how many threads have been created. In this assignment, you never need to decrement this variable.

```c
/* when this flag is one, tells the schedule not to schedule me out */
static int no_schedule = 0;
```

You are recommended to use this variable as a flag. Your *cthread_schedule*() just returns when this flag is 1. You are recommended to set this flag to 1 at the beginning of pretty much every function you are asked to implement - except *cthread_init*(). Question: then when to set this flag to 0?

### other global variables

There are also a few other global variables which will be described later in this README, in the [APIs](#apis) section.

## Provided Helper Functions

```c
void cthread_enqueue(struct Queue* queue, int tid);
int cthread_dequeue(struct Queue* queue);
```

As their names suggest, these two functions allow you to enqueue a tid to the tail of the *queue* and dequeue a tid from the head of the *queue*, respectively. You can use these two functions to manipulate the global variable *ready_queue*, and you can also use these two functions to manipulate your semaphores' waiting queue. Keep in mind when using *cthread_dequeue*(), it may return an invalid tid to you - when the queue is empty and you still attempt to dequeue - that may suggest that you have a bug in your code.

Also note that *cthread_dequeue*() has the following lines:

```c
    int tid = queue->tids[queue->front];
    /* set the array element to -1, i.e., set it to its initial value. */
    queue->tids[queue->front] = -1;
    queue->front = (queue->front + 1) % MAX_NUM_THREADS;
```

which means it returns the tid at the front of the queue, and then sets the corresponding element in the array to -1, which marks that this element is no longer storing a valid tid.

```c
static int isEmpty(struct Queue* queue);
static int isFull(struct Queue* queue);
```

These two functions tell you if the queue is empty, or if the queue is full.

```c
static inline uint xchg(volatile unsigned int *old_ptr, unsigned int new);
```

This helper function provides atomic test-and-set functionality for you. Read the chapter [Locks](https://pages.cs.wisc.edu/~remzi/OSTEP/threads-locks.pdf) to see why you need this and how you can use it when implementing your locks. You may want to use this function in your *cthread_mutex_lock*() and also in your *cthread_sem_wait*().

## APIs

I used the following APIs.

### user context (aka. ucontext) APIs

```c
int getcontext(ucontext_t *ucp);
int setcontext(const ucontext_t *ucp);
void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);
int swapcontext(ucontext_t *oucp, const ucontext_t *ucp);
```

**getcontext**() saves the current context in the structure pointed to by *ucp*. **setcontext**() restores to the previously saved context - the one pointed to by *ucp*. **makecontext**() modifies a context (pointed to by *ucp*), so that when this context is restored, *func*() will be called. **swapcontext**() saves the current context in the structure pointed to by *oucp*, and then activates the context pointed to by *ucp*.

- in *cthread_init*(), you may want to use **getcontext**() to save the context of the main thread into the address pointed to by *ucp*.
- in *cthread_create*(), you may want to use **getcontext**() to save the context of the newly created thread into the address pointed to by *ucp*, and use **makecontext**() to set up the start routine of this newly created thread.
- in *cthread_join*(), you may want to use **swapcontext**() to save the context of the parent thread, and switch to the context of some other thread - not necessarily the child thread.
- in *cthread_schedule*(), you may want to use **swapcontext**() to save the context of the current thread, and switch to the context of another thread.
- in *cthread_mutex_lock*(), you may want to use **swapcontext**() to save the context of the current thread, and switch to the context of another thread.
- in *cthread_sem_wait*(), you may want to use **swapcontext**() to save the context of the current thread, and switch to the context of another thread.
- in *cthread_exit*(), you may want to use **setcontext**() to switch to the context of another thread. Question: why this time it is **setcontext**(), rather than **swapcontext**()?

Among these functions, **makecontext**() requires you to pass a function pointer. You can do it like this:

```c
int cthread_create(cthread_t *thread, void *(*start_routine) (void *), void *arg) {
	...
	thread_control_block *tcb;
	...
	/* add code here so that tcb will point to (the address of) the right tcbs[] element */
	...
	getcontext(&tcb->ctx);
	/* ss_sp stores the starting address of the stack, which in our case, is tcb->stack. */
	tcb->ctx.uc_stack.ss_sp = (void *) tcb->stack;
	tcb->ctx.uc_stack.ss_size = STACK_SIZE;
	...
	/* add code here to initialize other fields of tcb, such as waiting, exited, parent... */
	...
	makecontext(&tcb->ctx, (void(*)(void))start_routine, 1, arg);
	...
}
```

In the above, do not change the third argument of **makecontext**(), which is "1", which says this *start_routine*() has only 1 argument, which is *arg*.

You do not need to call **makecontext**() in the main thread, and you do not need to set up any stack information for the main thread - it has its own stack by default. However, you still need to initialize its tcb, and call **getcontext**(), and you can do it like this: 

```c
static int cthread_init() {
	...
	thread_control_block *tcb;
	...
	/* add code here so that tcb will point to (the address of) the right tcbs[] element */
	...
	getcontext(&tcb->ctx);
	/* add code here to initialize other fields of tcb, such as waiting, exited, parent... */
	...
}
```

### timer APIs

```c
int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);
```

You will need to use this function so as to set a timer, which goes off every 50 milliseconds, which is why the starter code has this line (in cthreads.h):

```c
#define QUANTUM  50000
```

The timer you should use, which is the first argument of *setitimer*(), which is named as *which*, is *ITIMER_PROF*. To use *setitimer*(), you need to initialize a *struct itimerval* structure and pass its address to *setitimer*() as its second argument. To this end, a global variable is defined as the beginning of cthreads.c:

```c
struct itimerval time_quantum;
```

You can initialize this global variable like this:

```c
    /* we set it_value to determine when we want the first timer interrupt. */
    time_quantum.it_value.tv_sec = 0;
    time_quantum.it_value.tv_usec = QUANTUM;
    /* we set it_interval to determine that we want the timer to go off every 50 milliseconds. */
    time_quantum.it_interval = time_quantum.it_value;
```

Once *time_quantum* is initialized, you can pass its address as the second parameter to *setitimer*() like this:

```c
    if ((setitimer(ITIMER_PROF, &time_quantum, NULL)) != 0) {
        printf("oh no, setting a timer can fail?\n");
        return errno;
    }
```

Once *setitimer*() succeeds, a timer interrupt will trigger every 50 milliseconds. The man page of *setitimer*() says "at each expiration, a SIGPROF signal is generated". In this assignment, you should implement a signal handler to handle this signal. A signal handler is a function which will be called when the signal is generated - the OS will generate the signal, and you just need to tell the OS which function is your signal handler, and the OS will call that signal handler to handle this signal. Apparently, this signal handler is your *cthread_schedule*() function - you want to make a scheduling decision every 50 milliseconds, because that is the foundation of the round robin scheduling.

### signal handling APIs

To set up the signal handler, the following APIs are useful.

```c
int sigemptyset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

To facilitate you use these APIs, a global variable is defined:

```c
struct sigaction sig_action;
```

You can initialize this variable like this:

```c
sig_action.sa_handler = cthread_schedule;
sig_action.sa_flags = SA_RESTART;
/* the following two lines say, when the sa_handler function is in execution, all SIGPROF signals are blocked. Doing so avoids the complicated situation in which cthread_schedule() gets called in a nested way. */
sigemptyset(&sig_action.sa_mask);
sigaddset(&sig_action.sa_mask, SIGPROF);
```

Once it's initialized, you can install the signal handler like this:

```c
    if ((sigaction(SIGPROF, &sig_action, NULL)) != 0) {
        printf("oh no, installing a signal handler can fail?\n");
        return errno;
    }
```

Once *sigaction*() succeeds, every 50 milliseconds, your *cthread_schedule*() will be called.

### other APIs

Anytime in any of the functions you implement in this assignment, you are recommended to call *exit*() like this:

```c
exit(EXIT_FAILURE);
```

This is because "man 3 exit" says "The use of *EXIT_SUCCESS* and *EXIT_FAILURE* is slightly more portable (to non-UNIX environments) than the use of 0 and some nonzero value like 1 or -1." There is also one occasion in which you may want to call *exit*() like this:

```
exit(EXIT_SUCCESS);
```

Think about in which function you want to call this.

## Testing 

8 testing programs are provided in the starter code. They are cthreads-test[1-8].c. Once you run make, you will generate the binary files of these testing programs.

- cthreads-test[1-3] tests threads creation, join, exit, and schedule.
- cthreads-test[4-6] tests threads creation, join, exit, schedule, and locks.
- cthreads-test[7-8] tests threads creation, join, exit, schedule, locks, and semaphores;.

## Expected Results

- When running *cthreads-test1*, you are expected to get:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test1
main: begin
A
B
main: end
```

- When running *cthreads-test2*, you are expected to get the exactly same result as following:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test2
sum is 600
```

Note that *cthread-test2* uses multiple threads to compute the sum of an array, and your testing result must be 600.

- When running *cthreads-test3*, you are expected to get:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test3 4 100
initial balance = 0.000000
final balance = 400.000000
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test3 4 100000000
initial balance = 0.000000
final balance = 177966376.000000
```

In the above, when run "cthreads-test3 4 100" you must get 400 as the result, when run "./cthreads-test3 4 100000000", your result must be lower than 400000000, but does not have to be the same number as the one showed above.

- When running *cthreads-test4*, you are expected to get the exactly same result as following:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test4 4 100
initial balance = 0.000000
final balance = 400.000000
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test4 4 100000000
initial balance = 0.000000
final balance = 400000000.000000
```

- When running *cthreads-test5*, you are expected to get a result similar to this:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test5
thread 1 exiting
thread 2 exiting
thread 5 exiting
thread 6 exiting
thread 8 exiting
thread 9 exiting
thread 12 exiting
thread 13 exiting
thread 15 exiting
thread 16 exiting
thread 19 exiting
thread 20 exiting
thread 22 exiting
thread 23 exiting
thread 26 exiting
thread 27 exiting
thread 29 exiting
thread 30 exiting
thread 32 exiting
thread 3 exiting
thread 4 exiting
thread 7 exiting
thread 10 exiting
thread 14 exiting
thread 17 exiting
thread 18 exiting
thread 21 exiting
thread 24 exiting
thread 28 exiting
thread 31 exiting
thread 11 exiting
thread 25 exiting
main: exiting
```

In the above, the order of which thread exits first, which thread exits next, does not matter. But all 32 threads need to exit before the main thread exits.

- When running *cthreads-test6*, you are expected to get the exactly same result as following:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test6
main: output "foobar" 10 times in a row:
foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar
main: exiting
```
- When running *cthreads-test7*, you are expected to get the exactly same result as following:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test7
main: output "foobar" 10 times in a row:
foobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobarfoobar
main: exiting
```
As you can see, *cthreads-test6* and *cthreads-test7* produce the same result. They are just two different solutions to [Leetcode problem No.1115 - Print FooBar Alternately](https://leetcode.com/problems/print-foobar-alternately/). *cthreads-test6* uses locks. *cthreads-test7* uses semaphores.

- When running *cthreads-test8*, you are expected to get results like this:

```console
(base) [jidongxiao@onyx cthreads]$ ./cthreads-test8
main: building H2O:
HHOHHOHHOHHOHHO
main: exiting
```
In theory, your result might look different than the above result. Refer to [Leetcode problem No.1117 - Building H2O](https://leetcode.com/problems/building-h2o/), and see what other results are also acceptable.

## Submission

Due: 23:59pm, November 3rd, 2022. Late submission will not be accepted/graded.

## Project Layout

All files necessary for compilation and testing need to be submitted, this includes source code files, header files, and Makefile. The structure of the submission folder should be the same as what was given to you.

## Grading Rubric (Undergraduate and Graduate)
Grade: /100

- [ 80 pts] Functional Requirements:
  - [10 pts] threads create, schedule, join, exit work correctly - tested by cthreads-test1.
  - [10 pts] threads create, schedule, join, exit work correctly - tested by cthreads-test2.
  - [10 pts] threads create, schedule, join, exit work correctly - tested by cthreads-test3.
  - [10 pts] threads schedule and lock/unlock work correctly - tested by cthreads-test4.
  - [10 pts] threads schedule and lock/unlock work correctly - tested by cthreads-test5.
  - [10 pts] threads schedule and lock/unlock work correctly - tested by cthreads-test6.
  - [10 pts] threads schedule, lock/unlock, and semaphores work correctly - tested by cthreads-test7.
  - [10 pts] threads schedule, lock/unlock, and semaphores work correctly - tested by cthreads-test8.

- [10 pts] Compiler warnings:
  - Each compiler warning will result in a 3 point deduction.
  - You are not allowed to suppress warnings.
  - You won't get these points if you didn't implement any of the above functional requirements.

- [10 pts] Documentation:
  - README.md file (rename this current README file to README.orig and rename the README.template to README.md.)
  - You are required to fill in every section of the README template, missing 1 section will result in a 2-point deduction.
