/**

 Solution to DPP using Pthreads mutex's and condition vars.

 Each P will runs as sep. thread. 0..4.
 P's alternate between THINKING, EATING. // HUNGRY too. Which means he would like to eat.
 Simulate state by rand. sleep time between 1 and 3 secs.

""" From the book """

Condition variables in Pthreads use the pthread cond t data type and
are initialized using the pthread cond init() function. The following code
creates and initializes a condition variable as well as its associated mutex lock:

pthread mutex t mutex;
pthread cond t cond var;
pthread mutex init(&mutex,NULL);
pthread cond init(&cond var,NULL);


The pthread cond wait() function is used for waiting on a condition
variable. The following code illustrates how a thread can wait for the condition
a == b to become true using a Pthread condition variable:


pthread mutex lock(&mutex);
while (a != b)
  pthread cond wait(&mutex, &cond var);
pthread mutex unlock(&mutex);


The mutex lock associated with the condition variable must be locked
before the pthread cond wait() function is called, since it is used to protect
the data in the conditional clause from a possible race condition. Once this
lock is acquired, the thread can check the condition. If the condition is not true,
the thread then invokes pthread cond wait(), passing the mutex lock and
the condition variable as parameters. Calling pthread cond wait() releases
the mutex lock, thereby allowing another thread to access the shared data and
possibly update its value so that the condition clause evaluates to true. (To
protect against program errors, it is important to place the conditional clause
within a loop so that the condition is rechecked after being signaled.)
A thread that modifies the shared data can invoke the
pthread cond signal() function, thereby signaling one thread waiting
on the condition variable. This is illustrated below:


pthread mutex lock(&mutex);
a = b;
pthread cond signal(&cond var);
pthread mutex unlock(&mutex);


It is important to note that the call to pthread cond signal() does not
release the mutex lock. It is the subsequent call to pthread mutex unlock()
that releases the mutex. Once the mutex lock is released, the signaled thread
becomes the owner of the mutex lock and returns control from the call to
pthread cond. wait().

""" """
NEXT:
x1) Add thread creation code. Test with prints.
x2) Add state definitions.
x3) Add mutex and cond. var init. Test with prints.
NEXT: 4) Start translating Galvin's monitor solution.
Q: what is the diff. between the cond. var. def. by Galvin vs. the Pthreads def?
-one obvious difference is that a mutex is associated with the cond. var. explicitly instead
of hidden within the cond. var. implementation. NEXT: read man page for cond. var.

 The pthread_cond_init() function creates a new condition variable, with
     attributes specified with attr.  If attr is NULL the default attributes
     are used.

int
     pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);


The pthread_cond_wait() function atomically blocks the current thread
     waiting on the condition variable specified by cond, and releases the
     mutex specified by mutex.  The waiting thread unblocks only after another
     thread calls pthread_cond_signal(3), or pthread_cond_broadcast(3) with
     the same condition variable, and the current thread reacquires the lock
     on mutex.


     int
     pthread_cond_signal(pthread_cond_t *cond);


 The pthread_cond_signal() function unblocks one thread waiting for the
     condition variable cond.

SEE ALSO
     pthread_cond_broadcast(3), pthread_cond_destroy(3), pthread_cond_init(3),
     pthread_cond_timedwait(3), pthread_cond_wait(3)

The pthread_cond_init() function creates a new condition variable, with
     attributes specified with attr.  If attr is NULL the default attributes
     are used.
########## mutex man pages

 int
     pthread_mutex_init(pthread_mutex_t *mutex,
         const pthread_mutexattr_t *attr);

DESCRIPTION
     The pthread_mutex_init() function creates a new mutex, with attributes
     specified with attr.  If attr is NULL the default attributes are used.
 int
     pthread_mutex_destroy(pthread_mutex_t *mutex);

DESCRIPTION
     The pthread_mutex_destroy() function frees the resources allocated for
     mutex.

 int
     pthread_mutex_lock(pthread_mutex_t *mutex);

DESCRIPTION
     The pthread_mutex_lock() function locks mutex.  If the mutex is already
     locked, the calling thread will block until the mutex becomes available.
  int
     pthread_mutex_unlock(pthread_mutex_t *mutex);

DESCRIPTION
     If the current thread holds the lock on mutex, then the
     pthread_mutex_unlock() function unlocks mutex.

     Calling pthread_mutex_unlock() with a mutex that the calling thread does
     not hold will result in undefined behavior.

**/

//#include <unistd.h>
//#include <sys/types.h>
#include <errno.h> // errno
//#include <stdio.h>
//#include <stdlib.h>
#include <pthread.h>
//#include <string.h>
//#include <semaphore.h>
//#include <assert.h>

// my includes
#include <assert.h>
#include <stdlib.h> // rand()
#include <stdio.h> // printf

