#!/bin/bash
#compile and run PMCTest in 64 bit mode with nasm assembly syntax

set -e

make -s a64.o

nasm -f elf64 -l b64.lst -o b64.o PMCTestB64.nasm

g++ a64.o b64.o -lpthread

./a.out
