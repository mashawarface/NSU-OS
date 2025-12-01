cmake_minimum_required(VERSION 3.10)
project(ThreadsSync LANGUAGES C)

set(CMAKE_C_COMPILER clang)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/lib/include)

add_library(list_mutex STATIC
    ${CMAKE_SOURCE_DIR}/lib/src/list_mutex.c
)

add_executable(threads_mutex
    ${CMAKE_SOURCE_DIR}/src/threads_mutex.c
)
target_link_libraries(threads_mutex list_mutex)

add_library(list_rwlock STATIC
    ${CMAKE_SOURCE_DIR}/lib/src/list_rwlock.c
)

add_executable(threads_rwlock
    ${CMAKE_SOURCE_DIR}/src/threads_rwlock.c
)
target_link_libraries(threads_rwlock list_rwlock)

find_package(Threads REQUIRED)
target_link_libraries(threads_mutex Threads::Threads)
target_link_libraries(threads_rwlock Threads::Threads)
