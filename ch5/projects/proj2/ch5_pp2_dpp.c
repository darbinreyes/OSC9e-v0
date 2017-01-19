/**

 Solution to DPP using Pthreads mutex's and condition vars.

 Each P will runs as sep. thread. 0..4.
 P's alternate between THINKING, EATING. // HUNGRY too. Which means he would like to eat.
 Simulate state by rand. sleep time between 1 and 3 secs.

""" From the book """

Condition variables in Pthreads use the pthread cond t data type and
are initialized using the pthread cond init() function. The following code
creates and initializes a condition variable as well as its associated mutex lock:

pthread mutex t mutex;
pthread cond t cond var;
pthread mutex init(&mutex,NULL);
pthread cond init(&cond var,NULL);


The pthread cond wait() function is used for waiting on a condition
variable. The following code illustrates how a thread can wait for the condition
a == b to become true using a Pthread condition variable:


pthread mutex lock(&mutex);
while (a != b)
  pthread cond wait(&mutex, &cond var);
pthread mutex unlock(&mutex);


The mutex lock associated with the condition variable must be locked
before the pthread cond wait() function is called, since it is used to protect
the data in the conditional clause from a possible race condition. Once this
lock is acquired, the thread can check the condition. If the condition is not true,
the thread then invokes pthread cond wait(), passing the mutex lock and
the condition variable as parameters. Calling pthread cond wait() releases
the mutex lock, thereby allowing another thread to access the shared data and
possibly update its value so that the condition clause evaluates to true. (To
protect against program errors, it is important to place the conditional clause
within a loop so that the condition is rechecked after being signaled.)
A thread that modifies the shared data can invoke the
pthread cond signal() function, thereby signaling one thread waiting
on the condition variable. This is illustrated below:


pthread mutex lock(&mutex);
a = b;
pthread cond signal(&cond var);
pthread mutex unlock(&mutex);


It is important to note that the call to pthread cond signal() does not
release the mutex lock. It is the subsequent call to pthread mutex unlock()
that releases the mutex. Once the mutex lock is released, the signaled thread
becomes the owner of the mutex lock and returns control from the call to
pthread cond wait().

""" """

**/

/**

  Invoked by a P wishing to eat.

**/
static void pickup_forks(int philosopher_number) {
  // TODO
  return;
}

/**

  Invoked by a P when finished eating.

**/
static void return_forks(int philosopher_number) {
  // TODO
  return;
}