//
// Created by yuval on 5/9/2023.
//

#include "MapReduceFramework.h"

#include <iostream>
#include "Job.h"
#include <algorithm>


#define PTHREAD_CREATE_ERR "pthread_create failed"
#define PTHREAD_JOIN_ERR "pthread_join failed"
#define SEM_WAIT_ERR "sem_wait failed"
#define SEM_POST_ERR "sem_post failed"


struct
{
    bool operator()(IntermediatePair a, IntermediatePair b) const {
      return *(a.first) < *(b.first);
    }
}
comp;


void emit2 (K2* key, V2* value, void* context){
  auto inter_vec = (IntermediateVec *) context;
  IntermediatePair pair = {key, value};
  inter_vec->push_back(pair);
}

void emit3 (K3* key, V3* value, void* context){
  auto job = (Job *) context;
  OutputPair pair = {key, value};
  pthread_mutex_lock(&(job->output_mutex));
  job->output_vec.push_back (pair);
  pthread_mutex_unlock(&(job->output_mutex));
}

void sort_phase(Job *job, IntermediateVec *inter_vec){
  /*
   * Sorts the current thread's intermediate vector according to key and
   * adds it to the current job's database
   */
  std::sort(inter_vec->begin(), inter_vec->end(), comp);
  int res = pthread_mutex_lock(&(job->sorted_vec_mutex));
  if (res != 0){
      std::cout << ERR_STR << MUTEX_LOCK_ERR << std::endl;
      exit(EXIT_FAILURE);
  }
  (job->sorted_vec).push_back (inter_vec);
  res = pthread_mutex_unlock (&(job->sorted_vec_mutex));
  if (res != 0){
      std::cout << ERR_STR << MUTEX_UNLOCK_ERR << std::endl;
      exit(EXIT_FAILURE);
  }
}

void apply_phase(Job *job, IntermediateVec *inter_vec, int stage){
  /*
   * Takes unprocessed elements from the input vector of the given job as long
   * as they exist, runs the client's function according to given stage (map
   * or reduce) on each elements and saves all the results in a vector of
   * intermediate pairs
   */
  auto old_value = (job->atomic_counter)++;
  auto cur_key = old_value & 0x7fffffff;
  auto num_keys = (old_value >> 31) & 0x7fffffff;

  while (cur_key < num_keys){
    if (stage == MAP_STAGE){
      (job->client).map ((job->input_vec)[cur_key].first, (job->input_vec)
      [cur_key].second, inter_vec);
    }
    else{
      (job->client).reduce(&(job->shuffled_vec[cur_key]), job);
    }
    old_value = (job->atomic_counter)++;
    cur_key = old_value & 0x7fffffff;
  }
}

void shuffle_phase(Job *job){
    /*
     * Runs the shuffle phase on the given job's sorted_vec
     * (vector of sorted IntermediateVecs) after the sort phase
     * of each thread, and saves the output in shuffled_vec
     */
  uint64_t sorted_count = 0;
  IntermediateVec last_vec;
  K2 *cur_key;
  K2 *prev_key;
  for (auto vec : job->sorted_vec){
    if (!vec->empty()){
      last_vec.push_back(vec->back());
      vec->pop_back();
    }
  }
  std::sort(last_vec.begin(), last_vec.end(), comp);
  prev_key = last_vec.back().first;
  while (!last_vec.empty()){
    IntermediateVec cur_vec;
    cur_key = last_vec.back().first;
    while(!last_vec.empty() && !(*(last_vec.back().first) < *cur_key)){
      cur_vec.push_back(last_vec.back());
      job->atomic_counter++;
      last_vec.pop_back();
    }
    if (*cur_key < *prev_key || job->shuffled_vec.empty())
    {
      job->shuffled_vec.push_back (cur_vec);
      prev_key = cur_key;
      sorted_count++;
    }
    else{
      while (!cur_vec.empty()){
        int last_index = (int)(job->shuffled_vec).size() - 1;
        job->shuffled_vec[last_index].push_back(cur_vec.back());
        cur_vec.pop_back();
      }
    }
    for (auto vec : job->sorted_vec){
      if (!vec->empty()){
        last_vec.push_back(vec->back());
        vec->pop_back();
      }
    }
    std::sort(last_vec.begin(), last_vec.end(), comp);
  }
  job->atomic_counter = (sorted_count << 31) + (((uint64_t) 3) << 62);
}

void *run_thread(void *arg){
  /*
   * Manages the run of a MapReduce thread. A pointer to a Job object is
   * given as input.
   */
  auto job = static_cast<Job *>(arg);
  auto inter_vec = new IntermediateVec;
  apply_phase (job, inter_vec, MAP_STAGE);
  sort_phase(job, inter_vec);
  (job->barrier).barrier();
  int res = sem_wait (&(job->shuffle_sem));
  if (res != 0){
      std::cout << ERR_STR << SEM_WAIT_ERR << std::endl;
      exit(EXIT_FAILURE);
  }
  if (job->shuffle_flag){
    job->shuffle_flag = false;
    uint64_t size = job->input_vec.size();
    job->atomic_counter = (size << 31) + (((uint64_t) 2) << 62);
    shuffle_phase (job);
  }
  res = sem_post (&(job->shuffle_sem));
  if (res != 0){
      std::cout << ERR_STR << SEM_POST_ERR << std::endl;
      exit(EXIT_FAILURE);
  }
  apply_phase (job, inter_vec, REDUCE_STAGE);
  return nullptr;
}

JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel){

  auto job = new Job(multiThreadLevel, inputVec, outputVec, client);
  job->atomic_counter += ((uint64_t) 1) << 62;
  for (int i=0; i<multiThreadLevel; i++){
    auto res = pthread_create(job->threads + i, nullptr, run_thread, job);
    if (res != 0){
      std::cerr << ERR_STR << PTHREAD_CREATE_ERR << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  return job;
}

void getJobState(JobHandle job, JobState* state){
  auto job_ptr = static_cast<Job *>(job);
  auto counter = (job_ptr->atomic_counter).load();
  state->stage = (stage_t) (counter >> 62);
  float perc = ((counter & 0x7fffffff) /((counter >> 31) &
                                         0x7fffffff))*100;
  state->percentage = std::min (perc, (float) 100.0);
}

void waitForJob(JobHandle job){
  auto job_ptr = static_cast<Job *>(job);
  if (job_ptr->is_finished){
    return;
  }

  for (int i=0; i<job_ptr->n_threads; i++){
    auto res = pthread_join((job_ptr->threads)[i], nullptr);
    if (res < 0){
      std::cout << ERR_STR << PTHREAD_JOIN_ERR << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  job_ptr->is_finished = true;
}

void closeJobHandle(JobHandle job){
  waitForJob (job);
  auto job_ptr = static_cast<Job *>(job);
  int res = pthread_mutex_destroy(&(job_ptr->output_mutex));
  if (res != 0){
      std::cout << ERR_STR << MUTEX_DESTROY_ERR << std::endl;
      exit(EXIT_FAILURE);
  }
  res = pthread_mutex_destroy(&(job_ptr->sorted_vec_mutex));
  if (res != 0){
      std::cout << ERR_STR << MUTEX_DESTROY_ERR << std::endl;
      exit(EXIT_FAILURE);
  }
  res = sem_destroy (&(job_ptr->shuffle_sem));
  if (res != 0){
      std::cout << ERR_STR << SEM_DESTROY_ERR << std::endl;
      exit(EXIT_FAILURE);
  }
  for (auto inter_vec : job_ptr->sorted_vec){
    delete inter_vec;
  }
  delete [] job_ptr->threads;
  delete job_ptr;
}