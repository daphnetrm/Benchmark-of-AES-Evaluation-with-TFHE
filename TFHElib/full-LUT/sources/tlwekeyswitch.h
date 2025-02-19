#ifndef TLWEKEYSWITCH_H
#define TLWEKEYSWITCH_H

#include <tfhe.h>


struct TLweKeySwitchKey {
    int32_t n;            ///< length of the input key: s'
    int32_t t;            ///< decomposition length
    int32_t basebit;      ///< log_2(base)
    int32_t base;         ///< decomposition base: a power of 2
    const TLweParams * out_params; ///< params of the output key s
    TLweSample * ks0_raw;  
    TLweSample ** ks; ///< the keyswitch elements: a n.t matrix

    TLweKeySwitchKey(int32_t n, int32_t t, int32_t basebit, const TLweParams* out_params, TLweSample* ks0_raw);
    ~TLweKeySwitchKey();
    TLweKeySwitchKey(const TLweKeySwitchKey&) = delete;
    void operator=(const TLweKeySwitchKey&) = delete;
};

TLweKeySwitchKey* alloc_TLweKeySwitchKey();

void free_TLweKeySwitchKey(TLweKeySwitchKey* ptr);

void init_TLweKeySwitchKey(TLweKeySwitchKey* obj, int n, int t, int basebit, const TLweParams* out_params);

void destroy_TLweKeySwitchKey(TLweKeySwitchKey* obj);

//allocates and initialize the TLweKeySwitchKey structure
//(equivalent of the C++ new)

TLweKeySwitchKey* new_TLweKeySwitchKey(int n, int t, int basebit, const TLweParams* out_params);


//destroys and frees the TLweKeySwitchKey structure
//(equivalent of the C++ delete)

void delete_TLweKeySwitchKey(TLweKeySwitchKey* obj);

#endif //TLWEKEYSWITCH_H
