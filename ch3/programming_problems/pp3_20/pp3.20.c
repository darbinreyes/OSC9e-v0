/**

# Programming Problem 3.20.

## Start: From the Book

> Programming Exercise 3.20 required you to design a PID manager that allocated a
> unique process identifier to each process. Exercise 4.20 required you to modify
> your solution to Exercise 3.20 by writing a program that created a number of
> threads that requested and released process identifiers. Now modify your
> solution to Exercise 4.20 by ensuring that the data structure used to represent
> the availability of process identifiers is safe from race conditions. Use
> Pthreads mutex locks, described in Section 5.9.4.

## End

**/

#include <stdio.h>

#include "pid_manager.h"
// Prob.3.20 main
int main(void) {

  int pid0, pid1, pid2;

  // // TEST: Alloc then in same order release. // TODO move tests to seatest test project. DONT BE A NOOB!
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
  // while((pid0 = allocate_pid()) != -1)
  //   ;
  // release_pid(500);
  // pid0 = allocate_pid();

  // TEST: All alloc.ed PID case. 2x(+ 1 release + 1 alloc.)
  while((pid0 = allocate_pid()) != -1)
    ;
  release_pid(500);
  pid0 = allocate_pid();

  release_pid(400);
  pid1 = allocate_pid();


  return 0;
}
