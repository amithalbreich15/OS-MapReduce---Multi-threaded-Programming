//
// Created by yuval on 5/15/2023.
//
Job::Job(const Job &other): job_num(other.job_num), n_threads(other
                                                                  .n_threads), threads(other.threads),
                            next_elem(other.next_elem), barrier(other.n_threads),
                            jobstate (other.jobstate) {

}

Job &Job::operator=(const Job &other){
  if (this == &other){
    return *this;
  }
  job_num = other.job_num;
  n_threads = other.n_threads;
  threads = other.threads;
  next_elem = other.next_elem;
  barrier = other.barrier;
  jobstate = other.jobstate;
  return *this;
}
