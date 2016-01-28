#!/bin/bash
# Compile and run PMCTest in 32 bit mode using yasm assembler syntax
# In 32-bit Linux: Remove -m32 flag on g++ commands
# In 64-bit Linux: Must install g++-multilib first

# Compile A file if modified
if [ PMCTestA.cpp -nt a32.o ] ; then
g++ -O2 -c -m32 -oa32.o PMCTestA.cpp
fi

nasm -f elf32 -l b32.lst -o b32.o PMCTestB32.nasm
if [ $? -ne 0 ] ; then exit ; fi

g++  -m32 a32.o b32.o  -lpthread
if [ $? -ne 0 ] ; then exit ; fi

./a.out