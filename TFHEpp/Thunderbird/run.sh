#!/bin/bash
# This script will run our version of the Thunderbird framework.

echo -e "\e[1;33m\nExecution (won't work if Fregata is not already installed)\e[0m\n"
cp thunderbird.cpp ../Fregata/CBSmode/homoSM4_CB/thunderbird.cpp
cd ../Fregata/CBSmode/build
cd CBSmode

./homoSM4_CB/thunderbird


