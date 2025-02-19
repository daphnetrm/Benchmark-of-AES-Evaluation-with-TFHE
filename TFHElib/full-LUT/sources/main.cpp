#include <tfhe.h>
#include <tfhe_io.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <time.h>
#include <tfhe_garbage_collector.h>

#include "aes.h"


using namespace std; 

int BC, KC, ROUNDS;


int main() {
  printf("Keys generation...\n");
  BC =4;
  KC =4;
  const int minimum_lambda = 110;
  static const int32_t N = 2048;
  static const int32_t k = 1;
  static const int32_t n = 1024;
  static const int32_t bk_l = 3;
  static const int32_t bk_Bgbit = 8;
  static const int32_t ks_basebit = 10;
  static const int32_t ks_length = 2;
  static const double ks_stdev = pow(5.6,-8);//standard deviation
  static const double bk_stdev = pow(9.6,-11);//standard deviation
  static const double max_stdev = 0.012467; //max standard deviation for a 1/4 msg space
  LweParams *params_in = new_LweParams(n, ks_stdev, max_stdev);
  TLweParams *params_accum = new_TLweParams(N, k, bk_stdev, max_stdev);
  TGswParams *params_bk = new_TGswParams(bk_l, bk_Bgbit, params_accum);

  TfheGarbageCollector::register_param(params_in);
  TfheGarbageCollector::register_param(params_accum);
  TfheGarbageCollector::register_param(params_bk);
  
  TFheGateBootstrappingParameterSet* params = new TFheGateBootstrappingParameterSet(ks_length, ks_basebit, params_in, params_bk);
  uint32_t seed[] = {314, 1592, 657, 26363, 394, 4958, 4059, 3845};
  tfhe_random_generator_setSeed(seed, 8);
  TFheGateBootstrappingSecretKeySet* key = new_random_gate_bootstrapping_secret_keyset(params);
  const LweKey * k_in = key->lwe_key;
  const TLweKey * k_out = &key->tgsw_key->tlwe_key;
  BaseBKeySwitchKey* ks_key = new_BaseBKeySwitchKey( key->lwe_key->params->n, 2, 10, 16, key->cloud.bk->accum_params);
  BaseBExtra::CreateKeySwitchKey(ks_key, k_in, k_out);
  printf("Done ! Now AES execution\n");
  word8 a[4][8] = {{0x00, 0x44, 0x88, 0xcc, 0x00, 0x00, 0x00, 0x00},
		   {0x11, 0x55, 0x99, 0xdd, 0x00, 0x00, 0x00, 0x00},
		   {0x22, 0x66, 0xaa, 0xee, 0x00, 0x00, 0x00, 0x00},
		   {0x33, 0x77, 0xbb, 0xff, 0x00, 0x00, 0x00, 0x00}};
  
  word8 sk[4][8] = {{0x00, 0x04, 0x08, 0x0c, 0x00, 0x00, 0x00, 0x00},
		    {0x01, 0x05, 0x09, 0x0d, 0x00, 0x00, 0x00, 0x00},
		    {0x02, 0x06, 0x0a, 0x0e, 0x00, 0x00, 0x00, 0x00},
		    {0x03, 0x07, 0x0b, 0x0f, 0x00, 0x00, 0x00, 0x00}};

  printf("a    = ");
  for(int i = 0; i < 4; ++i){
    for(int j = 0; j < 4; ++j)
      printf("%.2x", a[j][i]);
  }
  printf("\n");
  int base = 16;
  word8 rk[MAXROUNDS+1][4][8];
  ROUNDS = 10;
  KeyExpansion(sk,rk, 4, 4, ROUNDS);
  
  vector <LweSample*> rk_fhe[MAXROUNDS+1][4][8];
  for(int i = 0; i<MAXROUNDS+1 ; ++i)
     Enc_tab(rk_fhe[i] , rk[i], key);

  vector <LweSample*> a_fhe[4][8];
  Enc_tab(a_fhe, a, key);

  double elapsed=0;
  float acc=0;

  struct timespec begin, end; 
  clock_gettime(CLOCK_REALTIME, &begin);
  Encrypt_fhe(a_fhe, rk_fhe, ROUNDS, key, ks_key);
  clock_gettime(CLOCK_REALTIME, &end);
  long seconds = end.tv_sec - begin.tv_sec;
  long nanoseconds = end.tv_nsec - begin.tv_nsec;
  elapsed = seconds + nanoseconds*1e-9;
  acc+=elapsed;

 

  for(int i = 0; i<4; ++i){
    for(int j = 0; j<8; ++j){
    int32_t decr0 = lweSymDecrypt(a_fhe[i][j][0], key->lwe_key, 32);
    double decrd0 = t32tod(decr0);
    int32_t decr1 = lweSymDecrypt(a_fhe[i][j][1], key->lwe_key, 32);
    double decrd1 = t32tod(decr1);
    a[i][j]= (int)(decrd0*32+base)%base + (int)(decrd1*32+base)%base * base;
    }    
  }
  printf("[a]  = ");
  for(int i = 0; i < 4; ++i){
    for(int j = 0; j < 4; ++j)
      printf("%.2x", a[j][i]);
  }
   
  delete_gate_bootstrapping_secret_keyset(key);
  delete_gate_bootstrapping_parameters(params);
  delete_BaseBKeySwitchKey(ks_key);
  printf("\nverif: 69c4e0d86a7b0430d8cdb78070b4c55a\n");
  printf("timing : %.5f\n", elapsed);

  

  return 0;
}

