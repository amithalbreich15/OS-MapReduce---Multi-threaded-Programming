#ifndef BARRIER_H
#define BARRIER_H
#include <pthread.h>
#include <iostream>

// a multiple use barrier

#define MUTEX_DESTROY_ERR "error on pthread_mutex_destroy"
#define SEM_DESTROY_ERR "error on sem_destroy"
#define PTHREAD_COND_ERR "error on pthread_cond_destroy"
#define MUTEX_LOCK_ERR "error on pthread_mutex_lock"
#define BROADCAST_ERR "error on pthread_cond_broadcast"
#define MUTEX_UNLOCK_ERR "error on pthread_mutex_unlock"
#define PTHREAD_COND_WAIT_ERR "error on pthread_cond_wait"
#define ERR_STR "system error: "

class Barrier {
public:
	explicit Barrier(int numThreads);
	~Barrier();
	void barrier();

private:
	pthread_mutex_t mutex;
	pthread_cond_t cv;
	int count;
	int numThreads;
};

#endif //BARRIER_H
