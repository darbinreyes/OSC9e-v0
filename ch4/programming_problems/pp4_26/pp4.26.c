/**

# Programming Problem 4.26.

## Start: From the Book

> The Fibonacci sequence is the series of numbers 0, 1, 1, 2, 3, 5, 8, ....
> Formally, it can be expressed as: fib0 =0 fib1 =1 fibn = fibn−1 + fibn−2 Write a
> multithreaded program that generates the Fibonacci sequence. This program should
> work as follows: On the command line, the user will enter the number of
> Fibonacci numbers that the program is to generate. The program will then create
> a separate thread that will generate the Fibonacci numbers, placing the sequence
> in data that can be shared by the threads (an array is probably the most
> convenient data structure). When the thread finishes execution, the parent
> thread will output the sequence generated by the child thread. Because the
> parent thread cannot begin outputting the Fibonacci sequence until the child
> thread finishes, the parent thread will have to wait for the child thread to
> finish. Use the techniques described in Section 4.4 to meet this requirement.

## End


# Plan:


**/

#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define N_FIB_MAX 1024

static int fib_sequence_buf[N_FIB_MAX] = {0,1}; // Buffer array for storing the computed Fibonacci sequence

void *runner(void *param); /* the thread */

int main(int argc, char *argv[])
{
 pthread_t tid; /* the thread identifier */
 pthread_attr_t attr; /* set of attributes for the thread */
 int n_fib, i;

  // "The user will enter the number of Fibonacci numbers that the program is to generate."

  if (argc != 2) {
    fprintf(stderr,"usage: a.out <integer value>\n");
    /*exit(1);*/
    return -1;
  }

  if ((n_fib = atoi(argv[1])) <= 0 || n_fib > N_FIB_MAX) { // der-ERROR-HANDLING-IMPROVEMENT: atoi() doesn't tell you if input was a valid int.
    fprintf(stderr,"Argument %d must greater than 0 and less than or equal to %d.\n", n_fib, N_FIB_MAX);
    /*exit(1);*/
    return -1;
  }

  // "Then create a separate thread that will generate the Fibonacci numbers, placing the sequence in data that can be shared by the threads"

  /* get the default attributes */
  pthread_attr_init(&attr); // FYI: Missing call to pthread_attr_destroy()? Ref: chegg.com + man page.

  /* create the thread */
  pthread_create(&tid, &attr, runner, &n_fib);

  // "When the thread finishes execution, the parent thread will output the sequence generated by the child thread."

  /* now wait for the thread to exit */
  pthread_join(tid, NULL);

  for(i = 0; i < n_fib - 1; i++) {
    printf("%d ", fib_sequence_buf[i]);
  }

  printf("%d\n", fib_sequence_buf[i]);

  pthread_attr_destroy(&attr);
}

/**
 * The thread will begin control in this function
 */
void *runner(void *param) 
{
  int n_fib, i;

  n_fib = *((int *)param);

  for(i = 2; i < n_fib; i++) {
    fib_sequence_buf[i] = fib_sequence_buf[i - 1] + fib_sequence_buf[i - 2];
  }

  pthread_exit(0);
}
