cmake_minimum_required(VERSION 3.22)

project(CPPStudy)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_LIST src/*.cpp)
add_executable(CPPStudy ${SRC_LIST})

find_package(Boost REQUIRED COMPONENTS thread)

target_link_libraries(CPPStudy PRIVATE Boost::thread)

if (WIN32)
    target_link_libraries(CPPStudy PRIVATE ws2_32 mswsock)
endif()