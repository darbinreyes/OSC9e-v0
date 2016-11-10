#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
  pid_t pid;

  printf("Hello, Chap.3 PP!\n");

  printf("Calling fork Yo!\n");

  pid = fork(); // recall, returns child PID to parent, 0 to child, < 0 on error.

  if(pid < 0) {
    printf("fork() error!\n");
    return 0;
  } else if(pid == 0) { // child process.
    printf("I am the child.\n");
    return 0; // terminate child so he becomes a zombie!
  } else { // parent process.
    printf("I am the parent. Child PID = %d. Sleeping for a bit.\n", pid);
    // problem specifies > 10 seconds for zombies lifetime.
    sleep(5);
    printf("Done sleeping.\n");
    wait(NULL);
  }

  return 0;
}