#!/bin/bash
# This script will run the AES implementation

cd FFT-based-CircuitBootstrap
echo -e "\e[1;36m\nExecution\e[0m\n"
sleep 3
cargo bench --bench bench_aes_half_cbs
