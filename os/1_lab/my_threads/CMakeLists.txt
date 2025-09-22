cmake_minimum_required(VERSION 3.10)
project(MyProject C)

set(CMAKE_C_COMPILER clang)

find_package(Threads REQUIRED)

add_executable(my_thread src/thread2.c)

target_link_libraries(my_thread ${CMAKE_THREAD_LIBS_INIT})

target_compile_options(my_thread PRIVATE -Wall -Wextra -pthread)
