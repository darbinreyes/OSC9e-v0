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

#define MAX_STUDENT_PROG_TIME 7 /// Maximum number of time units a student will spend programming.
#define MAX_WITH_TA_TIME 3 /// Maximum number of time units the TA will spend with a student before kicking him out of his office.

/**
 Student and TA state definitions.
**/
#define NUM_STUDENTS  5
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
static sem_t TA_lock_sem; // ONLY STUDENTS CAN WAIT/SIGNAL. TA SHOULD NEVER TOUCH THIS SEM.

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
static sem_t TA_office_chair_sem; // ONLY STUDENTS CAN WAIT. ONLY TA CAN SIGNAL.

/**
  Binary semaphore.

  Purpose: Used by the TA to take a nap. Used by the next student to signal the TA
  that he wants help, or to wake up the TA if he is currently napping.

  Usage: TA thread wait()'s on this semaphore when no students are requesting
  his help. Student thread looking for the TA's help signal()/post()'s this
  semaphore to wake the TA if he is currently napping. If the TA isn't napping
  then the TA must have just kicked out a student from his office to let the
  next student in. The next student signal()/post()'s this semaphore to inform
  the TA of his student_ID before he may enter the office and sit in the office
  chair. // NOTE: I this allows the TA thread to be the only one in control of
  state changes from PROGRAMMING to WITH TA. Seems safer to me in terms of

  correctness.
  // ONLY STUDENTS CAN SIGNAL. ONLY TA CAN WAIT.

  Initialized to 0.

**/
static sem_t TA_help_request_sem;  // TA naps inside this sem.
static sem_t TA_help_request_sem1; // Wait for student to sit outside. Students signal this sem. after changing their state to WAITING_FOR_TA
static sem_t TA_help_request_sem2; // Wait for student to set Student_with_TA. Students signal this sem. after releasing the TA lock.
static sem_t TA_help_request_sem3; // Students signal this sem. after releasing the TA lock.

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
static int student_with_TA; // written to by Students, read by TA. TA SHOULD NEVE WRITE TO THIS.

/*******************Waiting Chairs Related Definitions ********************/

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
static int free_chairs_count; // READ/WRITE FOR STUDENTS. READ ONLY FOR TA.GUARDED BY Wait_chairs_sem FOR BOTH READS AND WRITES.

/**

  TODO: comments.

  Chairs for a student to wait for the TA outside his office.
**/
static sem_t Chair_outside_sem; // Init. to 0. ONLY STUDENTS WAIT IN THESE SEM.S. ONLY THE TA SIGNALS THEM.
static sem_t Wait_chairs_sem; // Init. to NUM_WAIT_CHAIRS.