// Philosopher thread declarations.
#define MAX_SLEEP_TIME    3
#define NUM_PHILOSOPHERS  5

static pthread_t      Philosopher_tid[NUM_PHILOSOPHERS]; /* The Student thread(s) identifier */
static pthread_attr_t Philosopher_attr[NUM_PHILOSOPHERS]; /* Set of attributes for the thread */

typedef enum {
  THINKING,
  HUNGRY,
  EATING
} PHILOSOPHER_STATE;

static PHILOSOPHER_STATE PhilosopherState[NUM_PHILOSOPHERS];

// mutex and condition var. definitions
pthread_mutex_t mutex;
pthread_cond_t  cond_var;
// From Galvin's DDP monitor solution. mutex and condition var. definitions
pthread_mutex_t self_mutex[NUM_PHILOSOPHERS];
pthread_cond_t  self_cond_var[NUM_PHILOSOPHERS];

void *Philosopher_thread_func(void *param);

/**

  Cleanup this program's state before terminating.

**/
static void cleanup_state(void) {
  int i;

  if(pthread_mutex_destroy(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if(pthread_cond_destroy(&cond_var) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  for(i = 0; i < NUM_PHILOSOPHERS; i++) {
    if(pthread_mutex_destroy(&self_mutex[i]) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }

    if(pthread_cond_destroy(&self_cond_var[i]) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
  }
}

/**

  Initialize this program's state:
  -Philosophers thinking.
  -mutex and condition var. initializations.

**/
static void init_state(void) {
  int i = 0;



  if(pthread_mutex_init(&mutex, NULL) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (pthread_cond_init(&cond_var, NULL) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  for(i = 0; i < NUM_PHILOSOPHERS; i++) {
      PhilosopherState[i] = THINKING; // P's thinking.

      if(pthread_mutex_init(&self_mutex[i], NULL) != 0) {
        printf("%s\n", strerror(errno));
        assert(0);
      }

      if (pthread_cond_init(&self_cond_var[i], NULL) != 0) {
        printf("%s\n", strerror(errno));
        assert(0);
      }
  }
}

/**

  main.

**/
int main(void)
{
  int i;

  printf("I am main...\n");

  init_state();

  // Create and start the Philosopher thread(s)
  for(i = 0; i < NUM_PHILOSOPHERS; i++) {
    printf("Philosopher %d: Creating thread...\n", i);
    pthread_attr_init(&Philosopher_attr[i]);
    pthread_create(&Philosopher_tid[i], &Philosopher_attr[i], Philosopher_thread_func, NULL);
  }

  // Wait for each Philosopher thread to exit
  for(i = 0; i < NUM_PHILOSOPHERS; i++) {
    pthread_join(Philosopher_tid[i], NULL);
  }

  cleanup_state();

  return 0;
}


/**

  Invoked by a P wishing to eat.

**/
static void pickup_forks(int philosopher_number) {
  // TODO
  return;
}

/**

  Invoked by a P when finished eating.

**/
static void return_forks(int philosopher_number) {
  // TODO
  return;
}

/**

  Sleep for a random amount of time.

**/
void rand_sleep(int philosopher_number, int max) {
  int t;

  assert(philosopher_number >= 0 && max > 0);

#ifndef SLEEP_TIME_DISABLED

  // sleep rand. amnt. of time.
  t = rand() % max + 1;
  #ifdef MICRO_SECONDS_TIME_UNITS
    printf("#%d: sleeping %d usecs.\n", philosopher_number, t);
    usleep(t);
  #else
    printf("#%d: sleeping %d secs.\n", philosopher_number, t);
    sleep(t); // default
  #endif
  printf("#%d: Done sleeping.\n", philosopher_number);

#else // sleep calls disabled.

  static char no_sleep;
  if(!no_sleep) {
    no_sleep = 1;
    printf("#%d: Sleeping disabled.\n", philosopher_number);
  }

#endif

}

/**

  Assign a unique integer to each philosopher thread in the range of 0 to N-1,
  where N == Number of philosophers. The assignment is based to the thread's ID.

**/
static int get_philosopher_num(void) {
  int i = 0;
  pthread_t tid;

  tid = pthread_self();

  for(i = 0; i < NUM_PHILOSOPHERS; i++) {
    if(Philosopher_tid[i] == tid) {
      return i;
    }
  }

  assert(0);
}

void *Philosopher_thread_func(void *param) {
  int philosopher_number = get_philosopher_num();

  printf("#%d: Hello, I am a philosopher.\n", philosopher_number);

  do {

    // TODO:
    rand_sleep(philosopher_number, MAX_SLEEP_TIME);

  } while(1);

  pthread_exit(0); // der-Q: is this necessary, or is returning from here the same ? Suspect necessary of want to return a specific return status.
}