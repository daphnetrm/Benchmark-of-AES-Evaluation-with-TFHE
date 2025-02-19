#!/bin/bash
# This script will install dependencies and compile the Fregata framework.

cd Fregata
cd CBSmode
mkdir build
cd build
echo -e "\e[1;33m\nCompilation (it can take a few seconds)\e[0m\n"
sleep 3
cmake -DENABLE_TEST=ON ..
make 
sleep 3

