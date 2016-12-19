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



/**
 Student and TA state definitions.
**/
#define NUM_STUDENTS 2
#define NUM_TA_CHAIRS 3

typedef enum {
  PROGRAMMING,
  WAITING_FOR_TA,
  WITH_TA
} STUDENT_STATE;

typedef enum {
  NAPPING,
  TEACHING
} TA_STATE;

static STUDENT_STATE Students[NUM_STUDENTS];
static TA_STATE Ta;

// TA thread declarations
static pthread_t      TA_tid; /* the TA thread identifier */
static pthread_attr_t TA_attr; /* set of attributes for the thread */

// student thread declarations.
static pthread_t      Stu_tid[NUM_STUDENTS]; /* the Student thread(s) identifier */
static pthread_attr_t Stu_attr[NUM_STUDENTS]; /* set of attributes for the thread */

void *TA_thread_func(void *param);
void *Student_thread_func(void *param);

// semaphores declarations
static sem_t TA_lock_sem; // binary semaphore. ensures mutually exclusive access to TA's teaching time.
/**
  Semaphore representing the single chair in the TA's office.

  Init. to 0.

  A student waits in this semaphore until the TA signals it. The TA's signal tells
  the student that his time is up. He's must leave the office chair so that another
  student can have a chance.

**/
static sem_t TA_office_sem;

static int free_chairs_count;
static int studen_id_with_TA;

void init_state(void) {
  int i = 0;

  // init. state.
  studen_id_with_TA = -1; // no one has TA.
  Ta = NAPPING;

  for(i = 0; i < NUM_STUDENTS; i++) {
    Students[i++] = PROGRAMMING;
  }

  free_chairs_count = NUM_TA_CHAIRS;

  // init. semaphores.
  if (sem_init(&TA_lock_sem, 0, 1) == -1) { // create a semaphore, flags = 0, init. value = 1.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_init(&TA_office_sem, 0, 0) == -1) { // init to 0.
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

void cleanup_state(void) {

  // destroy semaphores.
  if (sem_destroy(&TA_lock_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_office_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

int main(void)
{
  int i;

  init_state();

  printf("Creating TA thread...\n", i);
  /* get the default attributes */
  pthread_attr_init(&TA_attr);

  /* create the TA thread */
  pthread_create(&TA_tid, &TA_attr, TA_thread_func, NULL);

  /* create the Student thread(s) */
  for(i = 0; i < NUM_STUDENTS; i++) {
    printf("Student %d: Creating thread...\n", i);
    pthread_attr_init(&Stu_attr[i]);
    pthread_create(&Stu_tid[i], &Stu_attr[i], Student_thread_func, NULL);
  }

  /* wait for the TA thread to exit */
  pthread_join(TA_tid, NULL);

  /* wait for each Student thread to exit */
  i = 0;
  for(i = 0; i < NUM_STUDENTS; i++) {
    pthread_join(Stu_tid[i], NULL);
  }

  cleanup_state();

  return 0;
}

void rand_sleep(int student_id, int max) { // sleep for a random number of seconds.
  int t;

  t = rand() % max;
  printf("student_id %d: sleeping for t = %d.\n", student_id, t);
  sleep(t);
}

void TA_kick_out_student (void) {
  printf("TA: Kicking out... student_id = %d.\n", studen_id_with_TA);
  Students[studen_id_with_TA] = PROGRAMMING; // done with TA, back go back to programming.
  studen_id_with_TA = -1;
  if (sem_post(&TA_office_sem) != 0) { // signal a waiting student that ur done with the TA.
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

/**
 * The thread will begin control in this function
 */
void *TA_thread_func(void *param)
{

  do {
    printf("TA: Napping...\n");
    rand_sleep(-1, 3);
    if(studen_id_with_TA != -1) {
      printf("TA: Teaching... student_id = %d.\n", studen_id_with_TA);
      rand_sleep(-1, 3);
      TA_kick_out_student ();
    }
  } while(1);

  pthread_exit(0);
}

void acquire_TA(int stud_id) {
  if (sem_trywait(&TA_lock_sem) != 0) { // acquire TA lock.
    printf("%s\n", strerror(errno));
    printf("*** %d: TA busy with other student! *** \n", stud_id);
    //assert(0);
    return;
  } else {
    printf("*** %d: acquired TA lock! *** \n", stud_id);
    assert(studen_id_with_TA == -1); // assert no one is currently with the TA.
    Students[stud_id] = WITH_TA;
    studen_id_with_TA = stud_id;
  }

  printf("*** %d: sitting with TA.\n", stud_id);
  if (sem_wait(&TA_office_sem) != 0) {
    printf("%s\n", strerror(errno));
    // EAGAIN = lock not avail.
    assert(0);
  }
  printf("*** %d: TA time is up. leaving office.\n", stud_id);

  //studen_id_with_TA = -1;
  //Students[stud_id] = PROGRAMMING; // done with TA, back go back to programming.
  if (sem_post(&TA_lock_sem) != 0) { // release TA lock.
    printf("%s\n", strerror(errno));
    assert(0);
  } else {
    printf("*** %d: released TA lock! *** \n", stud_id);
  }
}

int get_student_id(void) { // assign a unique integer to each student.
  int i = 0;
  pthread_t tid;

  for(i = 0; i < NUM_STUDENTS; i++) {
    tid = pthread_self();
    if(Stu_tid[i] == tid) {
      return i;
    }
  }

  assert(0);
}

/**
 * The thread will begin control in this function
 */
void *Student_thread_func(void *param)
{
  int student_id = get_student_id();

  do {

    rand_sleep(student_id, 3);
    acquire_TA(student_id);

  } while(1);

  pthread_exit(0);
}
