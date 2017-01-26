/**

Q:
a. Identify the data involved in the race condition.
b. Identify the location (or locations) in the code where the race
condition occurs.
c. Using a semaphore or mutex lock, fix the race condition. It is
permissible to modify the decrease count() function so that the
calling process is blocked until sufficient resources are available.

* ANS:
    * a) avail_resources is inc/dec.’d concurrently, just like in the producer consumer example described by Galvin at the beginning of chap5. Without a mechanism to ensure atomic execution of the read/writes to avail_resources int. arbitrary interleaving of the underlying machine instructions will result in incorrect behaviour. e.g. avail_resources = 5. P0 calls dec. count(5). compare 5 < 5 = FALSE. context switch to P1 calls dec. count(5) to completion. avail_resources is now 0. context sw back to P0, the comparison was already made and the result was false so avail_resources will end up being -5 which should never occur. There cannot be negative licenses in this scenario.
    * b) all lines with “avail_resources”.
    * c) Yes we can block a P until resources become available. This is the default behaviour of sem_t’s in pthreads using sem_wait(s) + sem_post(t), s should be init. to max resources.

**/

#include <pthread.h>
#include <stdio.h> // printf
#include <stdlib.h> // rand(), malloc(), free().
#include <semaphore.h> // sem_t
#include <errno.h> // errno
#include <assert.h>


#define MAX_RESOURCES 5

static int available_resources = MAX_RESOURCES;

// sem. to control resource access. Assuming the SW license scenario.
static sem_t available_resources_sem;
static sem_t not_available_resources_sem; // Opposite of available_resources_sem. Similar to the producer consumer buffer use case.
static pthread_mutex_t mutex; // Ensures mutually ex. access to available_resources shared int var.
// Part c problem implementation.
static sem_t user_req_pending_sem; // Init. to 1. wait on entry to dec_count(). post/signal() on exit from dec_count(). Threads who could not get their count due to sufficient resources are blocked in this semaphore while waiting for their turn to get resources.

// Thread definitions.
#define NUM_LICENSE_USER_THREADS 7
static pthread_t      License_user_tid[NUM_LICENSE_USER_THREADS];
void *Licence_user_thread_func(void *param);

// Cleanup state before terminating.
void cleanup_state (void) {

  if(pthread_mutex_destroy(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&available_resources_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&user_req_pending_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

}

// Init. program state.
void init_state(void) {

  // Init. mutex
  if(pthread_mutex_init(&mutex, NULL) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // Create an "unnamed" semaphore, flags = 0, init. value = MAX_RESOURCES.
  if (sem_init(&available_resources_sem, 0, MAX_RESOURCES) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_init(&user_req_pending_sem, 0, 1) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

int main(void) {
  int i;
  pthread_attr_t attr; /* set of attributes for the thread */

  /* get the default attributes */
  pthread_attr_init(&attr);

  // Init. buffer etc.
  init_state();

  /* Create the P threads */
  for(i = 0; i < NUM_LICENSE_USER_THREADS; i++) {
    pthread_create(&License_user_tid[i], &attr, Licence_user_thread_func, NULL);
  }

  //TODO: cancel threads here.

  /* Now wait for all the threads to exit */
  for(i = 0; i < NUM_LICENSE_USER_THREADS; i++) {
    pthread_join(License_user_tid[i], NULL);
  }

  cleanup_state();
  return 0;
}

/* decrease available resources by count resources */
/* return 0 if sufficient resources available, */
/* otherwise return -1 */
int decrease_count(int count) {
  int i;
  int result;

  if (sem_wait(&user_req_pending_sem) != 0) { // Only 1 thread can be active below this block at a time.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // der-NOTE/TODO: I suspect there is a simpler way to achieve the same effect....

  for(i = 0; i < count; i++) { // Wait until there are enough resources to satisfy your req.
    printf("decrease_count: sem_wait() i = %d. count = %d.\n", i, count);
    if (sem_wait(&available_resources_sem) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
  }

  // main lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("decrease_count: available_resources = %d.\n", available_resources);

  assert(available_resources <= MAX_RESOURCES);

  if (available_resources < count) {
    printf("Insuff. resources for %d. Returning later.\n", count);
    result = -1; // Silly, you were leaving the mutex lock b/c of this return.
  } else {
    available_resources -= count;
    result = 0;
  }

  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if(result == 0) { // Indicate that you successfully got your request. Let the next thread get his chance.
    if (sem_post(&user_req_pending_sem) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
  }

  return result;
}

/* increase available resources by count */
int increase_count(int count) {
  int i;

  // main lock // Problem. 1 guy waits for lock to dec. the other waits to inc. = deadlock.
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  assert(available_resources <= MAX_RESOURCES);

  printf("increase_count: available_resources = %d.\n", available_resources);

  available_resources += count;

  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }


  for(i = 0; i < count; i++) { // Update available_resources_sem sem.
    printf("increase_count: sem_post() i = %d. count = %d.\n", i, count);
    if (sem_post(&available_resources_sem) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
  }

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

  for(i = 0; i < NUM_LICENSE_USER_THREADS; i++) {
    if(License_user_tid[i] == tid) {
      return i;
    }
  }

  assert(0);
}

#define MAX_SLEEP_TIME 3

/**
 * The thread will begin control in this function
 */
void *Licence_user_thread_func(void *param)
{
  int req_count;
  int id = get_t_num();
  int result = 0;

  do {
    rand_sleep(id, MAX_SLEEP_TIME, 1); // Go do something else.

    // Ask for licences.
    if(!result) {
      req_count = rand() % MAX_RESOURCES + 1;
      printf("#%d: Requesting count = %d.\n", id, req_count);
    } else {
      printf("#%d: Retrying request count = %d.\n", id, req_count);
    }

    result = decrease_count(req_count);

    if(result) {
      printf("#%d: Failed request count = %d.\n", id, req_count);
    }
    else {
      printf("#%d: Successfully acquired count = %d.\n", id, req_count);
      rand_sleep(id, MAX_SLEEP_TIME, 1); // Use license.
      // Return licenses.
      printf("#%d: Returning count = %d.\n", id, req_count);
      increase_count(req_count);
      printf("#%d: Successfully returned count = %d.\n", id, req_count);
    }

  } while (1); // TODO: exit when main tells u to.

  pthread_exit(0);
}
