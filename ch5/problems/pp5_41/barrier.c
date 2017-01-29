/**

  Barrier implementation.

**/

#include <pthread.h>
#include <stdio.h> // printf
//#include <stdlib.h> // rand()
//#include <semaphore.h> // sem_t
#include <errno.h> // errno
#include <assert.h>

int enter_barrier(int id, pthread_mutex_t *b_mtx, pthread_cond_t *b_cond_var, int *b_counter, const int thresh) {
  assert(b_mtx);
  assert(b_cond_var);
  assert(b_counter);
  /** From Galvin

  pthread_mutex_lock(&mutex);
  a = b;
  pthread_cond_signal(&cond var);
  pthread_mutex_unlock(&mutex);

  **/

  // main lock
  if(pthread_mutex_lock(b_mtx) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("#%d: I'm inside bro. BEFORE inc. b_counter = %d.\n", id, *b_counter);

  *b_counter += 1;
  *b_counter %= (thresh + 1);

  printf("#%d: I'm inside bro. AFTER inc. b_counter = %d.\n", id, *b_counter);

  // cond. var. signal.
  if (pthread_cond_signal(b_cond_var) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  // main unlock
  if(pthread_mutex_unlock(b_mtx) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  return 0;
}



int exit_barrier(int id, pthread_mutex_t *b_mtx, pthread_cond_t *b_cond_var, int *b_counter, const int thresh) {
  assert(b_mtx);
  assert(b_cond_var);
  assert(b_counter);
  /// WAIT for the barrier to open. ///

  // main lock
  if(pthread_mutex_lock(b_mtx) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("#%d: I'm at the barrier wall.\n", id);


  while (*b_counter < thresh) {
    printf("#%d: Waiting for barrier to open... %d < %d.\n", id, *b_counter, thresh);
    // cond. var. wait.
    if (pthread_cond_wait(b_cond_var, b_mtx) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }

  }

  printf("#%d: THE BARRIER IS DOWN!\n", id);

  // main unlock
  if(pthread_mutex_unlock(b_mtx) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  /// The last T does the first signal, then all subsequent signals the guy behind him.

  // main lock
  if(pthread_mutex_lock(b_mtx) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  if(*b_counter >= thresh) {
    printf("#%d: No child left behind! signal for that last bro. b_counter = %d.\n", id, *b_counter);
    *b_counter += 1;
    if(pthread_cond_signal(b_cond_var) != 0) {
      printf("%s\n", strerror(errno));
      assert(0);
    }
  }

  // main unlock
  if(pthread_mutex_unlock(b_mtx) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  return 0;
}