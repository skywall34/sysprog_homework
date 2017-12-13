#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "hybrid_lock.h"

int hybrid_lock_init(struct hybrid_lock *lock)
{	
	if(pthread_mutex_init(&lock->mutex_lock, NULL)!= 0){
		printf("mutex_lock_init error\n");
		return -1;
	}
	if(pthread_spin_init(&lock -> spin_lock, PTHREAD_PROCESS_PRIVATE)!=0){
		printf("spin_lock_init error\n");
		return -1;
	}

	return 0;
}

int hybrid_lock_destroy(struct hybrid_lock *lock)
{
	if(pthread_mutex_destroy(&lock->mutex_lock)!=0){
		printf("mutex_lock_destroy error\n");
		return -1;
	}
	if(pthread_spin_destroy(&lock->spin_lock)!=0){
		printf("spin_lock_destroy error\n");
		return -1;
	}

	return 0;
}


int hybrid_lock_lock(struct hybrid_lock *lock)
{
	struct timeval start_time, current_time, end_time;
	//changed
	double diff_time;
	gettimeofday(&start_time, NULL);
	int i = 0, count = 0, j =0;
	while(i == 0){
		if(pthread_spin_trylock(&lock -> spin_lock)){
			i++;
			//pthread_spin_unlock(&lock->spin_lock);
		}
		gettimeofday(&current_time, NULL);
		count++;
		diff_time = current_time.tv_sec - start_time.tv_sec;
		diff_time += (current_time.tv_usec - start_time.tv_usec)/ 1000000.0;
		if(diff_time >1 ){
			break;
		}
	}
	if(i!=0){
		pthread_spin_unlock(&lock->spin_lock);
		if(pthread_mutex_trylock(&lock->mutex_lock)!=0){
			while(j==0){
				if(pthread_mutex_trylock(&lock->mutex_lock)==0){
					j++;
					return 0;
				}
				gettimeofday(&current_time, NULL);
				count++;
				diff_time = current_time.tv_sec-start_time.tv_sec;
				diff_time += (current_time.tv_usec - start_time.tv_usec)/1000000000.0;
				if(diff_time>1){
					break;
				}
			}
		}
		else{
			j++;
			return 0;
		}
	}
	gettimeofday(&end_time, NULL);
	diff_time = end_time.tv_sec-start_time.tv_sec;
	diff_time += (end_time.tv_sec - start_time.tv_sec)/10000000000.0;

	if(j ==0){
		pthread_mutex_lock(&lock->mutex_lock);

		if(i == 0){
			pthread_spin_lock(&lock -> spin_lock);
		}
	}
	else{
		if(i==0){
			pthread_spin_lock(&lock->spin_lock);
		}
	}
	//printf("total spin time: %lf\n", diff_time);
	//printf("gettimeofday count : %d\n", count);
	
}

int hybrid_lock_unlock(struct hybrid_lock *lock)
{
	if(pthread_spin_unlock(&lock->spin_lock)!=0)
		return -1;
	if(pthread_mutex_unlock(&lock->mutex_lock)!=0)
		return 1;

	return 0;
}
