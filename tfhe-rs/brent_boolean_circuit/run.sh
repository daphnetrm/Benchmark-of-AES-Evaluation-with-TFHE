#!/bin/bash
# This script will run the project.
cd tfhe-rs-examples
echo -e "\e[1;36m\nExecution\e[0m\n"
sleep 3
cd aes_128_threads
cargo run --release
