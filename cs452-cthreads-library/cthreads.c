#include <signal.h> /* for signal handling */
#include <sys/time.h> /* for timing */
#include <stdlib.h> /* for exit, malloc, free */
#include <errno.h> /* for errno */
#include <stdio.h> /* for printf */
#include <string.h> /* for memset */
#include <limits.h> /* for INT_MIN */
#include "cthreads.h"

/* an array for all 64 thread control blocks */
static thread_control_block tcbs[MAX_NUM_THREADS];

/* initialization flag, set this to true when cthread_init() is called already. */
int initialized = FALSE;

/* id of the current running thread.
 * current_tid changes in 5 situations:
 * 1. schedule.
 * 2. calling lock finds lock is held.
 * 3. child exit - child wants to wake up parent (or let someone else run).
 * 4. parent join - because parent wants to wait.
 * 5. calling sem_wait finds semaphore not available.
 **/
static cthread_t current_tid;

/* This structure is used to specify when a timer should expire. It contains the following members:
 *
 *	struct timeval it_interval
 *	This is the period between successive timer interrupts. If zero, the alarm will only be sent once.
 *
 *	struct timeval it_value
 *	This is the period between now and the first timer interrupt. If zero, the alarm is disabled.
 */
struct itimerval time_quantum;

/* signal action handler  */
static struct sigaction sig_action;

/* when this flag is one, tells the schedule not to schedule me out */
static int no_schedule = 0;

/* a global variable, we increment this by one every time we create a thread */
cthread_t tid_idx = 0;

/* Part 0: the ready queue, which is a circular queue, kind of... */

/* is queue full?
 * functions not exported to outside should be declared as static. */
static int isFull(struct Queue* queue)
{
    return (queue->size == MAX_NUM_THREADS);
}

/* is queue empty?
 * functions not exported to outside should be declared as static. */
static int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
/* add a tid to the queue - to the back of the queue. */
void cthread_enqueue(struct Queue* queue, int tid)
{
    if (isFull(queue)){
		printf("queue is full, you can't add more...\n");
        return;
	}
    queue->rear = (queue->rear + 1) % MAX_NUM_THREADS;
    queue->tids[queue->rear] = tid;
    queue->size = queue->size + 1;
//    printf("tid %d is enqueued\n", tid);
}
 
/* remove a tid from the queue - from the front of the queue. */
int cthread_dequeue(struct Queue* queue)
{
    if (isEmpty(queue)){
		printf("stop! queue is empty, you can't dequeue anything...\n");
        return INT_MIN;
	}
    int tid = queue->tids[queue->front];
	/* set the array element to -1, i.e., set it to its initial value. */
	queue->tids[queue->front] = -1;
    queue->front = (queue->front + 1) % MAX_NUM_THREADS;
    queue->size = queue->size - 1;
    return tid;
}

/* ready queue. we put thread IDs into this queue so they will be scheduled. 
 * a fancy way to initialize this struct: this is the so called using a designated initializer.
 * note: the way of initializing of an array like this only works if the compiler is gcc. */
struct Queue ready_queue = {.front = 0, .rear = MAX_NUM_THREADS - 1, .size = 0, .tids = {[0 ... (MAX_NUM_THREADS-1)] = -1}};

/* Part 1: threads */

/* a round-robin scheduler, runs every time the timers goes off.
 * the first time when this function is called and switches contexts,
 * it places the main thread into the queue, and pick one child thread to run.
 * functions not exported to outside should be declared as static.
 */
static void cthread_schedule(int sig) {
	//printf("The scheduler is called\n");
if (no_schedule == 1) {
	//printf("Not doing anything\n");
	return;
}
 if (isEmpty(&ready_queue)){
		
        exit(EXIT_SUCCESS);
	}

	//printf("This scheduler is working\n");
	int next = -1;
		thread_control_block * current_thread = NULL;
		thread_control_block * next_thread = NULL;
		int i = 0;
		while ((next == -1 || tcbs[next].waiting != -1) && i < ready_queue.size)  {
			if (i == ready_queue.size) {
				break;
			}
			next = cthread_dequeue(&ready_queue);

			if (tcbs[next].waiting != -1 && tcbs[tcbs[next].waiting].exited == TRUE) {
				tcbs[next].waiting = -1;
			}

			current_thread = &tcbs[current_tid];
			next_thread = &tcbs[next];
			i++;
			if (tcbs[next].waiting != -1 && tcbs[next].exited == FALSE) {
				cthread_enqueue(&ready_queue, next);
			} else {
				cthread_enqueue(&ready_queue, current_tid);
			}
		}
	current_tid = next;
	swapcontext(&(current_thread->ctx), &(next_thread->ctx));
	
	

}

