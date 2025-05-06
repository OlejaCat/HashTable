cmake_minimum_required(VERSION 3.20)
project(hash_table)

set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS s)

enable_language(ASM_NASM)

set(CMAKE_CXX_FLAGS "--std=c++20 -Wall -masm=intel -Wextra -pedantic -no-pie -mavx2 -march=native")

set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g")

set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")
set(CMAKE_ASM_NASM_FLAGS "-Wall -g")

add_executable(${PROJECT_NAME}
    source/main.cpp
    source/text_processing.cpp
    source/list.cpp
    source/hash_table.cpp
    source/my_memcmp.s   
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/
)

add_executable(hash_tester
    source/hash_tests.cpp
    source/hash_tests_logic.cpp
    source/text_processing.cpp
)

target_include_directories(hash_tester
    PRIVATE
        include/
)
