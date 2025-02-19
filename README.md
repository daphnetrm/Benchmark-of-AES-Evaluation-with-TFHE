# Benchmark of AES Evaluation with TFHE
This git repository is the benchmark associated to the paper **"Further Improvements in AES Execution over TFHE: Towards Breaking the 1 sec Barrier"** (available [here](https://eprint.iacr.org/2025/075.pdf)).

## Aim of this repository
This repository's aim is to be an open source unified software test bench for AES execution over TFHE, designed to obtain consistent same-machine experimental results as given in our paper.
We hope that it will be valuable resource for enabling fair comparisons in further works on AES in the community.
If you have a new AES implementation using TFHE that you feel should be part of this benchmark, do not hesitate to contact us!

## Updated List of homomorphic AES implementation with TFHE
There currently exist several implementation of the AES relying on TFHE. Here is a list of the implementations we ran in the article in order to produce the benchmark:
- The *full-LUT* approach of the **"A Homomorphic AES evaluation in Less than 30 Seconds by Means of TFHE"** paper, published at WAHC'23 (available on the [ACM site](https://dl.acm.org/doi/pdf/10.1145/3605759.3625260) or on [IACReprint](https://eprint.iacr.org/2023/1020.pdf)).
- The *p-encodings* approach of the **"Optimized Homomorphic Evaluation of Boolean Functions"** paper, published in TCHES 2024 (available on the [TCHES website](https://tches.iacr.org/index.php/TCHES/article/view/11680/11200) or on [IACReprint](https://eprint.iacr.org/2023/1589.pdf)).
- The *Fregata* framework of the **"Fregata: Faster Homomorphic Evaluation of AES via TFHE"** paper, published at ISC 2023 (available on the [ACM site](https://dl.acm.org/doi/10.1007/978-3-031-49187-0_20)).
- The *Thunderbird** framework approach of the **"Thunderbird: Efficient Homomorphic Evaluation of Symmetric Ciphers in 3GPP by combining two modes of TFHE"** paper, published in TCHES 2024 (available on the [TCHES website](https://tches.iacr.org/index.php/TCHES/article/view/11687/11207)).      
  *Note that no online implementation is given, see Section 5 of the **"Further Improvements in AES Execution over TFHE: Towards Breaking the 1 sec Barrier"** paper for more details.
- The *Hippogryph* framework of the **"Further Improvements in AES Execution over TFHE: Towards Breaking the 1 sec Barrier"** paper associated with this benchmark (available on the [IACReprint](https://eprint.iacr.org/2025/075.pdf))
 
## List of available implementations in this repository
This repository contains implementations and links to state-of-the-art AES homomorphic evaluation under TFHE. Each folder at the root of this git repository correspond to one TFHE library. In each one of these folder, you will find implementations or shell scripts to download, compile and execute implementations that already exist elsewhere on github. The names of the scripts are self-sufficient. To execute a script name script.sh, just enter the command "bash script.sh" on your terminal. If there is no script to execute, then a README file will explain how to install, compile and run the implementation.   
The repository is structured as follows:
- the "TFHElib" repository contains:
    - a **full-LUT** folder that contains the code of the **"A Homomorphic AES evaluation in Less than 30 Seconds by Means of TFHE"** paper (so there is no script to download it).
- the "tfhe-rs" repository contains:
    - a **Hippogryph** folder containing scripts to install and run the framework **Hippogryph** available at https://github.com/CryptoExperts/Hippogryph.git. 
    - a **p-encodings** folder that contains scripts to install and run the p-encodings approach available at https://github.com/CryptoExperts/bpr-boolean-fhe.git.
- the "TFHEpp" folder contains:
    - a **Fregata** folder containing scripts to install and run the **Fregata** framework that is available at https://github.com/WeiBenqiang/Fregata.git. 
    - a **Thunderbird** folder that contains a version of Thunderbird that we implemented ourselves. It is not fully correct (see Section 5 of *"Further Improvements in AES Execution over TFHE: Towards Breaking the 1 sec Barrier"* for more details) as some operations are performed on the wrong level. A public implementation of the framework is not yet available, but we will contact the authors to propose them to upload it on this repository. Scripts to compile and execute it are proposed in the parent directory.

Please note that these implementations are covered by their respective licences.
