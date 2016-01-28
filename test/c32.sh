#!/bin/bash
# Compile and run PMCTest in 32 bit mode using C++
# (c) 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses

# In 32-bit Linux: Remove -m32 flag on g++ commands
# In 64-bit Linux: Must install g++-multilib first

# Compile A file if modified
if [ PMCTestA.cpp -nt a32.o ] ; then
g++ -O2 -c -m32 -oa32.o PMCTestA.cpp
fi

# Compile B file and link
g++ -O2 -c -m32 a32.o PMCTestB.cpp
if [ $? -ne 0 ] ; then exit ; fi
./a.out

# read -p "Press [Enter]"
