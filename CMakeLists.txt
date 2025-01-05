cmake_minimum_required(VERSION 3.22)
project(OS_ex3)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)

include_directories(.)

add_executable(OS_ex3
        MapReduceClient.h
        MapReduceFramework.cpp
        MapReduceFramework.h
        Job.h
        Job.cpp
        Barrier.h
        Barrier.cpp
        test.cpp)
