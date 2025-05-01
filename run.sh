#!/bin/bash

# ./build/hash_table $@
nasm -f elf64 source/my_memcmp.s -o myMemcmp.o
nasm -f elf64 source/hash_function.s -o hash_function.o
gcc -Iinclude -masm=intel -no-pie -O2 -g -msse4 -march=native hash_function.o source/main.cpp source/hash_table.cpp source/list.cpp source/text_processing.cpp myMemcmp.o -o hash_table



