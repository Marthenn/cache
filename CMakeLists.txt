cmake_minimum_required(VERSION 3.27)
project(Cache)

set(CMAKE_CXX_STANDARD 17)

add_executable(Cache main.cpp
        eviction/lru.cpp
        eviction/lfu.cpp
        prefetch/sequential.cpp
        prefetch/leap.cpp)
