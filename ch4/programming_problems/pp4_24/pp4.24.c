/**

# Programming Problem 4.24.

## Start: From the Book

> Write a multithreaded program that outputs prime numbers. This program should
> work as follows: The user will run the program and will enter a number on the
> command line. The program will then create a separate thread that outputs all
> the prime numbers less than or equal to the number entered by the user.

## End


# Plan:
1. Implement int is_prime(int n); // return 1 if n is prime. 0 otherwise.
2. Add code fetch first int arg. prime_upper_bound. convert to int using atoi()
3. Create empty worker thread. Pass arg. int to thread.
4. implement void print_primes(int upper_bound); // for(i=2; i <= prime_upper_bound; i++) // recall even ints are never prime. + to implement is_prime(n); we only need to test for clean division from 2 to sqrt(n)
5. In worker thread call print_primes.


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
pthread_attr_init(&attr); // FYI: Missing call to pthread_attr_destroy()? Ref: chegg.com + man page.

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
