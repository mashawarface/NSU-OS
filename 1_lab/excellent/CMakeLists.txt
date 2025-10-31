cmake_minimum_required(VERSION 3.10)
project(thread LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(mythread
    lib/src/mythread.c
)

target_include_directories(mythread PUBLIC
    ${PROJECT_SOURCE_DIR}/lib/include
)

add_executable(test_mythread
    tests/main.c
)

target_link_libraries(test_mythread PRIVATE mythread)

install(TARGETS mythread
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY lib/include/ DESTINATION include)