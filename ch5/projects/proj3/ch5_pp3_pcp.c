/**
### Given main outline
include "buffer.h"
int main(int argc, char *argv[]) { // 1. Get command line arguments argv[1],argv[2],argv[3] //
// 2. Initialize buffer //
// 3. Create producer thread(s) //
// 4. Create consumer thread(s) //
// 5. Sleep //
// 6. Exit //
}

### Given buffer header.
// buffer.h //

typedef int buffer item;
#define BUFFER SIZE 5

### Given buffer outline

#include "buffer.h"

// the buffer //
buffer item buffer[BUFFER SIZE];

int insert item(buffer item item) {
  //  insert item into buffer
  //  return 0 if successful, otherwise
  //  return -1 indicating an error condition
}

int remove item(buffer item *item) {
  //  remove an object from buffer
  //  placing it in item
  //  return 0 if successful, otherwise
  //  return -1 indicating an error condition /
}

### Given P/C thread outline

#include <stdlib.h> // required for rand() //
#include "buffer.h"

void *producer(void *param) {
  buffer item item;
  while (true) { // sleep for a random period of time //
    sleep(...);
    // generate a random number //
    item = rand();
    if (insert item(item))
      fprintf("report error condition");
    else
      printf("producer produced %d\n",item);
  }
}

void *consumer(void *param) {
  buffer item item;
  while (true) { // sleep for a random period of time //
    sleep(...);
    if (remove item(&item))
      fprintf("report error condition");
    else
      printf("consumer consumed %d\n",item);
  }
}

###### My Misc. Notes:

der-FYI:
"""
IMPLEMENTATION NOTES
     The atoi() and atoi_l() functions are *****thread-safe***** and async-cancel-safe.
"""

**/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // rand(), malloc(), free().




#define DEFAULT_MAIN_SLEEP_TIME 5
#define DEFAULT_NUM_PRODUCER_THREADS 5
#define DEFAULT_NUM_CONSUMER_THREADS 5

// Thread definitions.
static pthread_attr_t Thrd_attr; // Attribute struct common to all threads.
static pthread_t      *Producer_tid;
static pthread_t      *Consumer_tid;

void *Producer_thread_func(void *param);
void *Consumer_thread_func(void *param);

int main(int argc, char *argv[])
{
  pthread_t tid; /* the thread identifier */
  pthread_attr_t attr; /* set of attributes for the thread */
  int main_sleep_time, num_producer_threads, num_consumer_threads;
  /*** START: Get args ***/
  if(argc == 1) {
    // My spec: No args means use default values.
    main_sleep_time = DEFAULT_MAIN_SLEEP_TIME;
    num_producer_threads = DEFAULT_NUM_PRODUCER_THREADS;
    num_consumer_threads = DEFAULT_NUM_CONSUMER_THREADS;
  } else if (argc != 4) {
    fprintf(stderr,"usage: a.out <main_sleep_time.integer value> <num_producer_threads.integer value> <num_consumer_threads.integer value>\n");
    /*exit(1);*/
    return -1;
  }

  if ((main_sleep_time = atoi(argv[1])) < 0) {
    fprintf(stderr,"Argument %d must be non-negative\n", atoi(argv[1]));
    /*exit(1);*/
    return -1;
  }
  if ((num_producer_threads = atoi(argv[2])) < 0) {
    fprintf(stderr,"Argument %d must be non-negative\n", atoi(argv[2]));
    /*exit(1);*/
    return -1;
  }

  if ((num_consumer_threads = atoi(argv[3])) < 0) {
    fprintf(stderr,"Argument %d must be non-negative\n", atoi(argv[3]));
    /*exit(1);*/
    return -1;
  }

  printf("main_sleep_time = %d. num_producer_threads = %d. num_consumer_threads = %d.\n", main_sleep_time, num_producer_threads, num_consumer_threads);
  /*** END: Get args ***/

  /* get the default attributes */
  pthread_attr_init(&attr);

  /* create the thread */
  pthread_create(&tid,&attr,runner,argv[1]);

  /* now wait for the thread to exit */
  pthread_join(tid,NULL);

  printf("sum = %d\n",sum);
}

/**
 * The thread will begin control in this function
 */
void *runner(void *param)
{
int i, upper = atoi(param);
sum = 0;

  if (upper > 0) {
    for (i = 1; i <= upper; i++)
      sum += i;
  }

  pthread_exit(0);
}
