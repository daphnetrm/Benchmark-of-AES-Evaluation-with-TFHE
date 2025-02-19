#ifndef H_TABLES
#define H_TABLES

#include <stdio.h>
#include <tfhe.h>
#include <tfhe_io.h>

#include "base_b_keyswitchkey.h"
#include "base_b_keyswitch.h"

typedef unsigned char word8;
typedef unsigned int word32;
using std::vector;

extern int BC, KC, ROUNDS;

#define MAXBC 8
#define MAXKC 8
#define MAXROUNDS 14




/* Used to determine the number of rounds.
 * Depends on N_b and N_k.
 */ 
static int numrounds[5][5] = {
			      {10, 11, 12, 13, 14},
			      {11, 11, 12, 13, 14},
			      {12, 12, 12, 13, 14},
			      {13, 13, 13, 13, 14},
			      {14, 14, 14, 14, 14}};


/*Used in ShiftRows
 * Sift offsets for different block lenhgts (N_b in  {4, 5, 6, 7, 8}).
 */
static word8 shifts[5][4] = {
			     {0, 1, 2, 3},
			     {0, 1, 2, 3},
			     {0, 1, 2, 3},
			     {0, 1, 2, 4},
			     {0, 1, 3, 4}};



extern word8 Alogtable[256];
extern word8 S[256];
extern word8 Si[256];
extern word32 RC[30];
extern word8 S_0_b16[256];
extern word8 S_1_b16[256];
extern word8 XOR_b16[256];
extern word8 XOR_0_b16[256];
extern word8 XOR_bis[256];
extern word8 *XOR[256];
extern word8 *Sbox[256];


extern word8 and_8[16];
extern word8 and_4[16];
extern word8 and_2[16];
extern word8 and_1[16];


void mul2_fhe (vector <LweSample*> &result, vector <LweSample*> &b, TFheGateBootstrappingSecretKeySet* gk, BaseBKeySwitchKey* ks_key);
void mul3_fhe (vector <LweSample*> &result, vector <LweSample*> &b, TFheGateBootstrappingSecretKeySet* gk, BaseBKeySwitchKey* ks_key);
void mul9_fhe (vector <LweSample*> &result, vector <LweSample*> &b, TFheGateBootstrappingSecretKeySet* gk, BaseBKeySwitchKey* ks_key);
void mulb_fhe (vector <LweSample*> &result, vector <LweSample*> &b, TFheGateBootstrappingSecretKeySet* gk, BaseBKeySwitchKey* ks_key);
void muld_fhe (vector <LweSample*> &result, vector <LweSample*> &b, TFheGateBootstrappingSecretKeySet* gk, BaseBKeySwitchKey* ks_key);
void mule_fhe (vector <LweSample*> &result, vector <LweSample*> &b, TFheGateBootstrappingSecretKeySet* gk, BaseBKeySwitchKey* ks_key);


void print_tables_Tx_b16();
void testv_b16(TorusPolynomial *testv, int32_t N, uint8_t idx, word8 tab[256]);
void testv_vi_b16(IntPolynomial *testv, int32_t N, uint8_t idx, word8 tab[256]);
void test_v0(TorusPolynomial *testv, int32_t N);
void testv_vi_b16_2(IntPolynomial *testv, int32_t N, word8 tab[16]);
void testv_vi_and(IntPolynomial *testv, int32_t N, uint8_t i) ;
void tLweMulByXai(TLweSample *result,
		  int32_t ai,
		  const TLweSample *bk,
		  const TLweParams *params);
void ks_batching(int i,
		 uint8_t B,
		 vector <LweSample*> &resLwe,
		 vector<TLweSample*> &resTLwe,
		 const TLweKey * k_out ,
		 BaseBKeySwitchKey* ks_key);

void testv_and(TorusPolynomial *testv, int32_t N, word8 tab[16]);

   
#endif
