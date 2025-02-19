#ifndef TLWE_FUNCTIONS_EXTRA_H
#define TLWE_FUNCTIONS_EXTRA_H

///@file
///@brief This file contains the operations on Lwe samples

#include "tlwekeyswitch.h"

struct TLweFunctionsExtra
{
public:
    static void CreateKeySwitchKey_fromArray(
        TLweSample** result,
        const TLweKey* out_key,
        const double out_alpha,
        const int* in_key,
        const int n,
        const int t,
        const int basebit);

    static void KeySwitchTranslate_fromArray(
        TLweSample* result,
        const TLweSample** ks,
        const TLweParams* params,
        const Torus32* ai,
        const int n,
        const int t,
        const int basebit);

    static void KeySwitchTranslate_fromArray_Generic(
        TLweSample * result,
        const TLweSample ** ks,
        const TLweParams * params,
        const Torus32 ** as,
        const int n,
        const int t,
        const int basebit,
        const int M);

    static void SEALKeySwitchTranslate_fromArray_Generic(
        TLweSample * result,
        const TLweSample ** ks,
        const TLweParams * params,
        const Torus32 ** as,
        const int n,
        const int t,
        const int basebit,
        const int M);
        
    static void KeySwitchTranslate_fromArray_Special(
		TLweSample * result,
		const TLweSample ** ks,
		const TLweParams * params,
		const int32_t ** as,
		const int n,
		const int t,
		const int basebit,
		const int M1,
		const int M2);

    /**
     * creates a Key Switching Key between the two keys
     */
    static void CreateKeySwitchKey(
        TLweKeySwitchKey* result,
        const LweKey* in_key,
        const TLweKey* out_key);

    /**
     * applies keySwitching
     */
    static void KeySwitch(
        TLweSample* result,
        const TLweKeySwitchKey* ks,
        const LweSample* sample);

    static void KeySwitch_Id(
        TLweSample * result,
        const TLweKeySwitchKey* ks,
        LweSample ** samples,
        const int M);

    static void SEALKeySwitch_Id(
        TLweSample * result,
        const TLweKeySwitchKey* ks,
        LweSample ** samples,
        const int M);

    static void KeySwitch_Matrix_Mul(
      TLweSample * result,
      const TLweKeySwitchKey* ks,
      LweSample ** samples,
      uint32_t ** matrix,
      const int M);
    
    static void KeySwitch_for_mult(
    TLweSample * result1,
    TLweSample * result2,
    const TLweKeySwitchKey* ks,
    LweSample ** samples1,
    LweSample ** samples2,
    const int M1,
    const int M2);

	static void torusPolynomialMulByXai(TorusPolynomial *result, int32_t a, const TorusPolynomial *source);

	static void tLweMulByXai(TLweSample *result, int32_t ai, const TLweSample *bk, const TLweParams *params);


};

#endif //TLWE_FUNCTIONS_EXTRA_H
