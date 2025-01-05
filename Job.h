//
// Created by yuval on 5/15/2023.
//

#ifndef _JOB_H_
#define _JOB_H_

#include <vector>
#include "Barrier.h"
#include <atomic>
#include <semaphore.h>

#include "MapReduceClient.h"
#include <iostream>

#define ERR_STR "system error: "
#define MUTEX_INIT_ERR "pthread_mutex_init failed"
#define SEM_INIT_ERR "sem_init failed"

#define SEM_SHARED 0
#define SEM_VALUE 1


class Job{
 public:
  Job(int num_threads, const InputVec& inputVec, OutputVec& outputVec,
      const MapReduceClient& c);

  int n_threads;
  pthread_t *threads;
  Barrier barrier;
  std::atomic<uint64_t> atomic_counter;

  // vectors
  const InputVec& input_vec;
  std::vector<IntermediateVec *> sorted_vec;
  std::vector<IntermediateVec> shuffled_vec;
  OutputVec& output_vec;

  // mutexes and semaphores
  pthread_mutex_t output_mutex;
  pthread_mutex_t sorted_vec_mutex;
  sem_t shuffle_sem;

  const MapReduceClient& client;
  bool is_finished;
  bool shuffle_flag;
};

#endif //_JOB_H_
