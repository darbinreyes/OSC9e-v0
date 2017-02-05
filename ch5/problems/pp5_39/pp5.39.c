/**

# Programming Problem 5.39.

## Start: From the Book

> Exercise 4.22 asked you to design a multithreaded program that esti- mated
> using the Monte Carlo technique. In that exercise, you were asked to create a
> single thread that generated random points, storing the result in a global
> variable. Once that thread exited, the parent thread performed the calcuation
> that estimated the value of  . Modify that program so that you create several
> threads, each of which generates random points and determines if the points fall
> within the circle. Each thread will have to update the global count of all
> points that fall within the circle. Protect against race conditions on updates
> to the shared global variable by using mutex locks.

## End


* Plan:
  * 1. Change to use a single buffer to hold the count for inside the unit circle.
  This creates a race condition.
  * Each thread will compute its inside count in a local variable. Once he has
  check each random point for intersection with the unit circle he will update the global
  count shared by all withs by simply incrementing the count with his private count.
  * Since the global count update is a += operation, each thread must read the global count
  first, then increment, and finally write the updated count. This is where the race condition lies.
  * Interleaved updates of the global count will be fucked up unless we use a mutex
  to ensure all updates occur 1 thread at a time.
  * Note that my original solution to pp4.22.c worked around the race condition by
  having each thread update a his own external variable, and having parent thread
  do the final computation.
**/

#include <pthread.h>
#include <unistd.h> // sleep()
#include <stdio.h> // printf
#include <stdlib.h> // rand()
//#include <semaphore.h> // sem_t
#include <string.h> // strerror()
#include <errno.h> // errno
#include <assert.h>

// Thread definitions.
#define NUM_RAND_POINTS (1 << 16)
#define NUM_WORKER_THREADS 16
static pthread_t      Worker_thread_tid[NUM_WORKER_THREADS];
void * Worker_thread_func(void *param);
static long long inside_circle_count;
static pthread_mutex_t mutex;

// Cleanup state before terminating.
void cleanup_state (void) {
  inside_circle_count = 0;

  if(pthread_mutex_destroy(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
    return;
  }
}

// Init. program state.
int init_state(void) {
  FILE *fh;
  long my_seed = 0;

  //
  // Init. the random num. generator with a fresh seed.
  //
  fh = fopen("/dev/random", "r");

  if(fh == NULL) {
    assert(0);
    return -1;
  }

  if(fread((void*)&my_seed, sizeof(my_seed), 1, fh) != 1) {
    assert(0);
  }

  printf("Seed = %ld.\n", my_seed);

  srand48(my_seed);

  if(fclose(fh) != 0) {
    assert(0);
    return -1;
  }

  //
  // Init. mutex.
  //
  if(pthread_mutex_init(&mutex, NULL) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
    return -1;
  }

  return 0;
}

static long double monte_carlo_estimate_pi(long double num_inside, long double num_total) {
  assert(num_inside >= 0.0 && num_total >= 0.0);

  return (4.0*num_inside/num_total);
}

int main(void) {
  int i;
  const long double total_points = (long double) (NUM_RAND_POINTS*NUM_WORKER_THREADS);
  pthread_attr_t attr; /* set of attributes for the thread */

  if(init_state() != 0) {
    return -1;
  }

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

  /**

  "When this thread has exited, the parent thread will calculate and output the estimated value of Pi."

  **/

  // Print result
  printf("Main: Inside/Total = %lld/%Lf. Pi estimate= %Lf. peace out.\n", inside_circle_count, total_points, monte_carlo_estimate_pi((long double)inside_circle_count, total_points));


  cleanup_state();

  printf("Main: peace out.\n");

  return 0;
}

static void monte_carlo_get_rand_point(double *x, double *y) {

  // drand48() Rand. number in [0.0-1.0)

  *x = (drand48() - 0.5) * 2.0;
  *y = (drand48() - 0.5) * 2.0;
}

static int monte_carlo_is_point_in_circle(double x, double y) { // returns 1 if the given point is inside the unit circle, 0 otherwise.
  double distance;
  // Compute distance from origin. If < 1 then its in the circle.

  distance = x * x + y * y;

  if(distance < 1.0)
    return 1;

  return 0;
}

/**
 * The thread will begin control in this function
 */
void *Worker_thread_func(void *param)
{
  pthread_t tid;
  unsigned long id = 0;
  double x, y;
  int i;
  long long my_inside_count;

  /*

  "Write a multi-threaded version of this algorithm that creates a separate
  thread to generate a number of random points. "

   "The thread will count the number of points that occur within the circle
   and store that result in a global variable."

  */

  // Use the tid address to uniquely identify a thread's prints.
  tid = pthread_self();
  id = (unsigned long) &tid; // !!! TODO: go fix this in all previous code. i.e. Rm. get_t_num().s

  printf("#%lu: Hello bro, monte carlo.\n", id);

  my_inside_count = 0;

  for(i = 0; i < NUM_RAND_POINTS; i++) {

    monte_carlo_get_rand_point(&x, &y); // Generate a random point inside the unit square.

    if(monte_carlo_is_point_in_circle(x, y)) {
      my_inside_count += 1;
      //printf("#%lu: INSIDE. (%f,%f). Count = %lld.\n", id, x, y, my_inside_count);
    } else {
      //printf("#%lu: OUTSIDE. (%f,%f). Count = %lld.\n", id, x, y, my_inside_count);
    }

  }

  printf("#%lu: Done bro. Count = %lld.\n", id, my_inside_count);
  // lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
    pthread_exit(NULL);
  }

  inside_circle_count += my_inside_count; // Update global count.

  //unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
    pthread_exit(NULL);
  }

  printf("#%lu: Goodbye bro.\n", id);

  pthread_exit(NULL);
}