/**
  Initialize this program's state:
  -TA is free.
  -Students programming.
  -sem. initializations.

**/
static void init_state(void) {
  int i = 0;

  student_with_TA = -1; // no one has TA.
  Ta = NAPPING;

  for(i = 0; i < NUM_STUDENTS; i++) {
    Students[i++] = PROGRAMMING;
  }

  /* init. semaphores */

  // create an "unnamed" semaphore, flags = 0, init. value = 1.
  if (sem_init(&TA_lock_sem, 0, 1) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // init to 0.
  if (sem_init(&TA_office_chair_sem, 0, 0) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // init to 0.
  if (sem_init(&TA_help_request_sem, 0, 0) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // init to 0.
  if (sem_init(&TA_help_request_sem1, 0, 0) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
  // init to 0.
  if (sem_init(&TA_help_request_sem2, 0, 0) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // init to 0.
  if (sem_init(&TA_help_request_sem3, 0, 0) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  free_chairs_count = NUM_TA_CHAIRS;

  // init. value = NUM_TA_CHAIRS.
  if (sem_init(&Wait_chairs_sem, 0, NUM_TA_CHAIRS) == -1) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

}

/**

  Cleanup this program's state before terminating.
  -destroys all semaphores.

**/
static void cleanup_state(void) {
  int i;

  if (sem_destroy(&TA_lock_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_office_chair_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_help_request_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_help_request_sem1) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_help_request_sem2) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&TA_help_request_sem3) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&Wait_chairs_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_destroy(&Chair_outside_sem) != 0) {
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

  printf("Creating TA thread...\n");

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

  Sleep for a random amount of time.

**/
void rand_sleep(int student_id, int max) { // student_id==-1 means TA thread is the caller.
  int t;

  assert(student_id >= -1 && max > 0);

#ifndef SLEEP_TIME_DISABLED

  // sleep rand. amnt. of time.
  t = rand() % max + 1;
  #ifdef MICRO_SECONDS_TIME_UNITS
    printf("#%d: sleeping %d usecs.\n", student_id, t);
    usleep(t);
  #else
    printf("#%d: sleeping %d secs.\n", student_id, t);
    sleep(t); // default
  #endif
  printf("#%d: Done sleeping.\n", student_id);

#else // sleep calls disabled.

  static char no_sleep;
  if(!no_sleep) {
    no_sleep = 1;
    printf("#%d: Sleeping disabled.\n", student_id);
  }

#endif

}

/**

  Student only function.

**/
void acquire_TA(int student_id) {

  assert(pthread_self() != TA_tid);

  /**

    Student: Try to get some help from the TA. If the student successfully
    acquires the lock, he will get a chance to spend time with the TA next.
    Otherwise, he returns without the lock from this function, in this
    case the student checks if a waiting chair is available.

    FYI: sem_trywait() is the version of sem_wait() that returns immediately from the call if
    the lock is not available, vs. sem_wait() which will spin or sleep the calling
    thread until it is signaled.

  **/
  if (sem_wait(&TA_lock_sem) != 0) {
    printf("%s\n", strerror(errno));
    //printf("#%d: Shit, the TA busy with another student.\n", student_id);

    //printf("#%d: I'll try again later.\n", student_id);
    assert(0);
    //return; // Sorry... Yah bitch.
  }

  printf("#%d: I acquired the TA lock, yay!\n", student_id);
  fprintf(stderr, "#%d: got TA lock.\n", student_id);

  student_with_TA = student_id;

  // Tell the TA you have set the "student_with_TA" var. and will wait in the TA_office_chair_sem until he kicks you out.
  if (sem_post(&TA_help_request_sem2) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("#%d: Sitting down in TA's office.\n", student_id);

  if (sem_wait(&TA_office_chair_sem) != 0) { // wait here until TA kicks you out.
    printf("%s\n", strerror(errno));
    // FYI: errno=EAGAIN = lock not avail.
    assert(0);
  }

  student_with_TA = -1; // No student is with the TA anymore.

  printf("#%d: Leaving TA's office.\n", student_id);

  // release TA lock.
  if (sem_post(&TA_lock_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("#%d: I released the TA lock. Peace out.\n", student_id);
  if (sem_post(&TA_help_request_sem3) != 0) { // Tell TA you released him. So he can help next student.
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

/**

  Assign a unique integer to each student in the range of 0 to N-1,
  where N == Number of students. The assignment is based to the thread's ID.

  This function should only be called by a student thread. // TODO: move to separate file.

**/
static int get_student_id(void) {
  int i = 0;
  pthread_t tid;

  tid = pthread_self();

  assert(tid != TA_tid);

  for(i = 0; i < NUM_STUDENTS; i++) {
    if(Stu_tid[i] == tid) {
      return i;
    }
  }

  assert(0);
}

/**

  Used by a student to acquire a chair and wait for the TA's attention.

**/
static void acquire_chair(int student_id) {

  assert(pthread_self() != TA_tid);

  // acquire chair.
  if (sem_trywait(&Wait_chairs_sem) != 0) {
    printf("%s\n", strerror(errno));
    printf("#%d: No chair for me. Num. free chairs = %d.\n", student_id, free_chairs_count);
    return;
  }

  printf("#%d: I got a chair. Num. free chairs = %d.\n", student_id, free_chairs_count);

  if (sem_post(&TA_help_request_sem) != 0) { // Tell TA ur waiting, so don't take a nap.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_post(&TA_help_request_sem1) != 0) { // Tell TA ur about to sit down and wait outside.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  free_chairs_count--;
  Students[student_id] = WAITING_FOR_TA;
  if (sem_wait(&Chair_outside_sem) != 0) { // Wait outside.
    printf("%s\n", strerror(errno));
    printf("#%d:\n", student_id);
    assert(0);
  }
  free_chairs_count++;

  // release chair.
  if (sem_post(&Wait_chairs_sem) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

static void student_programming(int student_id) {
  printf("#%d: Programming...Come with me if you want to live.\n", student_id);
  rand_sleep(student_id, MAX_STUDENT_PROG_TIME); // Spend some time programming.
  printf("#%d: Fuk. I need the TA's help on this shit.\n", student_id);
}
/**

 Entry point for a student thread.

**/
void *Student_thread_func(void *param)
{
  int student_id = get_student_id(); // Find out which student # you are.

  do {

    student_programming(student_id);

    acquire_chair(student_id);

    if(Students[student_id] == WAITING_FOR_TA) {
      printf("#%d: Its my turn with the TA bitches.\n", student_id);
      acquire_TA(student_id); // Try getting some help from the TA
    } else {
      printf("#%d: I'll be back...My CPU is a neural net processor. A learning computer.\n", student_id);
    }

  } while(1);

  pthread_exit(0);
}
/**

  Signal the next student waiting in a chair that it is his turn with the TA
  now.

**/
static void check_for_waiting_students(void) {

  assert(pthread_self() == TA_tid);

  printf("TA: Calling in next waiting student.\n");

  if (sem_wait(&TA_help_request_sem1) != 0) {  // Wait for student to sit outside.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if (sem_post(&Chair_outside_sem) != 0) { // Call next student into office.
    printf("%s\n", strerror(errno));
    assert(0);
  }
}

/**



  This function should only be called by the TA thread.

**/
static void TA_help_next_student (void) {

  assert(pthread_self() == TA_tid);

  printf("TA: Hey, how can I help? Please sit down.\n");

  if (sem_wait(&TA_help_request_sem2) != 0) {  // Wait for student to set student_with_TA var.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  assert(student_with_TA >= 0);

  printf("TA: #%d, now I can help you.\n", student_with_TA);

  // State of student requesting help should be programming or waiting for TA if the TA is calling him into the office.
  assert(Students[student_with_TA] != WITH_TA);
  Students[student_with_TA] = WITH_TA;

  printf("TA: Teaching student #%d.\n", student_with_TA);
  rand_sleep(-1, MAX_WITH_TA_TIME); // Help the student for a rand. amount of time, then kick him out. -1 identifies the TA.
  printf("TA: Get the fuck out, #%d. Yah bitch...\n", student_with_TA);
  Students[student_with_TA] = PROGRAMMING;

  if (sem_post(&TA_office_chair_sem) != 0) { // Tell student to leave office.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("TA: Waiting for student to release me.\n");

  if (sem_wait(&TA_help_request_sem3) != 0) { // Wait for student to release TA lock.
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("TA: I'm free to help the next student.\n");

}

static void TA_take_nap(void) {
  assert(pthread_self() == TA_tid);
  Ta = NAPPING;
  printf("TA: Napping...\n");
  if (sem_wait(&TA_help_request_sem) != 0) { // This is where the TA naps.
    printf("%s\n", strerror(errno));
    assert(0);
  }
  Ta = TEACHING;
  printf("TA: A student woke me up. Fukin mofos.\n");
}

/**

  TA thread entry point.

**/
void *TA_thread_func(void *param)
{

  do { // nap->help student->kick out student->nap->//repeat
    // TA should nap until I'm awakened by a student looking for help.
    TA_take_nap();
    // Wait for the student to be sitting down out side, the signal()/sem_post() the chair.
    check_for_waiting_students();
    // Awakened by a student. Let him in the office and wait for him to sit down.
    TA_help_next_student();
    // TODO: Instead of going back to napping unconditionally. Instead, check if a student is waiting for help first, and help him. If no one is waiting, then nap.
  } while(1);

  pthread_exit(0);
}