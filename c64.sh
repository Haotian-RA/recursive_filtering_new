#!/bin/bash
# Compile and run PMCTest in 64 bit mode using C++
# (c) 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses

# Compile A file if modified
if [ PMCTestA.cpp -nt a64.o ] ; then
g++ -O2 -c -m64 -oa64.o PMCTestA.cpp
fi

# Compile B file and link
clang++ -std=c++20 -mavx2 -mfma -O2 a64.o -march=native -lpthread PMCTestB.cpp
if [ $? -ne 0 ] ; then exit ; fi
taskset -c 0 ./a.out

# read -p "Press [Enter]"
