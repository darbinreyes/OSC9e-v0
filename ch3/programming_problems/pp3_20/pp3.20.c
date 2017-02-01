/**

  Programming Problem 3.20.
  // COMPILATION: "cc pp3.20.c pid_manager.c"
**/

#include <stdio.h>

#include "pid_manager.h"

int main(void) {

  int pid0, pid1, pid2;

  // // TEST: Alloc then in same order release.
  // pid0 = allocate_pid();
  // pid1 = allocate_pid();
  // pid2 = allocate_pid();

  // release_pid(pid0);
  // release_pid(pid1);
  // release_pid(pid2);

  // // TEST: Alloc then reversed order release.
  // pid0 = allocate_pid();
  // pid1 = allocate_pid();
  // pid2 = allocate_pid();

  // release_pid(pid2);
  // release_pid(pid1);
  // release_pid(pid0);

  // // TEST: Alloc then mixed up order release.
  // pid0 = allocate_pid();
  // pid1 = allocate_pid();
  // pid2 = allocate_pid();

  // release_pid(pid1);
  // release_pid(pid2);
  // release_pid(pid0);

  // TEST: All alloc.ed PID case.
  // while((pid0 = allocate_pid()) != -1)
  //   ;


    // TEST: All alloc.ed PID case. + 1 release.
  // while((pid0 = allocate_pid()) != -1)
  //   ;
  // release_pid(500);

  // TEST: All alloc.ed PID case. + 1 release + 1 alloc.
  while((pid0 = allocate_pid()) != -1)
    ;
  release_pid(500);
  pid0 = allocate_pid();

  // TEST: All alloc.ed PID case. 2x(+ 1 release + 1 alloc.)
  while((pid0 = allocate_pid()) != -1)
    ;
  release_pid(500);
  pid0 = allocate_pid();

  release_pid(400);
  pid1 = allocate_pid();


  return 0;
}