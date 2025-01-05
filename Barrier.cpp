#include "Barrier.h"
#include <cstdlib>
#include <cstdio>

Barrier::Barrier(int numThreads)
		: mutex(PTHREAD_MUTEX_INITIALIZER)
		, cv(PTHREAD_COND_INITIALIZER)
		, count(0)
		, numThreads(numThreads)
{ }


Barrier::~Barrier()
{
	if (pthread_mutex_destroy(&mutex) != 0) {
		std::cout << ERR_STR << MUTEX_DESTROY_ERR << std::endl;
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_destroy(&cv) != 0){
        std::cout << ERR_STR << PTHREAD_COND_ERR << std::endl;
		exit(EXIT_FAILURE);
	}
}


void Barrier::barrier()
{
	if (pthread_mutex_lock(&mutex) != 0){
        std::cout << ERR_STR << MUTEX_LOCK_ERR << std::endl;
		exit(EXIT_FAILURE);
	}
	if (++count < numThreads) {
		if (pthread_cond_wait(&cv, &mutex) != 0){
            std::cout << ERR_STR << PTHREAD_COND_WAIT_ERR << std::endl;
			exit(EXIT_FAILURE);
		}
	} else {
		count = 0;
		if (pthread_cond_broadcast(&cv) != 0) {
            std::cout << ERR_STR << BROADCAST_ERR << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	if (pthread_mutex_unlock(&mutex) != 0) {
        std::cout << ERR_STR << MUTEX_UNLOCK_ERR << std::endl;
		exit(EXIT_FAILURE);
	}
}
