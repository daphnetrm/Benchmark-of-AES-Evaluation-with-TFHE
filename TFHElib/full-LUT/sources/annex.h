#ifndef H_ANNEXE
#define H_ANNEXE

#include <tfhe.h>
#include <tfhe_io.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <cassert>
#include <ctime>

#include "base_b_keyswitchkey.h"
#include "base_b_keyswitch.h"
#include "tables.h"
#include "aes.h"

using namespace std; 


void print_coef_b16(TorusPolynomial *testv);

void print_int_b16(IntPolynomial *testv);

void print_testv(TorusPolynomial *testv);

void ks_batching(int i,
		 uint8_t B,
		 vector <LweSample*> &resLwe,
		 vector<TLweSample*> &resTLwe,
		 const TLweKey * k_out ,
		 BaseBKeySwitchKey* ks_key);

void tLweMulByXai(TLweSample *result, int32_t ai,
		  const TLweSample *bk,
		  const TLweParams *params);

void XOR_fhe(vector <LweSample*> &v1,
	     vector <LweSample*> &v2,
	     TFheGateBootstrappingSecretKeySet* gk,
	     BaseBKeySwitchKey* ks_key);

void Enc_tab(vector <LweSample*> tab_fhe[4][8] ,
	     word8 tab[4][8],
	     TFheGateBootstrappingSecretKeySet* key);


   
#endif