/* initialize this library - functions not exported to outside should be declared as static. */
static int cthread_init() {
	/* we set it_value to determine when we want the first timer interrupt. */
    time_quantum.it_value.tv_sec = 0;
    time_quantum.it_value.tv_usec = QUANTUM;
    /* we set it_interval to determine that we want the timer to go off every 50 milliseconds. */
    time_quantum.it_interval = time_quantum.it_value;
	if ((setitimer(ITIMER_PROF, &time_quantum, NULL)) != 0) {
        printf("oh no, setting a timer can fail?\n");
        return errno;
    }

	sig_action.sa_handler = cthread_schedule;
	sig_action.sa_flags = SA_RESTART;
	/* the following two lines say, when the sa_handler function is in execution, all SIGPROF signals are blocked. Doing so avoids the complicated situation in which cthread_schedule() gets called in a nested way. */
	sigemptyset(&sig_action.sa_mask);
	sigaddset(&sig_action.sa_mask, SIGPROF);
	if ((sigaction(SIGPROF, &sig_action, NULL)) != 0) {
        printf("oh no, installing a signal handler can fail?\n");
        return errno;
    }

	current_tid = 0;
	tid_idx = 0;
	
	getcontext(&(tcbs[0].ctx)) ;
	//initialize parent thread
	tcbs[current_tid].exited = 0;
	tcbs[current_tid].waiting = -1;
	tcbs[current_tid].parent = -1;
	int i;
	for(i = 1; i < ready_queue.size; i++){
		cthread_create(&i, NULL, NULL);

	}


	tid_idx++;
	initialized = TRUE;

	return TRUE;
}

/* starts a new thread in the calling process. 
 * the new thread starts execution by invoking start_routine(); 
 * arg is passed as the sole argument of start_routine().
 * before returning, a successful call to cthread_create() stores 
 * the ID of the new thread in the buffer pointed to by thread;
 * on success, returns 0; otherwise, prints an error message and exits.
 */

int cthread_create(cthread_t *thread, void *(*start_routine) (void *), void *arg) {


	no_schedule = 1;

	thread_control_block *tcb;
	
	if (tid_idx >= 64) {
		exit(-1);
	}

	if (!initialized) {
		if (cthread_init() != TRUE) {
			exit(-1);
		}
	}

	/* add code here so that tcb will point to (the address of) the right tcbs[] element */
	tcb = &tcbs[tid_idx];
	
	
	/* ss_sp stores the starting address of the stack, which in our case, is tcb->stack. */
	tcb->ctx.uc_stack.ss_sp = (void *) tcb->stack;
	tcb->ctx.uc_stack.ss_size = STACK_SIZE;
	getcontext(&(tcb->ctx));

	/* add code here to initialize other fields of tcb, such as waiting, exited, parent... */
	
	tcbs[tid_idx].waiting = -1;
	tcbs[tid_idx].exited = 0;
	tcbs[tid_idx].parent = current_tid;
	*thread = tid_idx;
	tid_idx++;

	makecontext(&tcb->ctx, (void(*)(void))start_routine, 1, arg);
	cthread_enqueue(&ready_queue, tid_idx-1);
	
	no_schedule = 0;
	
	return 0;
}

int checkForDuplicates(int target) {
	int targetCount = 0;
	int i;
	for (i = 0; i < MAX_NUM_THREADS; i++) {
		if (ready_queue.tids[i] == target) {
			targetCount++;
		}
	}
	return targetCount;

}


/* terminates the calling thread and returns a value via retval that
 * (if the thread is joinable) is available to another thread
 * in the same process that calls cthread_join().
 * in this assignment, we do not use retval.
 * note: we always assume child calls cthread_exit() to exit.
 * but the main thread never calls this function.
 */
