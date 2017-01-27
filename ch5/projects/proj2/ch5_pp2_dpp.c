/**

 Solution to DPP using Pthreads mutex's and condition vars.

 Each P will runs as sep. thread. 0..4.
 P's alternate between THINKING, EATING. // HUNGRY too. Which means he would like to eat.
 Simulate state by rand. sleep time between 1 and 3 secs.

""" From the book """

Condition variables in Pthreads use the pthread_cond_t data type and
are initialized using the pthread_cond_init() function. The following code
creates and initializes a condition variable as well as its associated mutex lock:

pthread_mutex_t mutex;
pthread_cond_t cond_var;
pthread_mutex_init(&mutex,NULL);
pthread_cond_init(&cond_var,NULL);


The pthread_cond_wait() function is used for waiting on a condition
variable. The following code illustrates how a thread can wait for the condition
a == b to become true using a Pthread condition variable:


pthread_mutex_lock(&mutex);
while (a != b)
  pthread_cond_wait(&mutex, &cond var);
pthread_mutex_unlock(&mutex);


The mutex lock associated with the condition variable must be locked
before the pthread_cond_wait() function is called, since it is used to protect
the data in the conditional clause from a possible race condition. Once this
lock is acquired, the thread can check the condition. If the condition is not true,
the thread then invokes pthread_cond_wait(), passing the mutex lock and
the condition variable as parameters. Calling pthread_cond_wait() releases
the mutex lock, thereby allowing another thread to access the shared data and
possibly update its value so that the condition clause evaluates to true. (To
protect against program errors, it is important to place the conditional clause
within a loop so that the condition is rechecked after being signaled.)
A thread that modifies the shared data can invoke the
pthread_cond_signal() function, thereby signaling one thread waiting
on the condition variable. This is illustrated below:


pthread_mutex_lock(&mutex);
a = b;
pthread_cond_signal(&cond var);
pthread_mutex_unlock(&mutex);


It is important to note that the call to pthread_cond_signal() does not
release the mutex lock. It is the subsequent call to pthread_mutex_unlock()
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

static int left_neighbor(int philosopher_number) {
  int l = (philosopher_number + 4) % 5;

  printf("Philosopher %d: Left neighbor = %d.\n", philosopher_number, l);

  return l;
}

static int right_neighbor(int philosopher_number) {
  int r = (philosopher_number + 1) % 5;

  printf("Philosopher %d: Right neighbor = %d.\n", philosopher_number, r);

  return r;
}

static void print_philosopher_state(void) {
  int i;

  for(i = 0; i < NUM_PHILOSOPHERS; i++) {
      printf("[%d]", PhilosopherState[i]);
  }
  printf("\n");

}

static void test(int philosopher_number) {
  // Use case 1: If I am hungry and both my neighbors are not eating, then I may start eating.
  // Use case 2: I am done eating now, let me inform my neighbors that I'm done eating in case they are hungry.
  printf("test(%d). IN.\n", philosopher_number);

  // Since this function is not called directly by a P thread. We don't need to
  // acquire the main mutex since it has already been acquired by the calling P
  // thread via pickup*() or return*().

  printf("test(%d). Got lock.\n", philosopher_number);

  if(   PhilosopherState[left_neighbor(philosopher_number)] != EATING
    &&  PhilosopherState[right_neighbor(philosopher_number)] != EATING
    &&  PhilosopherState[philosopher_number] == HUNGRY
    ) {

    // Not sure about this. This is direct translation of Galvin. May need adjustment
    printf("test(%d). Chopsticks available.\n", philosopher_number);

    // self lock // each self mutex ensures mutually exclusive access to the P state array.
    // if(pthread_mutex_lock(&self_mutex[philosopher_number]) != 0) { // der-NOTE: do i really need this? Seems like main mutex is sufficient. RESULT OF REMOVING: No asserts.
    //   printf("%s\n", strerror(errno));
    //   assert(0);
    // }

    printf("test(%d). Chopsticks available. Got lock.\n", philosopher_number);

    // self update cond.
    PhilosopherState[philosopher_number] = EATING;

    // self cond. var. signal.
    if (pthread_cond_signal(&self_cond_var[philosopher_number]) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
    // self unlock
    // if(pthread_mutex_unlock(&self_mutex[philosopher_number]) != 0) {
    //   printf("%s\n", strerror(errno));
    //   assert(0);
    // }
  }

  printf("test(%d). OUT.\n", philosopher_number);
}

/**

  Invoked by a P wishing to eat.

**/
static void pickup_forks(int philosopher_number) {

  // main lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
  // TODO
  printf("#%d: I so hungry.\n", philosopher_number);

  // I'm hungry.
  PhilosopherState[philosopher_number] = HUNGRY;
  // May I eat now?
  test(philosopher_number);

  // If I can't eat now, then I'll wait for my turn. One of my neighbors will tell me when its my turn.

  while (PhilosopherState[philosopher_number] != EATING) {
    printf("#%d:  Waiting to eat...\n", philosopher_number);
    // self cond. var. wait.
    if (pthread_cond_wait(&self_cond_var[philosopher_number], &mutex) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }

  }

  printf("#%d: I can eat now!\n", philosopher_number);
  print_philosopher_state();
  assert(PhilosopherState[left_neighbor(philosopher_number)] != EATING && PhilosopherState[right_neighbor(philosopher_number)] != EATING);

  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  return;
}

/**

  Invoked by a P when finished eating.

**/
static void return_forks(int philosopher_number, unsigned int eat_count) {
  printf("#%d: return_forks. IN.\n", philosopher_number);

  assert(eat_count > 0);

  // main lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("#%d: Done eating. Telling my neighbors I'm going back to thinking.\n", philosopher_number);

  PhilosopherState[philosopher_number] = THINKING;

  printf("#%d: I just set my state to thinking.\n", philosopher_number);
  if(eat_count % 2 == 0) { // even L then R neighbor.
    // der-NOTE: I suspect alternating these calls should fix the starvation possibility. First Lets see if we can observe starvation by counting the number of times each P gets to eat.
    test(left_neighbor(philosopher_number));
    test(right_neighbor(philosopher_number));
  } else { // Odd R then L neighbor.
    test(right_neighbor(philosopher_number));
    test(left_neighbor(philosopher_number));
  }
  // TODO

  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("#%d: return_forks. OUT.\n", philosopher_number);

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
  int eating_count = 0;

  printf("#%d: Hello, I am a philosopher.\n", philosopher_number);

  do {

    // TODO:
    printf("#%d: Thinking...\n", philosopher_number);
    rand_sleep(philosopher_number, MAX_SLEEP_TIME); // Think.

    printf("#%d: Hungry...\n", philosopher_number);
    pickup_forks(philosopher_number);

    // assert state neighbors not eating.
    assert(PhilosopherState[left_neighbor(philosopher_number)] != EATING && PhilosopherState[right_neighbor(philosopher_number)] != EATING);

    eating_count++;
    printf("#%d: Eating... count = %d.\n", philosopher_number, eating_count);
    fprintf(stderr, "#%d: Eating... count = %d.\n", philosopher_number, eating_count);
    rand_sleep(philosopher_number, MAX_SLEEP_TIME); // Eat.

    // assert state neighbors not eating.
    assert(PhilosopherState[left_neighbor(philosopher_number)] != EATING && PhilosopherState[right_neighbor(philosopher_number)] != EATING);

    return_forks(philosopher_number, eating_count);

  } while(1);

  pthread_exit(0); // der-Q: is this necessary, or is returning from here the same ? Suspect necessary of want to return a specific return status.
}