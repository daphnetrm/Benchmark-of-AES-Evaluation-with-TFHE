#ifndef H_AES
#define H_AES

#include <stdio.h>
#include <tfhe.h>
#include <tfhe_io.h>
#include <vector>
#include "tables.h"
#include "annex.h"



void InvMixColumns(word8 a[4][MAXBC],
		   int BC);

int KeyExpansion (word8 k[4][MAXKC],
		  word8 W[MAXROUNDS+1][4][MAXBC],
		  int BC,
		  int KC,
		  int ROUNDS);

int Decrypt (word8 a[4][MAXBC],
	     word8 rk[MAXROUNDS+1][4][MAXBC],
	     int BC,
	     int ROUNDS);


void AddRoundKey_fhe(vector <LweSample*> a[4][MAXBC],
		     vector <LweSample*> rk[4][MAXBC],
		     TFheGateBootstrappingSecretKeySet* gk,
		     BaseBKeySwitchKey* ks_key);

void SubBytes_fhe(vector <LweSample*> a[4][MAXBC],
		  TFheGateBootstrappingSecretKeySet* gk,
		  BaseBKeySwitchKey* ks_key);

void ShiftRows_fhe(vector <LweSample*> a[4][MAXBC],
		   word8 d,
		   int BC,
		   TFheGateBootstrappingSecretKeySet* gk);

void MixColumns_fhe( vector <LweSample*> a[4][MAXBC],
		     TFheGateBootstrappingSecretKeySet* gk,
		     BaseBKeySwitchKey* ks_key);

int Encrypt_fhe(vector <LweSample*> a[4][MAXBC],
		vector <LweSample*> rk[MAXROUNDS+1][4][MAXBC],
		int ROUNDS,
		TFheGateBootstrappingSecretKeySet* gk,
		BaseBKeySwitchKey* ks_key);

   
#endif
