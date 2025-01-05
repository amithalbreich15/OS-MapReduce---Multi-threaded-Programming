//
// Created by yuval on 5/15/2023.
//
#include "Job.h"

Job::Job(int num_threads, const InputVec& inputVec, OutputVec& outputVec,
         const MapReduceClient& c):
  n_threads(num_threads), barrier(n_threads), atomic_counter(0),
  input_vec(inputVec), sorted_vec(), shuffled_vec(), output_vec(outputVec),
  output_mutex(PTHREAD_MUTEX_INITIALIZER),
  sorted_vec_mutex(PTHREAD_MUTEX_INITIALIZER),
  client(c), is_finished(false),
  shuffle_flag(true)
{
  atomic_counter = (uint64_t) 0;
  threads = new pthread_t[n_threads];
  auto res = pthread_mutex_init (&output_mutex, nullptr);
  if (res != 0){
    std::cout << ERR_STR << MUTEX_INIT_ERR << std::endl;
    exit(EXIT_FAILURE);
  }
  res = pthread_mutex_init (&sorted_vec_mutex, nullptr);
  if (res != 0){
    std::cout << ERR_STR << MUTEX_INIT_ERR << std::endl;
    exit(EXIT_FAILURE);
  }
  res = sem_init (&shuffle_sem, SEM_SHARED, SEM_VALUE);
  if (res != 0){
    std::cout << ERR_STR << SEM_INIT_ERR << std::endl;
    exit(EXIT_FAILURE);
  }
  auto num_keys = input_vec.size();
  atomic_counter += (num_keys << 31);
}
