//
// Created by yuval on 5/21/2023.
//
//
// Created by yuval on 5/15/2023.
//
#include "MapReduceFramework.h"
#include <cstdio>
#include <string>
#include <array>
#include <unistd.h>
#include <iostream>

#define NUM_THREADS 3
#define NUM_JOBS 5

class VString : public V1 {
 public:
  VString(std::string content) : content(content) { }
  std::string content;
};

class KChar : public K2, public K3{
 public:
  KChar(char c) : c(c) { }
  virtual bool operator<(const K2 &other) const {
    return c < static_cast<const KChar&>(other).c;
  }
  virtual bool operator<(const K3 &other) const {
    return c < static_cast<const KChar&>(other).c;
  }
  char c;
};

class VCount : public V2, public V3{
 public:
  VCount(int count) : count(count) { }
  int count;
};


class CounterClient : public MapReduceClient {
 public:
  void map(const K1* key, const V1* value, void* context) const {
    std::array<int, 256> counts;
    counts.fill(0);
    for(const char& c : static_cast<const VString*>(value)->content) {
      counts[(unsigned char) c]++;
    }

    for (int i = 255; i >= 0; --i) {
      if (counts[i] == 0)
        continue;

      KChar* k2 = new KChar(i);
      VCount* v2 = new VCount(counts[i]);
      usleep(150000);
      emit2(k2, v2, context);
    }
  }

  virtual void reduce(const IntermediateVec* pairs,
                      void* context) const {
    const char c = static_cast<const KChar*>(pairs->at(0).first)->c;
    int count = 0;
    for(const IntermediatePair& pair: *pairs) {
      count += static_cast<const VCount*>(pair.second)->count;
      delete pair.first;
      delete pair.second;
    }
    KChar* k3 = new KChar(c);
    VCount* v3 = new VCount(count);
    usleep(150000);
    emit3(k3, v3, context);
  }
};


void *run_job(void *arg)
{
  auto id = static_cast<int *>(arg);
  CounterClient client;
  InputVec inputVec;
  OutputVec outputVec;
  VString s1("This string is full of characters");
  VString s2("Multithreading is awesome");
  VString s3("race conditions are bad");
  VString s4("race conditions are bad2");
  VString s5("race conditions are bad3");
  VString s6("race conditions are bad4");
  VString s7("My job is the best!");
  VString s8("count my letters");
  if ((*id) % 2 == 0){
    inputVec.push_back({nullptr, &s1});
    inputVec.push_back({nullptr, &s2});
    inputVec.push_back({nullptr, &s3});
    inputVec.push_back({nullptr, &s4});
    inputVec.push_back({nullptr, &s5});
    inputVec.push_back({nullptr, &s6});
  }
  else{
    inputVec.push_back({nullptr, &s6});
    inputVec.push_back({nullptr, &s1});
    inputVec.push_back({nullptr, &s2});
    inputVec.push_back({nullptr, &s7});
    inputVec.push_back({nullptr, &s8});
  }
  JobState state;
  JobState last_state={UNDEFINED_STAGE,0};
  JobHandle job = startMapReduceJob(
      client, inputVec, outputVec, NUM_THREADS+(*id));
  getJobState(job, &state);

  while (state.stage != REDUCE_STAGE || state.percentage != 100.0)
  {
    if (last_state.stage != state.stage || last_state.percentage != state.percentage){
      printf("Job %d: stage %d, %f%% \n",
             *id, state.stage, state.percentage);
    }
    usleep(100);
    last_state = state;
    getJobState(job, &state);
  }
  printf("Job %d: stage %d, %f%% \n",
         *id, state.stage, state.percentage);
  printf("Job %d: Done!\n", *id);
  for (int i=0; i<(*id); i++){
    waitForJob (job);
  }
  closeJobHandle(job);

  for (OutputPair& pair: outputVec) {
    char c = ((const KChar*)pair.first)->c;
    int count = ((const VCount*)pair.second)->count;
    printf("Job %d: The character %c appeared %d time%s\n",
           *id, c, count, count > 1 ? "s" : "");
    delete pair.first;
    delete pair.second;
  }

  return nullptr;
}


int main(int argc, char** argv)
{
  pthread_t threads[NUM_JOBS];
  int indexes[NUM_JOBS];
  for (int i=0; i<NUM_JOBS; i++){
    indexes[i] = i + 1;
    pthread_create(threads + i, nullptr, run_job, indexes + i);
  }

  for (auto thread : threads){
    pthread_join(thread, nullptr);
  }

  std::cout << "All jobs are done!" << std::endl;
  return EXIT_SUCCESS;
}

