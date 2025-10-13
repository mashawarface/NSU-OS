cmake_minimum_required(VERSION 3.10)
project(MyProject C)

set(CMAKE_C_COMPILER clang)

find_package(Threads REQUIRED)

set(SOURCES
    pthread_creation
    pthread_detach
    pthread_retval
    pthread_struct
    pthread_struct_detach
    )

set(COMPILE_OPTIONS
    -Wall
    -Wextra
    -pthread
    -fsanitize=address,undefined,leak
    -fno-omit-frame-pointer    
)

set(LINK_OPTIONS
    -fsanitize=address,undefined,leak
)

foreach(source_file ${SOURCES})
    add_executable(${source_file} src/${source_file}.c)

    target_compile_options(${source_file} PRIVATE ${COMPILE_OPTIONS})
    target_link_options(${source_file} PRIVATE ${LINK_OPTIONS})
    target_link_libraries(${source_file} ${CMAKE_THREAD_LIBS_INIT})
endforeach()
