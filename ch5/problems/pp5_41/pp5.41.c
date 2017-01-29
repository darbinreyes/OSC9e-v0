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

**/
#include <pthread.h>
#include <stdio.h> // printf
#include <stdlib.h> // rand()
#include <semaphore.h> // sem_t
#include <errno.h> // errno
#include <assert.h>
#include "barrier.h"

#define NUM_THREADS    7
#define MAX_SLEEP_TIME 13
#define NUM_WORKER_ITERS 2
#define INFINITE_ITERS 1

// Thread definitions.
static pthread_t      Barrier_thread_tid[NUM_THREADS];
void *Worker_thread_func(void *param);

int main(void) {
  int i;
  pthread_attr_t attr; /* set of attributes for the thread */

  init(NUM_THREADS);

  /* get the default attributes */
  pthread_attr_init(&attr);

  /* Create the P threads */
  for(i = 0; i < NUM_THREADS; i++) {
    pthread_create(&Barrier_thread_tid[i], &attr, Worker_thread_func, NULL);
  }

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

/**

  Sleep for a random amount of time.

**/
void rand_sleep(int caller_id, int max, char use_rand) {
  int t;

  assert(caller_id >= -1 && max > 0);

#ifndef SLEEP_TIME_DISABLED

  // sleep rand. amnt. of time.
  if(use_rand)
    t = rand() % max + 1;
  else
    t = max;

  #ifdef MICRO_SECONDS_TIME_UNITS
    printf("#%d: sleeping %d usecs.\n", caller_id, t);
    usleep(t);
  #else
    printf("#%d: sleeping %d secs.\n", caller_id, t);
    sleep(t); // default
  #endif
  printf("#%d: Done sleeping.\n", caller_id);

#else // sleep calls disabled.

  static char no_sleep;
  if(!no_sleep) {
    no_sleep = 1;
    printf("#%d: Sleeping disabled.\n", caller_id);
  }

#endif

}

void *Worker_thread_func(void *param) {
  int id = get_t_num(NUM_THREADS);
  int iters = NUM_WORKER_ITERS;


  do {
    /*
       1. do some work for awhile.
       2. barrier_point();
       3. do some work for awhile.
    */
    printf("#%d: Doing some pre-barrier work. Suk my balls!\n", id);
    //rand_sleep(id, MAX_SLEEP_TIME, 1); // Do something.

    barrier_point(id);

    printf("#%d: Doing post barrier work.\n", id);
    //rand_sleep(id, MAX_SLEEP_TIME, 1); // Do something else.

  } while ((--iters > 0) || INFINITE_ITERS);
}