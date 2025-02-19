#!/bin/bash
# This script will install, compile and run the Fregata framework.

echo -e "\e[1;33m\ncloning Fregata\e[0m\n"
git clone https://github.com/WeiBenqiang/Fregata.git

cd Fregata
cd CBSmode
mkdir build
cd build
echo -e "\e[1;33m\nCompilation (it can take a few seconds)\e[0m\n"
sleep 3
cmake -DENABLE_TEST=ON ..
make 
echo -e "\e[1;33m\nExecution\e[0m\n"
sleep 3
./homoSM4_CB/homoAES
