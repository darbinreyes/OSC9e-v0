/**
 * Example illustrating POSIX unnamed semaphores
 *
 * Compilation (on Linux):
 *
 *  gcc -lpthread posix-unnamed-sem.c
 *
 * This example includes the appropriate error checking
 * that is not covered in the text.
 *
 * Note that OS X does not support unnamed sempahores
 * so this program will not work on those systems.
 *
 * Operating System Concepts - Ninth Edition
 * John Wiley & Sons - 2013.
 */

#include <unistd.h>     
#include <sys/types.h>  
#include <errno.h>      
#include <stdio.h>      
#include <stdlib.h>     
#include <pthread.h>    
#include <string.h>     
#include <semaphore.h>  
#include <assert.h>

void *TA_thread_func(void *param);
void *Student_thread_func(void *param);

#define NUM_STUDENTS 2
#define NUM_TA_WAIT_CHAIRS 3

typedef enum {
  PROGRAMMING,
  WAITING_FOR_TA,
  WITH_TA
} STUDENT_STATE;

typedef enum {
  NAPPING,
  TEACHING
} TA_STATE;

static pthread_t      TA_tid; /* the TA thread identifier */
static pthread_t      Stu_tid[NUM_STUDENTS]; /* the Student thread(s) identifier */
static pthread_attr_t Stu_attr[NUM_STUDENTS]; /* set of attributes for the thread */
static pthread_attr_t attr; /* set of attributes for the thread */

static STUDENT_STATE Students[NUM_STUDENTS];
static TA_STATE Ta;

static sem_t TA_test_sem;
static sem_t TA_lock_sem;

void init_state(void) {
  int i = 0;

  Ta = NAPPING;

  while(i < NUM_STUDENTS) {
    Students[i++] = PROGRAMMING;
  }


  if (sem_init(&TA_test_sem, 0, 1) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_init(&TA_lock_sem, 0, 1) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

void cleanup_state(void) {

  if (sem_destroy(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_lock_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

int main(void)
{

  sem_t sem;
  int i;

  if (sem_init(&sem, 0, 1) == -1) {
    printf("%s\n", strerror(errno));
  }

  if (sem_wait(&sem) != 0) {
    printf("%s\n", strerror(errno));
  }

  printf("*** Critical Section *** \n");

  if (sem_post(&sem) != 0) {
    printf("%s\n", strerror(errno));
  }

  printf("*** Non-Critical Section *** \n");

  if (sem_destroy(&sem) != 0) {
    printf("%s\n", strerror(errno));
  }

  init_state();

  /* get the default attributes */
  pthread_attr_init(&attr);

  /* create the TA thread */
  pthread_create(&TA_tid, &attr, TA_thread_func, NULL);

  /* create the Student thread(s) */
  i = 0;
  while(i < NUM_STUDENTS) {
    printf("Student %d: Creating thread...\n", i);
    pthread_attr_init(&Stu_attr[i]);
    pthread_create(&Stu_tid[i], &Stu_attr[i], Student_thread_func, NULL); /// holy shit, this was a race condition! my first one encountered in practice.
    i++;
  }

  /* now wait for the thread(s) to exit */
  pthread_join(TA_tid, NULL);

  i = 0;
  while(i++ < NUM_STUDENTS) {
    pthread_join(Stu_tid[i], NULL);
  }

  cleanup_state();

  return 0;
}

/**
 * The thread will begin control in this function
 */
void *TA_thread_func(void *param)
{
  int count = 3;

  do {
    printf("TA: Napping...\n");
    sleep(1);
    printf("TA: Teaching...\n");
  } while(1);

  pthread_exit(0);
}

void test_is_ta_busy(int stud_id) { // returns 1 if the TA is currently busy with another student. 0 otherwise, meaning the TA is free, FYI : free, but he may be napping.
  int i = 0;
  int rv = 0;

  if (sem_wait(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    // EAGAIN = lock not avail.
    assert(0);
  }

  printf("*** TA test Critical Section *** \n");

  while(i < NUM_STUDENTS) {
    if(Students[i++] == WITH_TA) {
      rv = 1;
      break;
    }
  }

  if(rv == 0) { // TA available ! lock him in the office.
    if (sem_trywait(&TA_lock_sem) != 0) {
      printf("%s\n", strerror(errno));
      printf("*** %d: TA busy alone! BUG! *** \n", stud_id);
      Students[stud_id] = WAITING_FOR_TA;
      assert(0); // Sanity check, the TA lock should be available if no student is with the TA.
    } else {
      printf("*** %d: GOT TA lock! *** \n", stud_id);
      Students[stud_id] = WITH_TA;
    }
  } else {
    printf("*** %d: TA busy! *** \n", stud_id);
    Students[stud_id] = WAITING_FOR_TA; // TODO: add chair wait feature
  }

  if (sem_post(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

}

void release_TA(int stud_id) {
  Students[stud_id] = PROGRAMMING; // go back to programming.
  printf("*** %d: releasing TA lock! *** \n", stud_id);
  if (sem_post(&TA_lock_sem) != 0) { // release TA.
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

int get_student_id(void) { // assign a unique integer to each student.
  int i = 0;
  pthread_t tid;

  while(i < NUM_STUDENTS) {
    tid = pthread_self();
    if(Stu_tid[i++] == tid) {
      return i - 1;
    }
  }
  assert(0);
}

/**
 * The thread will begin control in this function
 */
void *Student_thread_func(void *param)
{
  #define MAX_TIME 5
  int count = 3;
  int t;
  int i = get_student_id();
  pthread_t  tid = pthread_self();

  do {

    t = rand() % MAX_TIME;
    printf("Student %d: %p: Programming... t = %d.\n", i, tid, t);
    sleep(t); // spend time programming.

    test_is_ta_busy(i);

    if(Students[i] == WAITING_FOR_TA) {
      t = rand() % MAX_TIME;
      printf("Student %d: %p: Waiting... t = %d.\n", i, tid, t);
      sleep(t); //  spend time waiting
      //test_is_ta_busy(i); // try again.
    }

    if(Students[i] == WITH_TA) {
      t = rand() % MAX_TIME;
      printf("Student %d: %p: With TA... t = %d.\n", i, tid, t);
      sleep(t); //  spend time with TA
      release_TA(i);
    }

    Students[i] = PROGRAMMING;

  } while(1);

  pthread_exit(0);
}
