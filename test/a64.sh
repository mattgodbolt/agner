#!/bin/bash
#compile and run PMCTest in 64 bit mode with yasm assembly syntax

# Compile A file if modified
if [ PMCTestA.cpp -nt a64.o ] ; then
g++ -O2 -c -m64 -oa64.o PMCTestA.cpp
fi

nasm -f elf64 -l b64.lst -o b64.o PMCTestB64.nasm
if [ $? -ne 0 ] ; then exit ; fi

g++ a64.o b64.o -lpthread
if [ $? -ne 0 ] ; then exit ; fi

./a.out