void cthread_exit(void *retval) {
	no_schedule = 1;
	//printf("setting in exit\n");
	thread_control_block * current_thread = &tcbs[current_tid];
	if (isEmpty(&ready_queue)) {
		current_thread->exited = 1;
		exit(EXIT_SUCCESS);
	}
	int next = cthread_dequeue(&ready_queue);


	thread_control_block * next_thread = &tcbs[next];
		if (next_thread->waiting != -1 && tcbs[next_thread->waiting].exited == TRUE) {
				next_thread->waiting = -1;
			}
	current_tid = next;
	no_schedule = 0;
	current_thread->exited = 1;
	setcontext(&(tcbs[next].ctx));
}

/* waits for the thread specified by thread to terminate. 
 * if that thread has already terminated, then cthread_join() returns immediately. 
 * only threads who have kids are supposed to call join.
 * on success, cthread_join() returns 0; on error, we print an error message and exit.
 * in this assignment, we do not use retval; 
 * plus, in our applications, right now, we do not use the return value of cthread_join().
 */
int cthread_join(cthread_t thread, void **retval) {
	

	if (thread < 0 || thread > MAX_NUM_THREADS) {
		exit(EXIT_FAILURE);
	}

	

	if (tcbs[thread].exited == TRUE || isEmpty(&ready_queue)) {
		return 0;
	}
	no_schedule = 1;


	int next = -1;
		thread_control_block * current_thread = NULL;
		
		int i = 0;
		while ((next == -1 || tcbs[next].waiting != -1) && i < ready_queue.size)  {
			if (i == ready_queue.size) {
				break;
			}
			next = cthread_dequeue(&ready_queue);

			if (tcbs[next].waiting != -1 && tcbs[tcbs[next].waiting].exited == TRUE) {
				tcbs[next].waiting = -1;
			}

			current_thread = &tcbs[current_tid];
		
			i++;
			if (tcbs[next].waiting != -1 && tcbs[next].exited == FALSE) {
				cthread_enqueue(&ready_queue, next);
			} else {
				cthread_enqueue(&ready_queue, current_tid);
			}
		}
	current_tid = next;
	no_schedule = 0;
	current_thread->waiting = thread;
	swapcontext(&(tcbs[0].ctx), &tcbs[next].ctx); 
	return 0;
}

int cycle_thread(thread_control_block * current_thread, thread_control_block * next_thread) {
	
	int next = cthread_dequeue(&ready_queue);
	current_thread = &tcbs[current_tid];
	next_thread = &tcbs[next];
	cthread_enqueue(&ready_queue, current_tid);
	return next;

}

/* Part 2: locks */

/* initialize a mutex. */
int cthread_mutex_init(cthread_mutex_t *mutex) {
	no_schedule = 1;
	mutex = (cthread_mutex_t*)malloc(sizeof(cthread_mutex_t));
	mutex->lock = 0;
	return 0;
}

/* test and set; matches with the book chapter. */
static inline uint xchg(volatile unsigned int *old_ptr, unsigned int new) {
	uint old;
	asm volatile("lock; xchgl %0, %1" :
			"+m" (*old_ptr), "=a" (old) :
			"1" (new) : "cc");
	return old;
}

/* lock a mutex, returns 0 if successful. */
int cthread_mutex_lock(cthread_mutex_t *mutex) {
	while (mutex->lock == 1) {
		no_schedule = 1;
		int next = -1;
		thread_control_block * current_thread = NULL;
		thread_control_block * next_thread = NULL;
		int i = 0;
		while ((next == -1 || tcbs[next].waiting != -1) && i < ready_queue.size)  {
			if (i == ready_queue.size) {
				break;
			}
			next = cthread_dequeue(&ready_queue);

			if (tcbs[next].waiting != -1 && tcbs[tcbs[next].waiting].exited == TRUE) {
				tcbs[next].waiting = -1;
			}

			current_thread = &tcbs[current_tid];
			next_thread = &tcbs[next];
			i++;
			if (tcbs[next].waiting != -1 && tcbs[next].exited == FALSE) {
				cthread_enqueue(&ready_queue, next);
			} else {
				cthread_enqueue(&ready_queue, current_tid);
			}
		}
		no_schedule = 0;
		current_tid = next;
		swapcontext(&(current_thread->ctx), &(next_thread->ctx));
	}
	while (xchg(&(mutex->lock), 1) == 1)
	{	

	}
	//mutex->lock = 1;
	
	return 0;
}

