/**

# Programming Problem 4.21.

## Start: From the Book

> Write a multithreaded program that calculates various statistical values for a
> list of numbers. This program will be passed a series of numbers on the command
> line and will then create three separate worker threads. One thread will
> determine the average of the numbers, the second will determine the maximum
> value, and the third will determine the minimum value. For example, suppose your
> program is passed the integers 90 81 78 95 79 72 85 The program will report The
> average value is 82 The minimum value is 72 The maximum value is 95 The
> variables representing the average, minimum, and maximum values will be stored
> globally. The worker threads will set these values, and the parent thread will
> output the values once the workers have exited. (We could obviously expand this
> program by creating additional threads that determine other statistical values,
> such as median and standard deviation.) ## End

## End


  * Plan:
    * Store ints in static buffer. Use atoi().
**/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // atoi()
#include <assert.h>

#define MAX_SAMPLE_SIZE 256
#define NUM_STATISTICAL_OPERATIONS 3 // Avg. , Min, Max. 1 T each.

static int val_buffer[MAX_SAMPLE_SIZE];
static int val_buffer_len = -1;

static double avg, max, min = -1.0; // Global result vars.

void *runner(void *param); /* the thread */

static void avg_calc(void) {
  int i, sum = 0;

  for(i = 0; i < val_buffer_len; i++)
    sum += val_buffer[i];


  avg = ((double)sum) / ((double) val_buffer_len);
}

static void min_calc(void) {
  int i, min_val;

  min_val = val_buffer[0];

  for(i = 1; i < val_buffer_len; i++)
    if(min_val > val_buffer[i])
      min_val = val_buffer[i];


  min = ((double)min_val);
}

static void max_calc(void) {
  int i, max_val;

  max_val = val_buffer[0];

  for(i = 1; i < val_buffer_len; i++)
    if(max_val < val_buffer[i])
      max_val = val_buffer[i];


  max = ((double)max_val);
}

typedef void (*statistical_calc_func)(void);

static statistical_calc_func calc_funcs[NUM_STATISTICAL_OPERATIONS] = {avg_calc, min_calc, max_calc};

int main(int argc, char *argv[])
{
  int i;
  pthread_t tid[NUM_STATISTICAL_OPERATIONS]; /* the thread identifier */
  pthread_attr_t attr; /* set of attributes for the thread */
  /**

  * calculates various statistical values for a list of numbers.
  * This program will be passed a series of numbers on the command line.
  * Then create three separate worker threads.
    * Avg.
    * Max.
    * Min.

  * E.g. Given 90 81 78 95 79 72 85. The program will report:

  * The average value is 82
  * The minimum value is 72
  * The maximum value is 95

  * variables representing the average, minimum, and maximum values will be stored globally.
  * worker threads will set these values, and the parent thread will output the values once the workers have exited.
  **/

  if (argc <= 1 || (argc - 1) >= MAX_SAMPLE_SIZE) {
    fprintf(stderr,"Usage: a.out <integer value> <integer value> ... N.\nMax N = %d.\n", MAX_SAMPLE_SIZE);
    /*exit(1);*/
    return -1;
  }

  for(i = 1; i < argc; i++) {
    val_buffer[i - 1] = atoi(argv[i]);
    if (val_buffer[i - 1] <= 0) {
      fprintf(stderr,"Argument %d must be > 0.\n", val_buffer[i - 1]);
      /*exit(1);*/
      return -1;
    }
  }

  val_buffer_len = argc - 1;

  /* get the default attributes */
  pthread_attr_init(&attr);

  for(i = 0; i < NUM_STATISTICAL_OPERATIONS; i++) {
    /* create the thread */
    pthread_create(&tid[i], &attr, runner, calc_funcs[i]);
  }

  for(i = 0; i < NUM_STATISTICAL_OPERATIONS; i++) {
    /* now wait for the thread to exit */
    pthread_join(tid[i], NULL);
  }

  pthread_attr_destroy(&attr); // Cleanup.

  printf("The average value is = %f.\n", avg);
  printf("The min value is = %f.\n", min);
  printf("The max value is = %f.\n", max);

  return 0;
}

/**
 * The thread will begin control in this function
 */
void *runner(void *param) 
{
  if(param != NULL)
    ((statistical_calc_func)param)();
  else
    assert(0);

  pthread_exit(0);
}
