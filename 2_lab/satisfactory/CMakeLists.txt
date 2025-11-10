cmake_minimum_required(VERSION 3.16)
project(queue_project C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(LIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib/include)
set(LIB_SRC_DIR ${CMAKE_SOURCE_DIR}/lib/src)
set(APP_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(${LIB_INCLUDE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fno-inline -fno-tree-vectorize")

add_executable(queue_example
    ${APP_SRC_DIR}/queue_example.c
    ${LIB_SRC_DIR}/queue.c
)

target_include_directories(queue_example PRIVATE ${LIB_INCLUDE_DIR})
target_link_libraries(queue_example pthread)

add_executable(queue_threads
    ${APP_SRC_DIR}/queue_threads.c
    ${LIB_SRC_DIR}/queue.c
)

target_include_directories(queue_threads PRIVATE ${LIB_INCLUDE_DIR})
target_link_libraries(queue_threads pthread)

set(QUEUE_IMPLS
    sem
    mutex
    cond
    spin
)

foreach(IMPL IN LISTS QUEUE_IMPLS)
    add_executable(queue_threads_${IMPL}
        ${APP_SRC_DIR}/queue_threads.c
        ${LIB_SRC_DIR}/queue_${IMPL}.c
    )
    target_include_directories(queue_threads_${IMPL} PRIVATE ${LIB_INCLUDE_DIR})
    target_link_libraries(queue_threads_${IMPL} pthread)
endforeach()
