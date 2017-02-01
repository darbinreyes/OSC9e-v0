/**

**/

#include <stdio.h>
#include <assert.h>

#include "pid_manager.h"

#define PID_IS_AVAILABLE   0
#define PID_IS_IN_USE      1
#define PID_STATE_BUF_SIZE (MAX_PID - MIN_PID + 1)

static char pid_alloc_state[PID_STATE_BUF_SIZE]; // TODO: Save mem. Use bits instead of bytes.
static int  next_free_pid_index = 0; // Always points to the next free PID in pid_alloc_state buff. if a free PID available.
static int  pid_in_use_count = 0; // Tracks the number of PIDs currently in use so we can detect when additional allocs are not possible.

// PUBLIC functions


// ret. 0 if successful, -1 on error.
int allocate_map(void) {

  // No-op since using static buffer for now.

  return 0;
}

int allocate_pid(void) {
  // If a free PID is available, find the first free PID by linear search. Increment the in use cause.
  int iter_count, ret_pid;

  assert(pid_in_use_count >= 0 && pid_in_use_count <= PID_STATE_BUF_SIZE); //  Validate in use range.

  if(pid_in_use_count >= PID_STATE_BUF_SIZE) {
    printf("allocate_pid: No free PID's right now, sorry.\n");
    return -1;
  }

  // Find the next free PID. Limit the search to the size of the state buff. to be safe.
  iter_count = 0;

  while(pid_alloc_state[next_free_pid_index] == PID_IS_IN_USE && iter_count < PID_STATE_BUF_SIZE) {
    printf("allocate_pid: next_free_pid_index = %d. iter_count = %d.\n", next_free_pid_index, iter_count++);
    iter_count++;
    next_free_pid_index++;
    next_free_pid_index %= PID_STATE_BUF_SIZE; // Loop/clock arithmetic
  }

  printf("allocate_pid: FREE next_free_pid_index = %d. iter_count = %d.\n", next_free_pid_index, iter_count);

  // Sanity check. The loop should have found a free PID.
  assert(pid_alloc_state[next_free_pid_index] == PID_IS_AVAILABLE);

  pid_alloc_state[next_free_pid_index] = PID_IS_IN_USE; // Mark PID as in use

  pid_in_use_count++; // Update in use count.

  next_free_pid_index++; // Update next free pid index.
  next_free_pid_index %= PID_STATE_BUF_SIZE;


  ret_pid = next_free_pid_index - 1 + MIN_PID; // Return PID, range adjusted. PID index 0 maps to MIN_PID.

  printf("allocate_pid: ret_pid = %d.\n", ret_pid);

  return ret_pid;
}

void release_pid(int pid) {
  // Mark the given PID as free. Decrement the in use count.

  printf("release_pid: pid = %d.\n", pid);

  assert(pid_in_use_count > 0); // At least 1 PID must be in use.
  assert(pid_in_use_count <= PID_STATE_BUF_SIZE);

  assert(pid >= MIN_PID && pid <= MAX_PID); // valid date the PID value

  pid -= MIN_PID;

  printf("release_pid: pid_index = %d.\n", pid);

  assert(pid_alloc_state[pid] == PID_IS_IN_USE); // The PID should be marked as in use if it is being released.

  pid_alloc_state[pid] = PID_IS_AVAILABLE;

  pid_in_use_count--; // A PID was returned. Update in use count.

  return;
}
