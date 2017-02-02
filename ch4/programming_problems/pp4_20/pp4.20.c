/**

  Programming Problem 4.20.

**/

#include <pthread.h>
#include <unistd.h> // sleep()
#include <stdio.h> // printf
#include <stdlib.h> // rand()
//#include <semaphore.h> // sem_t
//#include <errno.h> // errno
#include <assert.h>

#include "pid_manager.h"

// Thread definitions.
#define MAX_SLEEP_TIME 7
#define NUM_WORKER_THREADS 150
static pthread_t      Worker_thread_tid[NUM_WORKER_THREADS];
void * Worker_thread_func(void *param);

// Cleanup state before terminating.
void cleanup_state (void) {
}

// Init. program state.
void init_state(void) {
}

int main(void) {
  int i;
  pthread_attr_t attr; /* set of attributes for the thread */

  /* get the default attributes */
  pthread_attr_init(&attr);

  // Init. buffer etc.
  init_state();

  /* Create the P threads */
  for(i = 0; i < NUM_WORKER_THREADS; i++) {
    pthread_create(&Worker_thread_tid[i], &attr, Worker_thread_func, NULL);
  }

  //TODO: cancel threads here.

  /* Now wait for all the threads to exit */
  for(i = 0; i < NUM_WORKER_THREADS; i++) {
    pthread_join(Worker_thread_tid[i], NULL);
  }

  cleanup_state();
  return 0;
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

/**

  Assign a unique integer to each thread in the range of 0 to N-1,

**/
static int get_t_num(void) {
  int i = 0;
  pthread_t tid;

  tid = pthread_self();

  for(i = 0; i < NUM_WORKER_THREADS; i++) {
    if(Worker_thread_tid[i] == tid) {
      return i;
    }
  }

  assert(0);
}

/**
 * The thread will begin control in this function
 */
void *Worker_thread_func(void *param)
{
  int id = -1;//get_t_num();
  int my_pid;

  rand_sleep(id, MAX_SLEEP_TIME, 1);

  my_pid = allocate_pid();

  if(my_pid == -1) {
    printf("#%d: No PID available for me. Goodbye.\n", id);
    pthread_exit(0);
  }

  printf("#%d: Alloc.ed my_pid = %d.\n", id, my_pid);
  rand_sleep(id, MAX_SLEEP_TIME, 1);
  printf("#%d: Releasing my_pid = %d.\n", id, my_pid);
  release_pid(my_pid);

  pthread_exit(0);
}
