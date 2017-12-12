#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct hybrid_lock {
	pthread_mutex_t mutex_lock;
	pthread_spinlock_t spin_lock;
};

int hybrid_lock_init(struct hybrid_lock *lock);

int hybrid_lock_destroy(struct hybrid_lock *lock);

int hybrid_lock_lock(struct hybrid_lock *lock);

int hybrid_lock_unlock(struct hybrid_lock *lock);
