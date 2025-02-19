#ifndef H_BOOTSTRAPPING
#define H_BOOTSTRAPPING

#include <tfhe.h>
#include <tfhe_io.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <cassert>
#include <ctime>
#include <tfhe_garbage_collector.h>


#include "base_b_keyswitchkey.h"
#include "base_b_keyswitch.h"
#include "tlwe-functions-extra.h"
#include "tlwekeyswitch.h"
#include "tables.h"


using namespace std; 



void boot_lut_FFT(TLweSample *result,
	      TLweSample * lut,
	      const LweBootstrappingKeyFFT *bk,
	      int32_t *bara,
	      int32_t barb );

void deref_mvb_single(vector <LweSample*> &result,
	       TFheGateBootstrappingSecretKeySet* gk,
	       vector <LweSample*> &tab_ciphers,
	       uint32_t m_size,
	       uint8_t d, 
	       uint8_t B,
	       BaseBKeySwitchKey* ks_key,
	       word8 tab[256]);

void deref_boot_single(vector <LweSample*> &result,
		TFheGateBootstrappingSecretKeySet* gk,
		vector <LweSample*> &tab_ciphers,
		BaseBKeySwitchKey* ks_key,
		word8 tab[256]);

void deref_simple_boot(vector <LweSample*> &result,
		       TFheGateBootstrappingSecretKeySet* gk,
		       vector <LweSample*> &x,
		       BaseBKeySwitchKey* ks_key,
		       word8 idx_in,
		       word8 idx_out,
		       word8 tab[16]);

void deref_decompo(vector <LweSample*> &result,
		   TFheGateBootstrappingSecretKeySet* gk,
		   vector <LweSample*> &tab_ciphers,
		   BaseBKeySwitchKey* ks_key);

void deref_mvb_decompo(vector <LweSample*> &result,
		       TFheGateBootstrappingSecretKeySet* gk,
		       vector <LweSample*> &tab_ciphers,
		       uint32_t m_size,
		       uint8_t d, 
		       uint8_t B,
		       BaseBKeySwitchKey* ks_key) ;

void deref_boot_opti(vector <LweSample*> &result,
		     TFheGateBootstrappingSecretKeySet* gk,
		     vector <LweSample*> &tab_ciphers,
		     BaseBKeySwitchKey* ks_key,
		     word8* tab[256]);

void deref_mvb_opti(vector <LweSample*> &result,
		    TFheGateBootstrappingSecretKeySet* gk,
		    vector <LweSample*> &tab_ciphers,
		    uint32_t m_size,
		    uint8_t d, 
		    uint8_t B,
		    BaseBKeySwitchKey* ks_key,
		    word8* tab[256] );

void deref_boot_opti_2(vector <LweSample*> &result,
		       TFheGateBootstrappingSecretKeySet* gk,
		       vector <LweSample*> &tab_ciphers,
		       BaseBKeySwitchKey* ks_key,
		       uint8_t tab16_idx,
		       word8** tab);

void deref_mvb_opti_2(vector <LweSample*> &result,
		      TFheGateBootstrappingSecretKeySet* gk,
		      vector <LweSample*> &tab_ciphers,
		      uint32_t m_size,
		      uint8_t d, 
		      uint8_t B,
		      BaseBKeySwitchKey* ks_key,
		      uint8_t tab16_idx,
		      word8** tab);

void deref_simple_boot_opti(vector <LweSample*> &result,
		       TFheGateBootstrappingSecretKeySet* gk,
		       vector <LweSample*> &tab_ciphers,
		       BaseBKeySwitchKey* ks_key,
			    word8* tab[16]);

void deref_mvb_simple_opti(vector <LweSample*> &result,
		      TFheGateBootstrappingSecretKeySet* gk,
		      vector <LweSample*> &tab_ciphers,
		      uint32_t m_size,
		      uint8_t d, 
		      uint8_t B,
		      BaseBKeySwitchKey* ks_key,
			   word8* tab[16]);

void deref_boot_1KS(vector <LweSample*> &result,
		     TFheGateBootstrappingSecretKeySet* gk,
		     vector <LweSample*> &tab_ciphers,
		     BaseBKeySwitchKey* ks_key,
		     word8 tab[256]);

void deref_mvb_1KS(vector <LweSample*> &result,
		    TFheGateBootstrappingSecretKeySet* gk,
		    vector <LweSample*> &tab_ciphers,
		    uint32_t m_size,
		    uint8_t d, 
		    uint8_t B,
		    BaseBKeySwitchKey* ks_key,
		    word8 tab[256] );



#endif
