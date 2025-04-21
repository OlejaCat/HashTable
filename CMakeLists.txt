cmake_minimum_required(VERSION 3.20)
project(hash_table)

set(CMAKE_CXX_FLAGS "--std=c++20 -Wall -Wextra -pedantic")

set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -g ")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(${PROJECT_NAME}
    source/main.cpp
    source/text_processing.cpp
    source/list.cpp
    source/hash_table.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/
)
