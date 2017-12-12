#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "hybrid_lock.c"
long g_count = 0;

struct hybrid_lock g_lock;

void *thread_func(void *arg)
{
	long i,j,k, count = (long)arg;
	long long l;

	/*
	Increase the global variable, g_count
	This code should be protected by
	some locks, e.g. spin lock, and the lock that
	you implemented for assignment
	because g_count is shared by other threads
	*/

	for(i = 0; i < count; i++){
		/*Critical Section*/
		//The purpose of this code is to occupy cpu for a long time
		hybrid_lock_lock(&g_lock);
		for(j = 0; j < 100000; j++){
			for(k = 0; k < 3000; k++){
				l+= j * k;
			}
		}
		g_count++;
		hybrid_lock_unlock(&g_lock);
		/*****************/
	}
}

int main(int argc, char *argv[])
{
	pthread_t *tid;
	long i, thread_count, value;
	int rc;

	/*
	Check that the program has three arguemtns
	If the number of arguements is not 3, terminate the process
	*/

	if(argc != 3){
		fprintf(stderr, "usage: %s <thread count> <value>\n", argv[0]);
		exit(0);
	}

	/*
	Get the values of the each arguements as a long type.
	It is better that use long type instead of int type,
	becuase sizeof(long) is smae with sizeof(void*)
	It will be better when we pass argument to thread
	that will be created by this thread
	*/

	thread_count = atol(argv[1]);
	value = atol(argv[2]);

	/*
	Create arraya to get tids of each threads that will
	be breated by this thread
	*/

	tid = (pthread_t*)malloc(sizeof(pthread_t)*thread_count);
	if(!tid){
		fprintf(stderr, "calloc() error\n");
		exit(0);
	}

	/*
	Create a thread by the thread_count value received as
	an argument. Each thread will increase g_count for 
	value times
	*/

	hybrid_lock_init(&g_lock);

	for (i = 0; i < thread_count; i++){
		rc = pthread_create(&tid[i], NULL, thread_func, (void*)value);
		if(rc){
			fprintf(stderr, "pthread_create() error\n");
			free(tid);
			hybrid_lock_destroy(&g_lock);
			exit(0);
		}
	}

	/*
	Wait until all the threads you create above are finished
	*/

	for(i= 0;  i < thread_count ; i++){
		rc = pthread_join(tid[i], NULL);
		if(rc){
			fprintf(stderr, "pthread_join() error\n");
			free(tid);
			hybrid_lock_destroy(&g_lock);
			exit(0);
		}
	}


	/*
	Print the value of g_count
	it must be (thread_count * value)
	*/

	printf("value: %ld\n", g_count);
	
	free(tid);
}
