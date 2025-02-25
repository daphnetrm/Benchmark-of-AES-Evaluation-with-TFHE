#!/bin/bash
# This script will install, compile and run the source code of the FHEW-like Leveled Homomoprhic Evaluation: Refined Workflow and Polished Building Blocks paper.

echo -e "\e[1;36m\ncloning \e[0m\n"
git clone https://github.com/KAIST-CryptLab/FFT-based-CircuitBootstrap.git

echo -e "\e[1;36m\nInstalling Rust (enter N if you already have Rust)\e[0m\n"
# Install Rust if necessary
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
#if rpm -qi "rustc"; then echo rustc --version ; else echo curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh; fi
rustup update

cd FFT-based-CircuitBootstrap
echo -e "\e[1;36m\nExecution\e[0m\n"
sleep 3
cargo bench --bench bench_aes_half_cbs
