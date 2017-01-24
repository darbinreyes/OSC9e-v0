/**
### Given main outline
include "buffer.h"
int main(int argc, char *argv[]) { // 1. Get command line arguments argv[1],argv[2],argv[3] //
// 2. Initialize buffer //
// 3. Create producer thread(s) //
// 4. Create consumer thread(s) //
// 5. Sleep //
// 6. Exit //
}

### Given buffer header.
// buffer.h //

typedef int buffer item;
#define BUFFER SIZE 5

### Given buffer outline

#include "buffer.h"

// the buffer //
buffer item buffer[BUFFER SIZE];

int insert item(buffer item item) {
  //  insert item into buffer
  //  return 0 if successful, otherwise
  //  return -1 indicating an error condition
}

int remove item(buffer item *item) {
  //  remove an object from buffer
  //  placing it in item
  //  return 0 if successful, otherwise
  //  return -1 indicating an error condition /
}

### Given P/C thread outline

#include <stdlib.h> // required for rand() //
#include "buffer.h"

void *producer(void *param) {
  buffer item item;
  while (true) { // sleep for a random period of time //
    sleep(...);
    // generate a random number //
    item = rand();
    if (insert item(item))
      fprintf("report error condition");
    else
      printf("producer produced %d\n",item);
  }
}

void *consumer(void *param) {
  buffer item item;
  while (true) { // sleep for a random period of time //
    sleep(...);
    if (remove item(&item))
      fprintf("report error condition");
    else
      printf("consumer consumed %d\n",item);
  }
}

**/

#include <pthread.h>
#include <stdio.h>

int sum; /* this data is shared by the thread(s) */

void *runner(void *param); /* the thread */

int main(int argc, char *argv[])
{
pthread_t tid; /* the thread identifier */
pthread_attr_t attr; /* set of attributes for the thread */

if (argc != 2) {
  fprintf(stderr,"usage: a.out <integer value>\n");
  /*exit(1);*/
  return -1;
}

if (atoi(argv[1]) < 0) {
  fprintf(stderr,"Argument %d must be non-negative\n",atoi(argv[1]));
  /*exit(1);*/
  return -1;
}

/* get the default attributes */
pthread_attr_init(&attr);

/* create the thread */
pthread_create(&tid,&attr,runner,argv[1]);

/* now wait for the thread to exit */
pthread_join(tid,NULL);

printf("sum = %d\n",sum);
}

/**
 * The thread will begin control in this function
 */
void *runner(void *param)
{
int i, upper = atoi(param);
sum = 0;

  if (upper > 0) {
    for (i = 1; i <= upper; i++)
      sum += i;
  }

  pthread_exit(0);
}
