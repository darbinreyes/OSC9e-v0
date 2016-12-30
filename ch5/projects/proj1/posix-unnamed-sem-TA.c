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
static pthread_t      TA_tid; /* The TA thread identifier */
static pthread_attr_t TA_attr; /* Set of attributes for the thread */

// Student thread declarations.
static pthread_t      Stu_tid[NUM_STUDENTS]; /* The Student thread(s) identifier */
static pthread_attr_t Stu_attr[NUM_STUDENTS]; /* Set of attributes for the thread */

void *TA_thread_func(void *param); // TA thread entry point function.
void *Student_thread_func(void *param); // Student thread entry point function.

/** "Unnamed" semaphores declarations **/

/**

 Binary semaphore.

 Purpose: Used to ensure mutually exclusive access to the TA with respect to students.

 Lock state meaning WRT student:

 Lock available: The TA is currently free to service a student.
 Lock unavailable: The TA is currently servicing another student.

 Initialized to 1. 1 means is TA available.

**/
static sem_t TA_lock_sem;

/**
  Binary semaphore.

  Purpose: Represents the single chair in the TA's office.

  Purpose: A student waits in this semaphore until the TA signals it. The TA's signal
  tells the student that his time is up. He's must leave the office chair so
  that another student can have a chance.

  Usage: A student sits in the chair by calling wait()/sem_wait() on this sem. This causes the
  student thread to be blocked at this point until the TA kicks him out of
  the office by calling signal()/sem_post().

  This chair is guarded by the TA_sem_lock, i.e. only the lock holder may sit
  in this chair.

  Initialized to 0.

**/
static sem_t TA_office_sem;

/**

  The current number of FREE/available chairs outside of the TA's office.

  Purpose: *When the TA is busy*, the chairs effectively provide a finite length
  queue of students wishing to have access to the TA (the TA himself and the
  chair in his office).

  If a student wishing for time with the TA finds that the TA is busy with
  another student, then he checks if there is a chair free/available for him to
  sit and wait for his turn with the TA. If no chair is free/available, he must
  retry at a later time.

  Initial value is an argument to this program. It may be any non-negative
  integer. The OSC problem statement specifies 3 chairs.

  0 means no chairs are free/available. 1 means that one chair is available etc.

**/
static int free_chairs_count;

/**

  The index/"Student ID" of student that is currently with the TA.
  Each student thread is uniquely identified by an integer from
  0 to N-1, where N = the total number of student threads.
  A value of -1 indicates that no student is currently with the TA.

  Purpose: Provides a mechanism for the TA to set a student's state from
  "with TA" to "programming" once that students time with the TA is up.
  Also, provides very basic test to check that all students get a chance
  with the TA by printing the student's ID when he gets time with the TA (i.e.
  acquires the TA lock).

  Initialized to -1. -1 indicates that no student is currently with the TA.

**/
static int student_with_TA;

/**
  Initialize this program's state:
  -TA is free.
  -Students programming.
  -sem. initializations.

**/
void init_state(void) {
  int i = 0;

  student_with_TA = -1; // no one has TA.
  Ta = NAPPING;

  for(i = 0; i < NUM_STUDENTS; i++) {
    Students[i++] = PROGRAMMING;
  }

  free_chairs_count = NUM_TA_CHAIRS;

  /* init. semaphores */

  // create an "unnamed" semaphore, flags = 0, init. value = 1.
  if (sem_init(&TA_lock_sem, 0, 1) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // init to 0.
  if (sem_init(&TA_office_sem, 0, 0) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

/**

  Cleanup this program's state before terminating.
  -destroys all semaphores.

**/
void cleanup_state(void) {

  if (sem_destroy(&TA_lock_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_office_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

/**

  main.

  -create TA thread.
  -create all student threads.

**/
int main(void)
{
  int i;

  init_state();

  printf("Creating TA thread...\n"); // ? use kprint?

  // Create and start the TA thread
  pthread_attr_init(&TA_attr); // Get the default thread attributes
  pthread_create(&TA_tid, &TA_attr, TA_thread_func, NULL);

  // Create and start the Student thread(s)
  for(i = 0; i < NUM_STUDENTS; i++) {
    printf("Student %d: Creating thread...\n", i);
    pthread_attr_init(&Stu_attr[i]);
    pthread_create(&Stu_tid[i], &Stu_attr[i], Student_thread_func, NULL);
  }

  // Wait for the TA thread to exit
  pthread_join(TA_tid, NULL);

  // Wait for each Student thread to exit
  for(i = 0; i < NUM_STUDENTS; i++) {
    pthread_join(Stu_tid[i], NULL);
  }

  cleanup_state();

  return 0;
}

/**

  Sleep for a random number of seconds.

**/
void rand_sleep(int student_id, int max) { // student_id==-1 means TA thread is the caller.
  int t;

  assert(student_id >= -1 && max > 0);


  t = rand() % max + 1;
  printf("#%d: sleeping %d seconds.\n", student_id, t);
  sleep(t);
  printf("#%d: Done sleeping.\n", student_id);
}

void acquire_TA(int stud_id) {
  if (sem_trywait(&TA_lock_sem) != 0) { // acquire TA lock.
    printf("%s\n", strerror(errno));
    printf("*** %d: TA busy with other student! *** \n", stud_id);
    //assert(0);
    return;
  } else {
    printf("*** %d: acquired TA lock! *** \n", stud_id);
    assert(student_with_TA == -1); // assert no one is currently with the TA.
    Students[stud_id] = WITH_TA;
    student_with_TA = stud_id;
  }

  printf("*** %d: sitting with TA.\n", stud_id);
  if (sem_wait(&TA_office_sem) != 0) {
    printf("%s\n", strerror(errno));
    // EAGAIN = lock not avail.
    assert(0);
  }
  printf("*** %d: TA time is up. leaving office.\n", stud_id);

  //student_with_TA = -1;
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

/**

  TA: Politely inform the student that he must leave your office now. // fairness-improvement-idea-only kick out the student if someone else is waiting.

  This function is private to the TA. i.e. Only the TA only invokes this function. **

  -Set student's state to Programming.
  -signal()/sem_post() the TA office chair semaphore. This tells the student
  that he is being kicked out of the TA office chair by waking/unblocking the
  student's thread.

**/
static void TA_kick_out_student (void) {
  printf("TA: Get the fuck out, #%d. Yah bitch...\n", student_with_TA);
  assert(Students[student_with_TA] == WITH_TA);
  // Student is done with TA, go back to programming.
  Students[student_with_TA] = PROGRAMMING;
  student_with_TA = -1;
  // Signal the student that his time is up and must leave the TA office char.
  if (sem_post(&TA_office_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

/**

  TA thread entry point.

**/
void *TA_thread_func(void *param)
{

  do {

    // Nap.
    printf("TA: Napping...\n");
    rand_sleep(-1, 3);

    // Teach, if a student is waiting, else nap again.
    if(student_with_TA != -1) {
      printf("TA: Teaching student #%d.\n", student_with_TA);
      // Teaching for a lil time, then kick out the student.
      rand_sleep(-1, 3);
      TA_kick_out_student ();
    }
  } while(1);

  pthread_exit(0);
}