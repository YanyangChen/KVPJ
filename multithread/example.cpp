#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#define NUM_THREADS 3
int system(const char *command);
void *PrintHello0(void *threadid) {
	while(1)
	{
printf("\n%d: Hello David!\n", threadid);
}
pthread_exit(NULL);

}

void *PrintHello1(void *threadid) {
	while(1){
printf("\n%d: Hello Zerui!\n", threadid);
}

pthread_exit(NULL);

}

void *PrintHello2(void *threadid) {
	while(1){
printf("\n%d: Hello Billy!\n", threadid);
}

pthread_exit(NULL);

}


int main (int argc, char *argv[])
{
pthread_t threads[NUM_THREADS];
system("espeak 'activated'");
pthread_create(&threads[0], NULL, PrintHello0, (void *) 0);
pthread_create(&threads[1], NULL, PrintHello1, (void *) 1);
pthread_create(&threads[2], NULL, PrintHello2, (void *) 2);

/*
 * 
 * 
 *        int pthread_create(pthread_t *restrict thread,
              const pthread_attr_t *restrict attr,
              void *(*start_routine)(void*),
              void *restrict arg);*/

pthread_exit(NULL);
	}
