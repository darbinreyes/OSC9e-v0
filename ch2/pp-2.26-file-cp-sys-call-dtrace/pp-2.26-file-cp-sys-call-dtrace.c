/**

In Section 2.3,
we described a program that
copies the contents of one file to a destination file.
This program works by first prompting the user for the name of the source and destination files.


Write this program using either the Windows or

POSIX API.

Be sure to include all necessary error checking, including ensuring that the source file exists.

Once you have correctly designed and tested the program,

if you used a system that supports it, run the program using a utility that traces system calls.
Linux systems provide the strace utility, and Solaris and

Mac OS X systems use the dtrace command.


As Windows systems do not provide such features, you will have to trace through the Windows version of this program using a debugger.


**/

#include <stdio.h>
// #include "exampleProvider.h"

int main(int argc, char *argv[]) {
  //EXAMPLE_MY_HELLO_WORLD();
  printf("Hello, world!\n");
  return 0;
}