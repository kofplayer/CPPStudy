cmake_minimum_required(VERSION 3.21)
project(CPPStudy)

set(CMAKE_CXX_STANDARD 20)

add_executable(CPPStudy main.cpp common/queue/SafeQueue.hpp common/thread/ThreadPool.cpp common/thread/ThreadPool.h
        sample/echo_async_server/EchoAsyncServer.cpp
        sample/echo_async_server/EchoAsyncServer.h
        sample/echo_async_server/EchoAsyncServer.h)

find_package(Boost REQUIRED COMPONENTS thread)
find_library(JEMALLOC_LIB jemalloc)

target_link_libraries(CPPStudy PRIVATE Boost::thread ${JEMALLOC_LIB})
