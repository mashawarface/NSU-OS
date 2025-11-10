cmake_minimum_required(VERSION 3.10)
project(mythread LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# set(COMPILE_OPTIONS
#     -Wall
#     -Wextra
#     -pthread
#     -fsanitize=address,undefined,leak
#     -fno-omit-frame-pointer
# )

# set(LINK_OPTIONS
#     -fsanitize=address,undefined,leak
# )

add_library(mythread
    lib/src/mythread.c
)

target_include_directories(mythread PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/include
)

target_compile_options(mythread PRIVATE ${COMPILE_OPTIONS})
target_link_options(mythread PRIVATE ${LINK_OPTIONS})

set(TEST_EXECUTABLES
    tests/print
    tests/five_threads
    tests/retval
    tests/cancel
)

foreach(exec ${TEST_EXECUTABLES})
    get_filename_component(target_name ${exec} NAME_WE)
    set(source_file ${exec}.c)

    add_executable(${target_name} ${source_file})

    target_compile_options(${target_name} PRIVATE ${COMPILE_OPTIONS})
    target_link_options(${target_name} PRIVATE ${LINK_OPTIONS})
    target_link_libraries(${target_name} PRIVATE mythread)
endforeach()

install(TARGETS mythread
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY lib/include/ DESTINATION include)