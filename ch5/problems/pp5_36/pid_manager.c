/**

**/

#include <stdio.h>
#include <assert.h>
#include <string.h> // strerr()
#include <errno.h> // errno
#include <pthread.h> // mutex_t

#include "pid_manager.h"

#define PID_IS_AVAILABLE   0
#define PID_IS_IN_USE      1
#define PID_STATE_BUF_SIZE (MAX_PID - MIN_PID + 1)

static char pid_alloc_state[PID_STATE_BUF_SIZE]; // TODO: Save mem. Use bits instead of bytes.
static int  next_free_pid_index = 0; // Always points to the next free PID in pid_alloc_state buff. if a free PID available.
static int  pid_in_use_count = 0; // Tracks the number of PIDs currently in use so we can detect when additional allocs are not possible.

// mutex for m.ex. access to these function.
static pthread_mutex_t mutex;

// PUBLIC functions

int free_map(void) {

  if(pthread_mutex_destroy(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  return 0;
}

// ret. 0 if successful, -1 on error.
int allocate_map(void) {

  if(pthread_mutex_init(&mutex, NULL) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
  // No-op since using static buffer for now.
  printf("allocate_map() called.\n");

  return 0;
}



int allocate_pid(unsigned long id) {
  // If a free PID is available, find the first free PID by linear search. Increment the in use cause.
  int iter_count, ret_pid;

  // main lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  assert(pid_in_use_count >= 0 && pid_in_use_count <= PID_STATE_BUF_SIZE); //  Validate in use range.

  if(pid_in_use_count >= PID_STATE_BUF_SIZE) {
    printf("#%lu: allocate_pid: No free PID's right now, sorry.\n", id);
    ret_pid = -1;
    goto Exit;
  }

  // Find the next free PID. Limit the search to the size of the state buff. to be safe.
  iter_count = 0;

  while(pid_alloc_state[next_free_pid_index] == PID_IS_IN_USE && iter_count < PID_STATE_BUF_SIZE) {
    printf("#%lu: allocate_pid: next_free_pid_index = %d. iter_count = %d.\n", id, next_free_pid_index, iter_count);
    iter_count++;
    next_free_pid_index++;
    next_free_pid_index %= PID_STATE_BUF_SIZE; // Loop/clock arithmetic
  }

  printf("#%lu: allocate_pid: Alloc. next_free_pid_index = %d. iter_count = %d.\n", id, next_free_pid_index, iter_count);

  // Sanity check. The loop should have found a free PID.
  assert(pid_alloc_state[next_free_pid_index] == PID_IS_AVAILABLE);

  pid_alloc_state[next_free_pid_index] = PID_IS_IN_USE; // Mark PID as in use

  pid_in_use_count++; // Update in use count.

  ret_pid = next_free_pid_index + MIN_PID; // Return PID, range adjusted. PID index 0 maps to MIN_PID. index 200 maps to MAX_PID

  next_free_pid_index++; // Update next free pid index.
  next_free_pid_index %= PID_STATE_BUF_SIZE;

  printf("#%lu: allocate_pid: returning pid = %d.\n", id, ret_pid);

Exit:
  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  return ret_pid;
}

void release_pid(unsigned long id, int pid) {
  // Mark the given PID as free. Decrement the in use count.

  // main lock
  if(pthread_mutex_lock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }

  printf("#%lu: release_pid: pid = %d.\n", id, pid);

  assert(pid_in_use_count > 0); // At least 1 PID must be in use.
  assert(pid_in_use_count <= PID_STATE_BUF_SIZE);

  assert(pid >= MIN_PID && pid <= MAX_PID); // valid date the PID value

  pid -= MIN_PID;

  printf("#%lu: release_pid: pid_index = %d.\n", id, pid);

  assert(pid_alloc_state[pid] == PID_IS_IN_USE); // The PID should be marked as in use if it is being released.

  pid_alloc_state[pid] = PID_IS_AVAILABLE;

  pid_in_use_count--; // A PID was returned. Update in use count.

  // main unlock
  if(pthread_mutex_unlock(&mutex) != 0) {
    printf("%s\n", strerror(errno));
    assert(0);
  }
  return;
}