/* unlock a mutex, returns 0 if successful. */
int cthread_mutex_unlock(cthread_mutex_t *mutex) {
	no_schedule = 1;
	while (xchg(&(mutex->lock), 0) == 0)
	{	

	}
	//mutex->lock = 0;
	return 0;
}

/* Part 3: semaphores */

/* initialize a semaphore, no need to return anything. */
void cthread_sem_init(cthread_sem_t *sem, int value) {

	no_schedule = 1;
struct Queue new_queue = {.front = 0, .rear = MAX_NUM_THREADS - 1, .size = 0, .tids = {[0 ... (MAX_NUM_THREADS-1)] = -1}};
	sem->count = value;
	sem->queue = new_queue;
	
	cthread_mutex_init(&(sem->mutex));

	no_schedule = 0;

}

/* lock a semaphore, no need to return anything. */
/* note: in sema_wait, we will change current_tid if we decide to wait;
 * but in sema_post, we should not change current_tid no matter what, because
 * in sema_post we wake up another thread but we don't necessarily need to switch to
 * that thread. */
void cthread_sem_wait(cthread_sem_t *sem) {
	//no_schedule = 1;
		while (xchg(&(sem->mutex.lock), TRUE) == 1) {

	}
	if (sem->count <= 0) {
		
		cthread_enqueue(&(sem->queue), current_tid);
		sem->mutex.lock = 0;
		int next = -1;
		thread_control_block * current_thread = NULL;
		thread_control_block * next_thread = NULL;
		int i = 0;
		while ((next == -1 || tcbs[next].waiting != -1) && i < ready_queue.size)  {
			if (i == ready_queue.size) {
				break;
			}
			next = cthread_dequeue(&ready_queue);

			if (tcbs[next].waiting != -1 && tcbs[tcbs[next].waiting].exited == TRUE) {
				tcbs[next].waiting = -1;
			}

			current_thread = &tcbs[current_tid];
			next_thread = &tcbs[next];
			i++;
			if (tcbs[next].waiting != -1 && tcbs[next].exited == FALSE) {
				cthread_enqueue(&ready_queue, next);
			}
		}
		no_schedule = 0;
		current_tid = next;
		swapcontext(&(current_thread->ctx), &(next_thread->ctx));

	}
	else {
		sem->count--;
		sem->mutex.lock = 0;
		no_schedule = 0;
	}
}

/* unlock a semaphore, no need to return anything. */
void cthread_sem_post(cthread_sem_t *sem) {
	no_schedule = 1;
		while (xchg(&(sem->mutex.lock), TRUE) == 1) {

	}
	if (!isEmpty(&(sem->queue))) {
		cthread_t semnext = cthread_dequeue(&(sem->queue));
		cthread_enqueue(&ready_queue, semnext);

		int next = -1;
		thread_control_block * current_thread = NULL;
		thread_control_block * next_thread = NULL;
		int i = 0;
		while ((next == -1 || tcbs[next].waiting != -1) && i < ready_queue.size)  {
			if (i == ready_queue.size) {
				break;
			}
			next = cthread_dequeue(&ready_queue);

			if (tcbs[next].waiting != -1 && tcbs[tcbs[next].waiting].exited == TRUE) {
				tcbs[next].waiting = -1;
			}

			current_thread = &tcbs[current_tid];
			next_thread = &tcbs[next];
			i++;
			if (tcbs[next].waiting != -1 && tcbs[next].exited == FALSE) {
				cthread_enqueue(&ready_queue, next);
			} else {
				cthread_enqueue(&ready_queue, current_tid);
			}
		}
		current_tid = next;
		no_schedule = 0;
		swapcontext(&(current_thread->ctx), &(next_thread->ctx));
	
	} else {
		sem->count++;
		no_schedule = 0;
	}
	sem->mutex.lock = 0;
		
}

/* vim: set ts=4: */
