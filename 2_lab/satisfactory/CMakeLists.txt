cmake_minimum_required(VERSION 3.14)
project(QueueProject C)

set(CMAKE_C_COMPILER clang)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread -fsanitize=address,undefined,leak -fno-omit-frame-pointer")

include_directories(${PROJECT_SOURCE_DIR}/lib/include)

add_library(queue STATIC
    lib/src/queue.c
)

add_executable(queue_example
    src/queue_example.c
)
target_link_libraries(queue_example queue)

add_executable(queue_threads
    src/queue_threads.c
)
target_link_libraries(queue_threads queue)