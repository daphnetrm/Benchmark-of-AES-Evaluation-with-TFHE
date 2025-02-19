#!/bin/bash
# This script will install, compile and run the p-encodings framework.

echo -e "\e[1;36m\ncloning Hippogryph\e[0m\n"
git clone https://github.com/CryptoExperts/bpr-boolean-fhe.git

echo -e "\e[1;36m\nInstalling Rust (enter N if you already have Rust)\e[0m\n"
# Install Rust if necessary
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
#if rpm -qi "rustc"; then echo rustc --version ; else echo curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh; fi
rustup update
export PATTERN="no"
cd bpr-boolean-fhe
echo -e "\e[1;36m\nCompilation\e[0m\n"
sleep 3
cd implementations_crypto_primitive_tfhe/crypto_primitives
cargo build --release

echo -e "\e[1;36m\nExecution\e[0m\n"
sleep 3
cargo run --release

