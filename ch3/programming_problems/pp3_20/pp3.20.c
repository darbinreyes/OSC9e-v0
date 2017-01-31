/**

  Programming Problem 3.20.
  // COMPILATION: "cc pp3.20.c pid_manager.c"
**/

#include <stdio.h>

#include "pid_manager.h"

int main(void) {

  int pid0, pid1, pid2;

  pid0 = allocate_pid();
  pid1 = allocate_pid();
  pid2 = allocate_pid();

  release_pid(pid2);
  release_pid(pid1);
  release_pid(pid0);

  pid0 = allocate_pid();
  pid1 = allocate_pid();
  pid2 = allocate_pid();

  release_pid(pid2);
  release_pid(pid1);
  release_pid(pid0);

  return 0;
}