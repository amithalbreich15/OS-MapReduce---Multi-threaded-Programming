//
// Created by yuval on 5/15/2023.
//

#ifndef _JOBMANAGER_H_
#define _JOBMANAGER_H_

#include <vector>
#include <memory>
#include "Job.h"


class JobManager {
 public:
  JobManager();
  std::vector<std::shared_ptr<Job>> jobs;
  pthread_mutex_t jobs_mutex;
};

#endif //_JOBMANAGER_H_
