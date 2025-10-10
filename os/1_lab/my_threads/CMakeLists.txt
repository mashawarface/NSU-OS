cmake_minimum_required(VERSION 3.10)
project(MyProject C)

set(CMAKE_C_COMPILER clang)

find_package(Threads REQUIRED)

add_executable(my_thread src/thread3.c)

target_compile_options(my_thread PRIVATE
    -Wall
    -Wextra
    -pthread
    -fsanitize=address,undefined,leak
    -fno-omit-frame-pointer
)

target_link_options(my_thread PRIVATE
    -fsanitize=address,undefined
)

target_link_libraries(my_thread ${CMAKE_THREAD_LIBS_INIT})