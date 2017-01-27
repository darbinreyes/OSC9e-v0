/**

PProblem 5.41

### From the book

A barrier is a tool for synchronizing the activity of a number of threads.
When a thread reaches a barrier point, it cannot proceed until all other
threads have reached this point as well. When the last thread reaches
the barrier point, all threads are released and can resume concurrent
execution.
Assume that the barrier is initialized to N—the number of threads that
must wait at the barrier point:
init(N);
Each thread then performs some work until it reaches the barrier point:

// do some work for awhile //
barrier point();
// do some work for awhile //
Using synchronization tools described in this chapter, construct a barrier
that implements the following API:
• int init(int n)—Initializes the barrier to the specified size.
• int barrier point(void)—Identifies the barrier point. All
threads are released from the barrier when the last thread reaches
this point.
The return value of each function is used to identify error conditions.
Each function will return 0 under normal operation and will return
−1 if an error occurs. A testing harness is provided in the source code
download to test your implementation of the barrier.
#####

### PLAN:

0) Add code to create threads.
1) Add init. and cleanup functions.
2) Implement barrier().


### THOUGHTS:

* When did I use the pthread cond. var? ANS: The DPP solution.
  * Right, the cond. var was used as a mechanism to wait for their state to be set from HUNGRY to EATING.
  The P. would loop on until the while condition being waited on was TRUE.
  * In this problem, I think we can use the same concept, just a diff. condition for the P to wait on.
  That condition is while(barrier_gate_is_closed) IOW. while(in_barrier_count < NUM_THREADS);
  * From pproj2: How to use a pthread cond var. see below.
    * It seems that the pthread cond. var provides a way for use to give up a lock we have already acquired.
    Why give up the lock after you just acquired it? ANS: The lock is serving 2 purposes here:
    1. M.ex. access to a shared variable, i.e. prevents race conditions from fuking shit up.
    2. Via the cond_wait(mx,cnd_v), a P that just got the lock can lend out the lock it has a acquired .
    Why lend out the lock? Why not do your data update and release the lock? ANS: Because in this scenario the data update should only be done after some event has occured.
    When this event occurs, the "condition" has become true, and the P's can proceed tot he next sequence of instructions.

### START PPROJ2 NOTES

Condition variables in Pthreads use the pthread_cond_t data type and
are initialized using the pthread_cond_init() function. The following code
creates and initializes a condition variable as well as its associated mutex lock:

pthread_mutex_t mutex;
pthread_cond_t cond_var;
pthread_mutex_init(&mutex,NULL);
pthread_cond_init(&cond_var,NULL);


The pthread_cond_wait() function is used for waiting on a condition
variable. The following code illustrates how a thread can wait for the condition
a == b to become true using a Pthread condition variable:


pthread_mutex_lock(&mutex);
while (a != b)
  pthread_cond_wait(&mutex, &cond var);
pthread_mutex_unlock(&mutex);


The mutex lock associated with the condition variable must be locked
before the pthread_cond_wait() function is called, since it is used to protect
the data in the conditional clause from a possible race condition. Once this
lock is acquired, the thread can check the condition. If the condition is not true,
the thread then invokes pthread_cond_wait(), passing the mutex lock and
the condition variable as parameters. Calling pthread_cond_wait() releases
the mutex lock, thereby allowing another thread to access the shared data and
possibly update its value so that the condition clause evaluates to true. (To
protect against program errors, it is important to place the conditional clause
within a loop so that the condition is rechecked after being signaled.)
A thread that modifies the shared data can invoke the
pthread_cond_signal() function, thereby signaling one thread waiting
on the condition variable. This is illustrated below:


pthread_mutex_lock(&mutex);
a = b;
pthread_cond_signal(&cond var);
pthread_mutex_unlock(&mutex);


It is important to note that the call to pthread_cond_signal() does not
release the mutex lock. It is the subsequent call to pthread_mutex_unlock()
that releases the mutex. Once the mutex lock is released, the signaled thread
becomes the owner of the mutex lock and returns control from the call to
pthread cond. wait().

### END PPROJ2 NOTES
**/
#include <pthread.h>
#include <stdio.h> // printf
#include <stdlib.h> // rand()
#include <semaphore.h> // sem_t
#include <errno.h> // errno
#include <assert.h>

#define NUM_THREADS 2

// Sync. decls.
static sem_t entry_gate_sem;
static pthread_mutex_t mutex;
static inside_barrier_count; // Counts num. threads inside the barrier walls.
static const barrier_threshold_count = NUM_THREADS; // Num. of threads at which the barrier is released/opened.


// Thread definitions.
static pthread_t      Barrier_thread_tid[NUM_THREADS];
void *Barrier_thread_func(void *param);

// Cleanup state before terminating.
void cleanup_state (void) {

  if(pthread_mutex_destroy(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&entry_gate_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // if (sem_destroy(&user_req_pending_sem) != 0) {
  //   printf("%s\n", strerror(errno));
  //   assert(0);
  // }

}

/**
Assume that the barrier is initialized to N—the number of threads that
must wait at the barrier point:
init(N);
Each thread then performs some work until it reaches the barrier point:

// do some work for awhile //
barrier point();
// do some work for awhile //
Using synchronization tools described in this chapter, construct a barrier
that implements the following API:
• int init(int n)—Initializes the barrier to the specified size.
• int barrier point(void)—Identifies the barrier point. All
threads are released from the barrier when the last thread reaches
this point.
The return value of each function is used to identify error conditions.
Each function will return 0 under normal operation and will return
−1 if an error occurs. A testing harness is provided in the source code
download to test your implementation of the barrier.
**/
int init(int N) { // void init_state(void) {

  assert(N > 0);

  inside_barrier_count = 0;

  // Init. mutex
  if(pthread_mutex_init(&mutex, NULL) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // Create an "unnamed" semaphore, flags = 0, init. value = NUM_B_THREADS.
  if (sem_init(&entry_gate_sem, 0, N) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // if (sem_init(&exit_gate_sem, 0, 1) == -1) {
  //   printf("%s\n", strerror(errno));
  //   assert(0);
  // }

  return 0;
}

int main(void) {
  int i;
  pthread_attr_t attr; /* set of attributes for the thread */

  init(NUM_THREADS);

  /* get the default attributes */
  pthread_attr_init(&attr);



  /* Create the P threads */
  for(i = 0; i < NUM_THREADS; i++) {
    pthread_create(&Barrier_thread_tid[i], &attr, Barrier_thread_func, NULL);
  }

  //TODO: cancel threads here.

  /* Now wait for all the threads to exit */
  for(i = 0; i < NUM_THREADS; i++) {
    pthread_join(Barrier_thread_tid[i], NULL);
  }

  cleanup_state();
}

/**

  Assign a unique integer to each thread in the range of 0 to N-1,

**/
static int get_t_num(int num_t) {
  int i = 0;
  pthread_t tid;

  assert(num_t > 0);

  tid = pthread_self();

  for(i = 0; i < num_t; i++) {
    if(Barrier_thread_tid[i] == tid) {
      return i;
    }
  }

  assert(0);
}

void *Barrier_thread_func(void *param) {
  int id = get_t_num(NUM_THREADS);


  do {

    printf("#%d: Suk my balls!\n", id);

  } while (1);
}