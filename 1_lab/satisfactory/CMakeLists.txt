cmake_minimum_required(VERSION 3.10)
project(MyProject C)

set(CMAKE_C_COMPILER clang)

find_package(Threads REQUIRED)

set(EXECUTABLES
    basic/pthread_creation
    basic/pthread_retval
    cancel/cancel_printf
    cancel/cancel
    cancel/cancel_malloc
    detached/pthread_detach
    detached/pthread_struct
    detached/pthread_struct_detach
    signal/signal
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

foreach(exec ${EXECUTABLES})
    get_filename_component(target_name ${exec} NAME_WE)
    set(source_file src/${exec}.c)

    add_executable(${target_name} ${source_file})

    target_compile_options(${target_name} PRIVATE ${COMPILE_OPTIONS})
    target_link_options(${target_name} PRIVATE ${LINK_OPTIONS})
    target_link_libraries(${target_name} PRIVATE Threads::Threads)
endforeach()