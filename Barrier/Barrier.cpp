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
		fprintf(stderr, "[[Barrier]] error on pthread_mutex_destroy");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_destroy(&cv) != 0){
		fprintf(stderr, "[[Barrier]] error on pthread_cond_destroy");
		exit(EXIT_FAILURE);
	}
}


void Barrier::barrier()
{
	if (pthread_mutex_lock(&mutex) != 0){
		fprintf(stderr, "[[Barrier]] error on pthread_mutex_lock");
		exit(EXIT_FAILURE);
	}
	if (++count < numThreads) {
		if (pthread_cond_wait(&cv, &mutex) != 0){
			fprintf(stderr, "[[Barrier]] error on pthread_cond_wait");
			exit(EXIT_FAILURE);
		}
	} else {
		count = 0;
		if (pthread_cond_broadcast(&cv) != 0) {
			fprintf(stderr, "[[Barrier]] error on pthread_cond_broadcast");
			exit(EXIT_FAILURE);
		}
	}
	if (pthread_mutex_unlock(&mutex) != 0) {
		fprintf(stderr, "[[Barrier]] error on pthread_mutex_unlock");
		exit(EXIT_FAILURE);
	}
}
