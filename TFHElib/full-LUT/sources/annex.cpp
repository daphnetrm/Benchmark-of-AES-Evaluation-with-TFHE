#include "annex.h"
#include "bootstrapping.h"
#include <omp.h>



using namespace std;




void XOR_fhe(vector <LweSample*> &v1,
	     vector <LweSample*> &v2,
	     TFheGateBootstrappingSecretKeySet* gk,
	     BaseBKeySwitchKey* ks_key) {
  vector <LweSample*> d0;
  vector <LweSample*> d1;
  vector <LweSample*> res0 (1);
  vector <LweSample*> res1 (1);
  d0.push_back(v1[0]);
  d0.push_back(v2[0]);
  d1.push_back(v1[1]);
  d1.push_back(v2[1]);
  res0[0] = new_LweSample(gk->lwe_key->params);
  deref_boot_single(res0, gk, d0, ks_key, XOR_b16);
  res1[0] = new_LweSample(gk->lwe_key->params);
  deref_boot_single(res1, gk, d1, ks_key, XOR_b16);
  res0.push_back(res1[0]);
  v1.swap(res0);
  d0.clear();
  d1.clear();
  res0.clear();
  res1.clear();
}




void print_coef_b16(TorusPolynomial *testv) {
  for(int i = 0; i<16; ++i)
    printf("coeff[%d] = %d  \n", i , (int) ((t32tod(testv->coefsT[i*64]))*16+16)%16);
  printf("\n");
}


void print_int_b16(IntPolynomial *testv) {
  float norme = 0;
  for(int i = 0; i<16; ++i){
    printf("coeff[%d] = %d  \n", i ,testv->coefs[i*64]);
    norme += testv->coefs[i*64] * testv->coefs[i*64];
  }
  printf("\n");
}


void print_testv(TorusPolynomial *testv) {
  for(int i = 0; i < testv->N; ++i)
    printf("%fX^%d + ", t32tod(testv->coefsT[i]), i);
  printf("\n");			
}



void ks_batching(int i,
		 uint8_t B,
		 vector <LweSample*> &resLwe,
		 vector<TLweSample*> &resTLwe,
		 const TLweKey * k_out ,
		 BaseBKeySwitchKey* ks_key) {
      resTLwe[i/B] = new_TLweSample(k_out->params);
      BaseBExtra::KeySwitch_Id(resTLwe[i/B], ks_key, resLwe);
      for(int m = 0; m<B; ++m)
	delete_LweSample(resLwe[m]);
}



void tLweMulByXai(TLweSample *result,
		  int32_t ai,
		  const TLweSample *bk,
		  const TLweParams *params) {
    const int32_t k = params->k;
    for (int32_t i = 0; i <= k; i++)
        torusPolynomialMulByXai(&result->a[i], ai, &bk->a[i]);
}




void Enc_tab (vector <LweSample*> tab_fhe[4][8],
	      word8 tab[4][8],
	      TFheGateBootstrappingSecretKeySet* key) {
  double alpha = key->lwe_key->params->alpha_min;
  for(int i = 0; i<4; ++i){
    for(int j = 0; j<8; ++j){
      tab_fhe[i][j].push_back(new_LweSample(key->lwe_key->params));
      lweSymEncrypt(tab_fhe[i][j][0], modSwitchToTorus32(tab[i][j]%16, 32), alpha, key->lwe_key);
      tab_fhe[i][j].push_back(new_LweSample(key->lwe_key->params));
      lweSymEncrypt(tab_fhe[i][j][1], modSwitchToTorus32(tab[i][j]/16, 32), alpha, key->lwe_key);
    }
  }
}


