/**

Q:
a. Identify the data involved in the race condition.
b. Identify the location (or locations) in the code where the race
condition occurs.
c. Using a semaphore or mutex lock, fix the race condition. It is
permissible to modify the decrease count() function so that the
calling process is blocked until sufficient resources are available.

* ANS:
    * a) avail_resources is inc/dec.’d concurrently, just like in the producer consumer example described by Galvin at the beginning of chap5. Without a mechanism to ensure atomic execution of the read/writes to avail_resources int. arbitrary interleaving of the underlying machine instructions will result in incorrect behaviour. e.g. avail_resources = 5. P0 calls dec. count(5). compare 5 < 5 = FALSE. context switch to P1 calls dec. count(5) to completion. avail_resources is now 0. context sw back to P0, the comparison was already made and the result was false so avail_resources will end up being -5 which should never occur. There cannot be negative licenses in this scenario.
    * b) all lines with “avail_resources”.
    * c) Yes we can block a P until resources become available. This is the default behaviour of sem_t’s in pthreads using sem_wait(s) + sem_post(t), s should be init. to max resources.

**/

#define MAX_RESOURCES 5

int available_resources = MAX_RESOURCES;

/* decrease available resources by count resources */
/* return 0 if sufficient resources available, */
/* otherwise return -1 */
int decrease_count(int count) {
  if (available_resources < count)
    return -1;
  else {
    available_resources -= count;
    return 0;
  }
}

/* increase available resources by count */
int increase_count(int count) {
  available_resources += count;
  return 0;
}