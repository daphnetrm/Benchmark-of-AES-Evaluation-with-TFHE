#!/bin/bash
# This script will run our version of the Thunderbird framework.

echo -e "\e[1;33m\nCompilation (won't work if Fregata is not already installed)\e[0m\n"
cp thunderbird.cpp ../Fregata/CBSmode/homoSM4_CB/thunderbird.cpp
cd ../Fregata
cd CBSmode
echo "add_executable(thunderbird thunderbird.cpp) \ntarget_link_libraries(thunderbird tfhe++ )\n" >> homoSM4_CB/CMakeLists.txt
cd build
make
