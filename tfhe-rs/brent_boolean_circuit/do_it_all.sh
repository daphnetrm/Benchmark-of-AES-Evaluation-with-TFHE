#!/bin/bash
# This script will install, compile and run the source code of the project.

echo -e "\e[1;36m\ncloning[0m\n"
git clone https://github.com/CryptoPenguin15/tfhe-rs-examples.git

echo -e "\e[1;36m\nInstalling Rust (enter N if you already have Rust)\e[0m\n"
# Install Rust if necessary
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
#if rpm -qi "rustc"; then echo rustc --version ; else echo curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh; fi
rustup update
export PATTERN="no"
cd tfhe-rs-examples
echo -e "\e[1;36m\nCompilation\e[0m\n"
sleep 3
cd aes_128_threads
cargo build --release

echo -e "\e[1;36m\nExecution\e[0m\n"
sleep 3
cargo run --release



