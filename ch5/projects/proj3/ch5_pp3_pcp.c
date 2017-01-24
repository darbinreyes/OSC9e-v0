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
#include <semaphore.h> // sem_t
#include <errno.h> // errno
#include <assert.h>
#include "buffer.h"

#define DEFAULT_MAIN_SLEEP_TIME 5
#define DEFAULT_NUM_PRODUCER_THREADS 1
#define DEFAULT_NUM_CONSUMER_THREADS 1
#define MAX_PC_SLEEP_TIME 3

// Thread definitions.
static pthread_attr_t Thrd_attr; // Attribute struct common to all threads.
static pthread_t      *Producer_tid;
static pthread_t      *Consumer_tid;

void *Producer_thread_func(void *param);
void *Consumer_thread_func(void *param);
void rand_sleep(int caller_id, int max, char use_rand);

// Given shared data btw P/C threads.
int n; // My spec. : num items currently in the buffer.
static sem_t full_sem;
static sem_t empty_sem;
pthread_mutex_t mutex;
// the buffer
buffer_item buffer[BUFFER_SIZE];
int head_index;
int tail_index;

void cleanup_state (void) {
  assert(Producer_tid != NULL);
  free(Producer_tid);
  assert(Consumer_tid != NULL);
  free(Producer_tid);

  if (sem_destroy(&full_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&empty_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if(pthread_mutex_destroy(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}
// Init. program state.
void init_state(int num_producer_threads, int num_consumer_threads, int buff_size) {
  assert(buff_size > 0);
  assert(num_producer_threads > 0);
  assert(num_consumer_threads > 0);

  head_index = 0;
  tail_index = 0;
  n = 0;


  // Alloc. tid structs.
  Producer_tid = malloc(sizeof(pthread_t)*num_producer_threads);
  assert(Producer_tid != NULL);
  Consumer_tid = malloc(sizeof(pthread_t)*num_consumer_threads);
  assert(Consumer_tid != NULL);

  // Create an "unnamed" semaphore, flags = 0, init. value = buff_size.
  if (sem_init(&full_sem, 0, 0) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // Create an "unnamed" semaphore, flags = 0, init. value = 0.
  if (sem_init(&empty_sem, 0, buff_size) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // Init. mutex
  if(pthread_mutex_init(&mutex, NULL) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

int main(int argc, char *argv[])
{
  int i;
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

  if (argc == 4) {
    if ((main_sleep_time = atoi(argv[1])) <= 0) {
      fprintf(stderr,"Argument %d must be > 0.\n", atoi(argv[1]));
      /*exit(1);*/
      return -1;
    }
    if ((num_producer_threads = atoi(argv[2])) <= 0) {
      fprintf(stderr,"Argument %d must be > 0.\n", atoi(argv[2]));
      /*exit(1);*/
      return -1;
    }

    if ((num_consumer_threads = atoi(argv[3])) <= 0) {
      fprintf(stderr,"Argument %d must be > 0.\n", atoi(argv[3]));
      /*exit(1);*/
      return -1;
    }
  }

  printf("main_sleep_time = %d. num_producer_threads = %d. num_consumer_threads = %d.\n", main_sleep_time, num_producer_threads, num_consumer_threads);
  /*** END: Get args ***/

  /* get the default attributes */
  pthread_attr_init(&attr);

  // Init. buffer etc.
  init_state(num_producer_threads, num_consumer_threads, BUFFER_SIZE);

  /* Create the P threads */
  for(i = 0; i < num_producer_threads; i++) {
    pthread_create(&Producer_tid[i], &attr, Producer_thread_func, NULL);
  }
  /* Create the C threads */
  for(i = 0; i < num_consumer_threads; i++) {
    pthread_create(&Consumer_tid[i], &attr, Consumer_thread_func, NULL);
  }

  // Sleep for rand time.
  rand_sleep(-1, main_sleep_time, 0); // TODO: force threads to terminate


  /* Now wait for all the threads to exit */
  for(i = 0; i < num_producer_threads; i++) {
    pthread_join(Producer_tid[i], NULL);
  }
  for(i = 0; i < num_consumer_threads; i++) {
    pthread_join(Consumer_tid[i], NULL);
  }

  cleanup_state();
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

int insert_item(buffer_item item) {
  int result = 0;

  // main lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if(n < BUFFER_SIZE) {
    if (sem_wait(&empty_sem) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }

    // assert(n <= BUFFER_SIZE);

    buffer[tail_index] = item; // New items arrive at the tail of the queue.
    tail_index++;
    tail_index %= BUFFER_SIZE;
    n++;

    if (sem_post(&full_sem) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
  } else { // n >= BUFFER_SIZE
    result = -1;
  }

  //  insert item into buffer
  //  return 0 if successful, otherwise
  //  return -1 indicating an error condition
  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  return result;
}

int remove_item(buffer_item *item) {
  int result = 0;

  // main lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if(n > 0) {
    if (sem_wait(&full_sem) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }

    //assert(n > 0);

    *item = buffer[head_index]; // Items are removed from the head of the queue.
    head_index++;
    head_index %= BUFFER_SIZE;
    n--;

    if (sem_post(&empty_sem) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
  } else { // n == 0
    printf("Nothing for you to consume right now, bitch. Come back lata.\n");
    result = -1;
  }

  //  remove an object from buffer
  //  placing it in item
  //  return 0 if successful, otherwise
  //  return -1 indicating an error condition /
  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  return result;
}


/**
 * The thread will begin control in this function
 */
void *Producer_thread_func(void *param)
{
  buffer_item dp;
  //char was_full = 0;

  do {
   rand_sleep(0, MAX_PC_SLEEP_TIME, 1);

   dp = rand();
   printf("I produced this: %d.\n", dp);

   if(insert_item(dp)) {
    // assert(0);
     printf("No space to produce.\n");
   } else {
     printf("I produced that.\n"); // TODO: save the item until there's space ?
   }

  } while (1); // TODO: exit when main tells u to.

  pthread_exit(0);
}

/**
 * The thread will begin control in this function
 */
void *Consumer_thread_func(void *param)
{
  buffer_item dc;

  do {
    rand_sleep(1, MAX_PC_SLEEP_TIME, 1);
    printf("I consume shit.\n");
    if(remove_item(&dc)) {
      // assert(0);
      printf("NADA to consume.\n");
    } else {
      printf("I consumed. %d.\n", dc);
    }

  } while (1);

  pthread_exit(0);
}
