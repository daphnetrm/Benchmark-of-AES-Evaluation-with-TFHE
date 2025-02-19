#!/bin/bash
# This script will run the p-encodings framework.


cd bpr-boolean-fhe
cd implementations_crypto_primitive_tfhe/crypto_primitives
echo -e "\e[1;36m\nExecution\e[0m\n"
sleep 3
cargo run --release
