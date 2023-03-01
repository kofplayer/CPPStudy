cmake_minimum_required(VERSION 3.21)
project(CPPStudy)

set(CMAKE_CXX_STANDARD 20)

add_executable(CPPStudy main.cpp common/queue/SafeQueue.hpp common/thread/ThreadPool.cpp common/thread/ThreadPool.h)
