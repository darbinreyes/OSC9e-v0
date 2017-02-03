/**

# Programming Problem 4.22.

## Start: From the Book

An interesting way of calculating   is to use a technique known as Monte Carlo,
which involves randomization. This technique works as follows: Suppose you have
a circle inscribed within a square, as shown in Figure 4.18. (Assume that the
radius of this circle is 1.) First, generate a series of random points as simple
(x, y) coordinates. These points must fall within the Cartesian coordinates that
bound the square. Of the total number of random points that are generated, some
will occur within the circle. Next, estimate by performing the following
calculation:   = 4× (number of points in circle) / (total number of points)
Write a multithreaded version of this algorithm that creates a separate thread
to generate a number of random points. The thread will count the number of
points that occur within the circle and store that result in a global variable.
When this thread has exited, the parent thread will calculate and output the
estimated value of  . It is worth experimenting with the number of random points
generated. As a general rule, the greater the number of points, the closer the
approximation to  . In the source-code download for this text, we provide a
sample program that provides a technique for generating random numbers, as well
as determining if the random (x, y) point occurs within the circle. Readers
interested in the details of the Monte Carlo method for esti- mating   should
consult the bibliography at the end of this chapter. In Chapter 5, we modify
this exercise using relevant material from that chapter.

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

// Thread definitions.
#define MAX_SLEEP_TIME 7
#define NUM_WORKER_THREADS 100 // asserts with 300+
static pthread_t      Worker_thread_tid[NUM_WORKER_THREADS];
void * Worker_thread_func(void *param);
static int thread_counter;

// Cleanup state before terminating.
void cleanup_state (void) {
}

// Init. program state.
void init_state(void) {
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
  unsigned long id = 0;
  pthread_t tid;

  thread_counter++;

  tid = pthread_self();

  id = (unsigned long) &tid; // !!! TODO: go fix this in all previous code. i.e. Rm. get_t_num().s

  printf("#%lu: Hello bro.\n", id);

  rand_sleep(id, MAX_SLEEP_TIME, 1);

  printf("#%lu: Goodbye bro.\n", id);

  pthread_exit(0);
}
