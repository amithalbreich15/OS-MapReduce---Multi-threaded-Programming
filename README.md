This project involves the implementation of a **MapReduce framework** for multi-threaded programming, aimed at improving the performance of tasks by leveraging multiple processors and threads. The exercise focuses on the challenges of efficiently distributing tasks across multiple threads, managing synchronization between threads, and optimizing runtime performance.

### High-Level Overview:
The core of the project is implementing a **MapReduce framework** where tasks are split into **Map**, **Shuffle**, and **Reduce** phases, with each phase utilizing multiple threads to increase performance. The **Map** phase applies a function to input elements, the **Shuffle** phase organizes intermediate results, and the **Reduce** phase aggregates the results. The challenge lies in managing these phases and synchronizing threads for optimal performance.

### Key Components:
- **Client Implementation**: Includes the map and reduce functions, which are specific to each task. The `map` function processes input data and produces intermediary key-value pairs, while the `reduce` function processes these pairs to generate final output.
  
- **Framework Implementation**: Handles the multi-threading, synchronization, job management, and orchestration of different phases. The framework ensures efficient distribution of work, manages job state, and coordinates between threads to prevent race conditions.

### Detailed Steps:
- **Map Phase**: Each thread reads input data, applies the map function, and produces intermediate results.
- **Sort Phase**: Threads sort their intermediate data.
- **Shuffle Phase**: One thread collects and organizes the data into sequences by key.
- **Reduce Phase**: Each thread reduces sorted data based on keys and produces the final output.

### Framework Interface:
The framework supports asynchronous job execution and offers functions to start a job, monitor its progress, and manage synchronization between threads. Key functions include `startMapReduceJob`, `waitForJob`, and `getJobState`.

The project emphasizes performance, thread safety, and efficient resource management. You'll need to ensure that the implementation avoids race conditions, deadlocks, and memory leaks while maintaining optimal execution speed. The submission requires providing source files, a makefile to compile the project, and a README outlining the implementation.
