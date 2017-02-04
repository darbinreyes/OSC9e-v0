/**

# Programming Problem 4.22.

## Start: From the Book

> An interesting way of calculating   is to use a technique known as Monte Carlo,
> which involves randomization. This technique works as follows: Suppose you have
> a circle inscribed within a square, as shown in Figure 4.18. (Assume that the
> radius of this circle is 1.) First, generate a series of random points as simple
> (x, y) coordinates. These points must fall within the Cartesian coordinates that
> bound the square. Of the total number of random points that are generated, some
> will occur within the circle. Next, estimate by performing the following
> calculation:   = 4× (number of points in circle) / (total number of points)
> Write a multithreaded version of this algorithm that creates a separate thread
> to generate a number of random points. The thread will count the number of
> points that occur within the circle and store that result in a global variable.
> When this thread has exited, the parent thread will calculate and output the
> estimated value of  . It is worth experimenting with the number of random points
> generated. As a general rule, the greater the number of points, the closer the
> approximation to  . In the source-code download for this text, we provide a
> sample program that provides a technique for generating random numbers, as well
> as determining if the random (x, y) point occurs within the circle. Readers
> interested in the details of the Monte Carlo method for esti- mating   should
> consult the bibliography at the end of this chapter. In Chapter 5, we modify
> this exercise using relevant material from that chapter.

> First, generate a series of random points as simple (x, y) coordinates. These
> points must fall within the Cartesian coordinates that bound the square.

## End


* Plan:
  * 1. Implement void monte_carlo_get_rand_point(double *x, double *y); // x y in square
  * 2. Implement int monte_carlo_is_in_circle(double x, double y); // ret. 1 if in circle, else 0;
  * 3. In thread, use for loop and the set the global result array before terminating.
  * 4. In main, once all threads have terminated, compute the estimate for Pi.
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
#define NUM_RAND_POINTS 16
#define NUM_WORKER_THREADS 1
static pthread_t      Worker_thread_tid[NUM_WORKER_THREADS];
void * Worker_thread_func(void *param);
static int inside_circle_count[NUM_RAND_POINTS];

// Cleanup state before terminating.
void cleanup_state (void) {
}

// Init. program state.
int init_state(void) {
  FILE *fh;
  long my_seed = 0;

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

  return 0;
}

int main(void) {
  int i;
  const double total_points = (double) NUM_RAND_POINTS;
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
    if(pthread_create(&Worker_thread_tid[i], &attr, Worker_thread_func, &inside_circle_count[i]) != 0) {
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

  // Print result
  for(i = 0; i < NUM_WORKER_THREADS; i++) {
    printf("Index %d: Inside/Total = %d/%f. Pi estimate= %f. peace out.\n", i, inside_circle_count[i], total_points, (4.0*((double)inside_circle_count[i])/total_points));
  }

  cleanup_state();

  printf("Main. peace out.\n");

  return 0;
}

static void monte_carlo_get_rand_point(double *x, double *y) {
  double dt;

  dt = drand48(); // Rand. number in [0.0-1.0)

  if(dt < 0.5)
    *x = drand48() * -1.0;
  else
    *x = drand48() * 1.0;

  dt = drand48(); // Rand. number in [0.0-1.0)

  if(dt < 0.5)
    *y = drand48() * -1.0;
  else
    *y = drand48() * 1.0;

}

static int monte_carlo_is_point_in_circle(double x, double y) { // returns 1 if the given point is inside the unit circle, 0 otherwise.
  double distance;
  // Compute distance from origin. If < 1 then its in the circle.

  distance = x * x + y * y;

  if(distance < 1)
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
  int i, my_inside_count;

  /*

  "Write a multithreaded version of this algorithm that creates a separate
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
      printf("#%lu: INSIDE. (%f,%f). Count = %d.\n", id, x, y, my_inside_count);
    } else {
      printf("#%lu: OUTSIDE. (%f,%f). Count = %d.\n", id, x, y, my_inside_count);
    }

  }

  *((int *)param) = my_inside_count;

  printf("#%lu: Goodbye bro.\n", id);

  pthread_exit(0);
}
