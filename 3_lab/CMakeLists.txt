cmake_minimum_required(VERSION 3.28)
project(proxy LANGUAGES C)

set(CMAKE_C_COMPILER clang)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(lib/include)

set(LIB_SOURCES
    lib/src/logger.c
    lib/src/picohttpparser.c
)

set(APP_SOURCES
    src/proxy.c
)

add_executable(proxy ${APP_SOURCES} ${LIB_SOURCES})