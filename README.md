# Benchmark of AES Evaluation with TFHE
This git repository is the benchmark associated with the paper **"Further Improvements in AES Execution over TFHE: Towards Breaking the 1 sec Barrier"** (available [here](https://eprint.iacr.org/2025/075.pdf)).

## Aim of this repository
This repository aims to be an open-source unified software test bench for AES execution over TFHE, designed to obtain consistent same-machine experimental results, as given in our paper.
We hope it will be a valuable resource for enabling fair comparisons in further works on AES in the community.
If you have a new AES implementation using TFHE that you feel should be part of this benchmark, do not hesitate to contact us!

## Updated List of homomorphic AES implementation with TFHE
There currently exist several implementations of the AES relying on TFHE. Here is a list of the implementations we ran in the article to produce the benchmark:
- The *full-LUT* approach of the **"A Homomorphic AES evaluation in Less than 30 Seconds by Means of TFHE"** paper, published at WAHC'23 (available on the [ACM site](https://dl.acm.org/doi/pdf/10.1145/3605759.3625260) or on [IACReprint](https://eprint.iacr.org/2023/1020.pdf)).
- The *p-encodings* approach of the **"Optimized Homomorphic Evaluation of Boolean Functions"** paper, published in TCHES 2024 (available on the [TCHES website](https://tches.iacr.org/index.php/TCHES/article/view/11680/11200) or on [IACReprint](https://eprint.iacr.org/2023/1589.pdf)).
- The *Fregata* framework of the **"Fregata: Faster Homomorphic Evaluation of AES via TFHE"** paper, published at ISC 2023 (available on the [ACM site](https://dl.acm.org/doi/10.1007/978-3-031-49187-0_20)).
- The *Thunderbird** framework approach of the **"Thunderbird: Efficient Homomorphic Evaluation of Symmetric Ciphers in 3GPP by combining two modes of TFHE"** paper, published in TCHES 2024 (available on the [TCHES website](https://tches.iacr.org/index.php/TCHES/article/view/11687/11207)).      
  *We have contacted the authors, and the source code will soon be available.
- The *Hippogryph* framework of the **"Further Improvements in AES Execution over TFHE: Towards Breaking the 1 sec Barrier"** paper associated with this benchmark (available on the [IACReprint](https://eprint.iacr.org/2025/075.pdf)).

Here is a list of new TFHE implementations that are not a part of our paper's benchmark:
- an *FFT-based* implementation of the **"FHEW-like Leveled Homomorphic Evaluation: Refined Workflow and Polished Building Blocks"** paper, available on the [IACReprint](https://eprint.iacr.org/2024/1318.pdf).
 
## List of available implementations in this repository
This repository contains implementations and links to state-of-the-art AES homomorphic evaluation under TFHE. Each folder at the root of this git repository corresponds to one TFHE library. In each of these folders, you will find implementations or shell scripts to download, compile, and execute implementations that already exist elsewhere on Git Hub. The names of the scripts are self-sufficient. To execute a script name script.sh, just enter the command "bash script.sh" on your terminal. If there is no script to execute, then a README file will explain how to install, compile, and run the implementation.   
The repository is structured as follows:
- the "TFHElib" repository contains:
    - a **full-LUT** folder that contains the code of the **"A Homomorphic AES evaluation in Less than 30 Seconds by Means of TFHE"** paper (so there is no script to download it).
- the "tfhe-rs" repository contains:
    - a **Hippogryph** folder containing scripts to install and run the framework **Hippogryph** available at https://github.com/CryptoExperts/Hippogryph.git. 
    - a **p-encodings** folder that contains scripts to install and run the p-encodings approach available at https://github.com/CryptoExperts/bpr-boolean-fhe.git.
    - an **FTT-based** folder that contains scripts to install and run the approach of the **FHEW-like Leveled Homomorphic Evaluation: Refined Workflow and Polished Building Blocks** paper, available at https://github.com/KAIST-CryptLab/FFT-based-CircuitBootstrap/tree/main.
- the "TFHEpp" folder contains:
    - a **Fregata** folder containing scripts to install and run the **Fregata** framework that is available at https://github.com/WeiBenqiang/Fregata.git. 
    - a **Thunderbird** folder that is currently empty.

Please note that these implementations are covered by their respective licences.
