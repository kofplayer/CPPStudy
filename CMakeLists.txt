cmake_minimum_required(VERSION 3.21)
project(CPPStudy)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_LIST src/*.cpp)
add_executable(CPPStudy ${SRC_LIST})

find_package(Boost REQUIRED COMPONENTS thread)
find_library(JEMALLOC_LIB jemalloc)

target_link_libraries(CPPStudy PRIVATE Boost::thread ${JEMALLOC_LIB})
target_compile_definitions(CPPStudy PRIVATE BOOST_MYSQL_SEPARATE_COMPILATION)
