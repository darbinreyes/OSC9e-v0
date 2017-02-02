/**

# Programming Problem 5.36


## Start: From the Book

> Programming Exercise 3.20 required you to design a PID manager that allocated a
> unique process identifier to each process. Exercise 4.20 required you to modify
> your solution to Exercise 3.20 by writing a program that created a number of
> threads that requested and released process identifiers. Now modify your
> solution to Exercise 4.20 by ensuring that the data structure used to represent
> the availability of process identifiers is safe from race conditions. Use
> Pthreads mutex locks, described in Section 5.9.4.

## End


**/

#include <pthread.h>
#include <unistd.h> // sleep()
#include <stdio.h> // printf
#include <stdlib.h> // rand()
//#include <semaphore.h> // sem_t
#include <string.h> // strerr()
#include <errno.h> // errno
#include <assert.h>

#include "pid_manager.h"

// Thread definitions.
#define MAX_SLEEP_TIME 7
#define NUM_WORKER_THREADS 500 // asserts with 300+
static pthread_t      Worker_thread_tid[NUM_WORKER_THREADS];
void * Worker_thread_func(void *param);
static int thread_counter;

// Cleanup state before terminating.
void cleanup_state (void) {
  free_map();
}

// Init. program state.
void init_state(void) {
  allocate_map();
  thread_counter = 0;
}

int main(void) {
  int i;
  pthread_attr_t attr; /* set of attributes for the thread */

  init_state();

  /* get the default attributes */
  if(pthread_attr_init(&attr) != 0) {
    printf("%s\n",strerror(errno));
    assert(0);
  }

  /* Create the P threads */
  for(i = 0; i < NUM_WORKER_THREADS; i++) {
    if(pthread_create(&Worker_thread_tid[i], &attr, Worker_thread_func, NULL) != 0) {
      printf("%s\n",strerror(errno));
      assert(0);
    }
  }


  /* Now wait for all the threads to exit */
  for(i = 0; i < NUM_WORKER_THREADS; i++) {
    if(pthread_join(Worker_thread_tid[i], NULL) != 0) {
      printf("%s\n",strerror(errno));
      assert(0);
    }
  }

  cleanup_state();

  printf("Main. peace out.\n");

  return 0;
}

/**

  Sleep for a random amount of time.

**/
void rand_sleep(unsigned long caller_id, int max, char use_rand) {
  int t;

  assert(max > 0);

#ifndef SLEEP_TIME_DISABLED

  // sleep rand. amnt. of time.
  if(use_rand)
    t = rand() % max + 1;
  else
    t = max;

  #ifdef MICRO_SECONDS_TIME_UNITS
    printf("#%lu: sleeping %d usecs.\n", caller_id, t);
    usleep(t);
  #else
    printf("#%lu: sleeping %d secs.\n", caller_id, t);
    sleep(t); // default
  #endif
  printf("#%lu: Done sleeping.\n", caller_id);

#else // sleep calls disabled.

  static char no_sleep;
  if(!no_sleep) {
    no_sleep = 1;
    printf("#%lu: Sleeping disabled.\n", caller_id);
  }

#endif

}

/**
 * The thread will begin control in this function
 */
void *Worker_thread_func(void *param)
{
  unsigned long id = 0; // get_t_num();
  int my_pid;
  pthread_t tid;

  thread_counter++;

  tid = pthread_self();

  id = (unsigned long) &tid; // !!! TODO: go fix this in all previous code. i.e. Rm. get_t_num().s

  my_pid = allocate_pid(id);

  if(my_pid == -1) {
    printf("#%lu: No PID available for me. Goodbye.\n", id);
    goto Done;
  }

  printf("#%lu: Alloc.ed my_pid = %d.\n", id, my_pid);

  rand_sleep(id, MAX_SLEEP_TIME, 1);

  printf("#%lu: Releasing my_pid = %d.\n", id, my_pid);

  release_pid(id, my_pid);

Done:
  printf("#%lu: I'm done.\n", id);

  pthread_exit(0);
}