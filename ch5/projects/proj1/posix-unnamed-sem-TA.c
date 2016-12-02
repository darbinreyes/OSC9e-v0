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
static sem_t TA_chairs_sem;
static int waiting_count;

void init_state(void) {
  int i = 0;

  Ta = NAPPING;
  waiting_count = 0;

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

  if (sem_init(&TA_chairs_sem, 0, 0) == -1) {
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

  if (sem_destroy(&TA_chairs_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

int main(void)
{
  int i;

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

void acquire_chair(int stud_id) {

  if (sem_wait(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    // EAGAIN = lock not avail.
    assert(0);
  }

  printf("*** %d: acquiring chair! *** \n", stud_id);

  if(waiting_count < NUM_TA_WAIT_CHAIRS) {
    assert(waiting_count < 3);
    Students[stud_id] = WAITING_FOR_TA; // sit down , wait for TA.
    waiting_count++;
    printf("*** %d: chair available. waiting...count = %d. *** \n", stud_id, waiting_count);
  } else {
    Students[stud_id] = PROGRAMMING;
    printf("*** %d: No chairs available! back to programming. waiting...count = %d. *** \n", stud_id, waiting_count);
    assert(waiting_count == 3);
  }

  if (sem_post(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

}

void release_chair(int stud_id) {
  int next_student, i;
  // this test is mutually exclusive.
  if (sem_wait(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    // EAGAIN = lock not avail.
    assert(0);
  }

  if (sem_post(&TA_lock_sem) != 0) { // release TA.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("*** %d: releasing chair! *** \n", stud_id);

  Students[stud_id] = PROGRAMMING;
  // assert((waiting_count > 0));
  if(waiting_count > 0) {
    i = (stud_id + 1) % NUM_STUDENTS; // give next student a chance.

    while(i != stud_id) {
      if(Students[i] == WAITING_FOR_TA)
        break;
      i = i + 1 % NUM_STUDENTS;
    }

    if(Students[i] == WAITING_FOR_TA) {
      printf("*** next_student = %d *** \n", i);
      Students[i] = WITH_TA;
    } else {
      // assert
    }

    waiting_count--;
    // if (sem_post(&TA_chairs_sem) != 0) { // signal a waiting student that ur done with the TA.
    //   printf("%s\n", strerror(errno));
    //   assert(0);
    // }
  }

  if (sem_post(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

void acquire_TA(int stud_id) {
  if (sem_trywait(&TA_lock_sem) != 0) {
    printf("%s\n", strerror(errno));
    printf("*** %d: TA busy alone! BUG! *** \n", stud_id);
    assert(0); // Sanity check, the TA lock should be available if no student is with the TA.
  } else {
    printf("*** %d: GOT TA lock! *** \n", stud_id);
    Students[stud_id] = WITH_TA;
  }
}

void release_TA(int stud_id) {
  release_chair(stud_id);

  printf("*** %d: releasing TA lock! *** \n", stud_id);
}

void test_is_ta_busy(int stud_id) { // returns 1 if the TA is currently busy with another student. 0 otherwise, meaning the TA is free, FYI : free, but he may be napping.
  int i = 0;
  int is_student_with_TA = 0;
  // this test is mutually exclusive.
  if (sem_wait(&TA_test_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("*** TA test Critical Section *** \n");

  if(waiting_count > 0) {
    // if you see students already waiting for the TA you either:
    // 1) sit down and wait in a chair for the TA.
    // OR
    // 2) go back to programming because there are no chairs.
    printf("*** %d: TA busy! And theres a line. fuk. *** \n", stud_id);
    goto Exit;
  }

  //assert(waiting_count == 0);

  // no one is waiting. check if someone is with him.
  while(i < NUM_STUDENTS) {
    if(Students[i++] == WITH_TA) { // TODO: you should need to check your own state. add assertions on state. e.g. programming on entry.
      is_student_with_TA = 1;
      break;
    }
  }

  if(is_student_with_TA == 0) { // TA available ! lock him in the office.
    acquire_TA(stud_id);
  } else {
    printf("*** %d: TA busy! *** \n", stud_id);
  }

Exit:
  if (sem_post(&TA_test_sem) != 0) {
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

     // spend time programming.
    t = rand() % MAX_TIME;
    printf("Student %d: %p: Programming... t = %d.\n", i, tid, t);
    sleep(t);

    // check if TA is available.
    test_is_ta_busy(i);

    if(Students[i] != WITH_TA) {
      // he wasn't available.
      acquire_chair(i);
    }

    if(Students[i] == WAITING_FOR_TA) {
      //t = rand() % MAX_TIME;
      printf("Student %d: %p: Waiting in chair...\n", i, tid);
      // if (sem_wait(&TA_chairs_sem) != 0) {
      //   printf("%s\n", strerror(errno));
      //   // EAGAIN = lock not avail.
      //   assert(0);
      // }
      while(Students[i] == WAITING_FOR_TA);
      printf("Student %d: %p: A student left the TAs office. Trying to get TA again... waiting_count = %d.\n", i, tid, waiting_count);
      acquire_TA(i);
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
