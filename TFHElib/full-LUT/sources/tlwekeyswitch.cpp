#include "tlwekeyswitch.h"


TLweKeySwitchKey::TLweKeySwitchKey(
        const int n,
        const int t,
        const int basebit,
        const TLweParams* out_params,
        TLweSample* ks0_raw)
        :
        n(n),
        t(t),
        basebit(basebit),
        base(1<<basebit),
        out_params(out_params),
        ks0_raw(ks0_raw)
{
    ks = new TLweSample*[n];
    for (int p = 0; p < n; ++p)
        ks[p] = ks0_raw + t*p;
}


TLweKeySwitchKey::~TLweKeySwitchKey() {
    delete[] ks;
}


void init_TLweKeySwitchKey(TLweKeySwitchKey* obj, int n, int t, int basebit, const TLweParams* out_params) {
    const int base=1<<basebit;
    TLweSample* ks0_raw = new_TLweSample_array(n*t*base, out_params);

    new(obj) TLweKeySwitchKey(n,t,basebit,out_params, ks0_raw);
}


void destroy_TLweKeySwitchKey(TLweKeySwitchKey* obj) {
    const int n = obj->n;
    const int t = obj->t;
    const int base = obj->base;
    delete_TLweSample_array(n*t*base,obj->ks0_raw);

    obj->~TLweKeySwitchKey();
}



TLweKeySwitchKey* alloc_TLweKeySwitchKey() {
    return (TLweKeySwitchKey*) malloc(sizeof(TLweKeySwitchKey));
}

void free_TLweKeySwitchKey(TLweKeySwitchKey* ptr) {
    free(ptr);
}
 
TLweKeySwitchKey* new_TLweKeySwitchKey(int n, int t, int basebit, const TLweParams* out_params) {
    TLweKeySwitchKey* obj = alloc_TLweKeySwitchKey();
    init_TLweKeySwitchKey(obj, n,t,basebit,out_params);
    return obj;
}

void delete_TLweKeySwitchKey(TLweKeySwitchKey* obj) {
    destroy_TLweKeySwitchKey(obj);
    free_TLweKeySwitchKey(obj);
}
