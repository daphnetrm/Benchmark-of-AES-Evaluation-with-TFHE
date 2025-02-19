#include "tlwe-functions-extra.h"
#include <iostream>
#include <cassert>

/**
 * fills the KeySwitching key array
 * @param result The (n x t x base) array of samples.
 *    result[i][j][k] encodes k.s[i]/base^(j+1)
 * @param out_key The TLwe key to encode all the output samples
 * @param out_alpha The standard deviation of all output samples
 * @param in_key The (binary) input Lwe key
 * @param n The size of the input key
 * @param t The precision of the keyswitch (technically, 1/2.base^t)
 * @param basebit Log_2 of base
 * @param N degree of the polynomials of output samples
 * @param k number of polynomials of output samples
 */

void TLweFunctionsExtra::CreateKeySwitchKey_fromArray(
    TLweSample** result,
    const TLweKey* out_key,
    const double out_alpha,
    const int* in_key,
    const int n,
    const int t,
    const int basebit)
{

  //#pragma omp parallel for
    for(int i=0;i<n;i++) {
        for(int j=0;j<t;j++){
            int32_t x=in_key[i]*(int32_t(1)<<(32-(j+1)*basebit));
            // std::cout << "i*j " << i*j << '\n';
            tLweSymEncryptT(&result[i][j],x,out_alpha,out_key);
            // printf("i,j,k,ki,x,phase=%d,%d,%d,%d,%d,%d\n",i,j,k,in_key->key[i],x,lwePhase(&result->ks[i][j][k],out_key));
        }
    }
}

/**
 * translates the message of the result sample by -sum(a[i].s[i]) where s is the secret
 * embedded in ks.
 * @param result the TLWE sample to translate by -sum(ai.si).
 * @param ks The (n x t x base) key switching key
 *    ks[i][j][k] encodes k.s[i]/base^(j+1)
 * @param params The common TLwe parameters of ks and result
 * @param ai The input torus array
 * @param n The size of the input key
 * @param t The precision of the keyswitch (technically, 1/2.base^t)
 * @param basebit Log_2 of base
 */

void TLweFunctionsExtra::KeySwitchTranslate_fromArray(
    TLweSample* result,
    const TLweSample** ks,
    const TLweParams* params,
    const int32_t* ai,
    const int n,
    const int t,
    const int basebit)
{
    const int base=1<<basebit;     // base=2 in [CGGI16]
    const int32_t prec_offset=int32_t(1)<<(32-(1+basebit*t)); //precision
    const int mask=base-1;

    for (int i=0;i<n;i++){
      // std::cout << "i ---------- " << i << '\n';
        const int64_t aibar=ai[i]+prec_offset;
        for (int j=0;j<t;j++){
            // std::cout << "j " << j << '\n';
            const int64_t aij=(aibar>>(32-(j+1)*basebit)) & mask;
            tLweSubMulTo(result,aij,&ks[i][j],params);
        }
    }
}


void TLweFunctionsExtra::CreateKeySwitchKey(
    TLweKeySwitchKey* result,
    const LweKey* in_key,
    const TLweKey* out_key)
{
    const int n=result->n;
    const int basebit=result->basebit;
    const int t=result->t;

    //TODO check the parameters

    CreateKeySwitchKey_fromArray(
        result->ks,
        out_key, out_key->params->alpha_min,
        in_key->key, n, t, basebit);
}

// The original key switch used for instance in the Hopfield paper, returns the TLWE sample as a TRLWE sample

void TLweFunctionsExtra::KeySwitch(
    TLweSample * result,
    const TLweKeySwitchKey * ks,
    const LweSample* sample)
{
    const TLweParams* params=ks->out_params;
    const int n=ks->n;
    const int basebit=ks->basebit;
    const int t=ks->t;

    tLweClear(result, params);
    result->b->coefsT[0] = sample->b;

    // std::cout << "n " << n << '\n';

    TLweFunctionsExtra::KeySwitchTranslate_fromArray(
        result,
        (const TLweSample**) ks->ks, params,
        sample->a, n, t, basebit);
}

// An optimized version of the keyswitch over the identity function f

void TLweFunctionsExtra::KeySwitch_Id(
    TLweSample * result,
    const TLweKeySwitchKey* ks,
    LweSample ** samples,
    const int M)
{
    const TLweParams * params = ks->out_params;
    const int n = ks->n;
    const int basebit=ks->basebit;
    const int t=ks->t;
    const int N = params->N;

    // Notations
    /*
    / c1,..,c_M are the TLWE inputs with c_i = (a_i,b_i)
    / c' = (a',b') is the TRLWE result of the functional keyswitch
    / f is the T^M -> T_N[X] morphism. In this case f is the identity function
    / see the 2018 TFHE paper for the specifics
    */

    // Check that we can actually do the key switch
    if (M > N)
    {
      std::cout << "ERROR: Number of inputs for the keyswitch too big" << '\n';
      return;
    }

    // c' = (0,0)
    tLweClear(result, params);

    /*
    / c' = (0, f(b_1,..,b_M))
    / Therefore for the first M coefficients of result->b,
    / we give them the b's from the samples in order
    */
    for (int i = 0; i < M; i++)
      result->b->coefsT[i] = samples[i]->b;

    /*
    / We create as, an array of exactly n polynomials
    / The first M coefficients of the polynomial #i are filled with
    / the ith value from all the samples taken in order.
    */
    int32_t ** as = (int32_t **) malloc(sizeof(int32_t*)*n);
    for (int i = 0; i < n; i++)
      as[i] = (int32_t*) malloc(sizeof(int32_t)*M);
    // const int32_t ** as = (const int32_t **) malloc(sizeof(samples[0]->a)*n);
    for (int i = 0; i < n; i++)
      for (int j = 0; j < M; j++)
        as[i][j] = samples[j]->a[i];
    printf("VERIFICATION\n");
    TLweFunctionsExtra::KeySwitchTranslate_fromArray_Generic(
        result,
        (const TLweSample **) ks->ks,
        params,
        (const int32_t **) as, n, t, basebit, M);

    // Clean up
    free(as);
}

// An optimized version of the keyswitch over the identity function f but for SEAL

void TLweFunctionsExtra::SEALKeySwitch_Id(
    TLweSample * result,
    const TLweKeySwitchKey* ks,
    LweSample ** samples,
    const int M)
{
    const TLweParams * params = ks->out_params;
    const int n = ks->n;
    const int basebit=ks->basebit;
    const int t=ks->t;
    const int N = params->N;

    // Notations
    /*
    / c1,..,c_M are the TLWE inputs with c_i = (a_i,b_i)
    / c' = (a',b') is the TRLWE result of the functional keyswitch
    / f is the T^M -> T_N[X] morphism. In this case f is the identity function
    / see the 2018 TFHE paper for the specifics
    */

    // Check that we can actually do the key switch
    if (M > N)
    {
      std::cout << "ERROR: Number of inputs for the keyswitch too big" << '\n';
      return;
    }

    // c' = (0,0)
    tLweClear(result, params);

    /*
    / c' = (0, f(b_1,..,b_M))
    / Therefore for the first M coefficients of result->b,
    / we give them the b's from the samples in order
    */
    for (int i = 0; i < M; i++)
      result->b->coefsT[i] = samples[i]->b;

    /*
    / We create as, an array of exactly n polynomials
    / The first M coefficients of the polynomial #i are filled with
    / the ith value from all the samples taken in order.
    */
    int32_t ** as = (int32_t **) malloc(sizeof(int32_t*)*n);
    for (int i = 0; i < n; i++)
      as[i] = (int32_t*) malloc(sizeof(int32_t)*M);

    for (int i = 0; i < n; i++)
      for (int j = 0; j < M; j++)
        as[i][j] = samples[j]->a[i];

    TLweFunctionsExtra::SEALKeySwitchTranslate_fromArray_Generic(
        result,
        (const TLweSample **) ks->ks,
        params,
        (const int32_t **) as, n, t, basebit, M);

    // Clean up
    free(as);
}

// A general matrix multiplication key switch

void TLweFunctionsExtra::KeySwitch_Matrix_Mul(
    TLweSample * result,
    const TLweKeySwitchKey* ks,
    LweSample ** samples,
    uint32_t ** matrix,
    const int M)
{
    const TLweParams * params = ks->out_params;
    const int n = ks->n;
    const int basebit=ks->basebit;
    const int t=ks->t;
    const int N = params->N;

    // Notations
    /*
    / c1,..,c_M are the TLWE inputs with c_i = (a_i,b_i)
    / c' = (a',b') is the TRLWE result of the functional keyswitch
    / f is the T^M -> T_N[X] morphism. In this case f is a matrix multiplication.
    / see the 2018 TFHE paper for the specifics
    */

    // Check that we can actually do the key switch
    if (M > N)
    {
      std::cout << "ERROR: Number of inputs for the keyswitch too big" << '\n';
      return;
    }

    // c' = (0,0)
    tLweClear(result, params);

    // c' = (0, f(b_1,..,b_M))
    int32_t temp;
    for (int i = 0; i < M; i++)
    {
      temp = 0;
      for (int j = 0; j < M; j++)
        temp += matrix[i][j] * samples[j]->b;
      result->b->coefsT[i] = temp;
    }

    // Apply (as_1,..,as_n) = ( f[a_1^(1), .., a_1^(M)], ..., f[a_n^(1), .., a_n^(M)] )
    int32_t ** as = (int32_t **) malloc(sizeof(int32_t*)*n);
    for (int i = 0; i < n; i++)
      as[i] = (int32_t*) malloc(sizeof(int32_t)*M);

    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < M; j++)
      {
        // At this point we want to compute as[i][j]
        // We need the approrpiate scalar product within the matrix
        temp = 0;
        for (int k = 0; k < M; k++)
          temp += samples[k]->a[i]*matrix[j][k];
        as[i][j] = temp;
      }
    }

    TLweFunctionsExtra::KeySwitchTranslate_fromArray_Generic(
        result,
        (const TLweSample **) ks->ks,
        params,
        (const int32_t **) as, n, t, basebit, M);

    // Clean up
    free(as);
}



// AJOUT
void TLweFunctionsExtra::torusPolynomialMulByXai(TorusPolynomial *result, int32_t a, const TorusPolynomial *source) {
    const int32_t N = source->N;
    Torus32 *out = result->coefsT;
    Torus32 *in = source->coefsT;

    assert(a >= 0 && a < 2 * N);

    if (a < N) {
        for (int32_t i = 0; i < a; i++)//sur que i-a<0
            out[i] = -in[i - a + N];
        for (int32_t i = a; i < N; i++)//sur que N>i-a>=0
            out[i] = in[i - a];
    } else {
        const int32_t aa = a - N;
        for (int32_t i = 0; i < aa; i++)//sur que i-a<0
            out[i] = in[i - aa + N];
        for (int32_t i = aa; i < N; i++)//sur que N>i-a>=0
            out[i] = -in[i - aa];
    }
}


void TLweFunctionsExtra::tLweMulByXai(TLweSample *result, int32_t ai, const TLweSample *bk, const TLweParams *params) {
    const int32_t k = params->k;
    for (int32_t i = 0; i <= k; i++)
        torusPolynomialMulByXaiMinusOne(&result->a[i], ai, &bk->a[i]);
}



//FIN AJOUT 

void TLweFunctionsExtra::KeySwitchTranslate_fromArray_Generic(
    TLweSample * result,
    const TLweSample ** ks,
    const TLweParams * params,
    const int32_t ** as,
    const int n,
    const int t,
    const int basebit,
    const int M)
{
    const int base=1<<basebit;
    const int32_t prec_offset=int32_t(1)<<(32-(1+basebit*t)); //precision
    const int mask=base-1;

    TLweSample * temp = new_TLweSample(params);

    ;

    /*
    / There are n as polynomials (index i)
    / Each of these binary polynomials has M coefficients (index j)
    / We decompose each of them into t binary polynomials (index p)
    /
    */

    // Here we can maybe optimize by building an integer polynomial
    // aij from its coefficients aijp and multiplying it directly
    // to &ks[i][j]. Then just subtract it from result.
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < M; j++)
      {
        // Approximate all the coefficients of
        const int64_t aijbar = as[i][j]+prec_offset;

        for (int p = 0; p < t; p++)
        {
          const int64_t aijp = (aijbar>>(32-(p+1)*basebit)) & mask;

          tLweMulByXai(temp, j, &ks[i][p], params);                              // KS_{i,p} * X^j
          tLweSubMulTo(result, aijp, temp, params);                              // KS_{i,p} * aijp * X^j
        }
      }
    }

    // Clean up
    delete temp;
}


void TLweFunctionsExtra::SEALKeySwitchTranslate_fromArray_Generic(
    TLweSample * result,
    const TLweSample ** ks,
    const TLweParams * params,
    const int32_t ** as,
    const int n,
    const int t,
    const int basebit,
    const int M)
{
    const int base=1<<basebit;
    const int32_t prec_offset=int32_t(1)<<(32-(1+basebit*t)); //precision
    const int mask=base-1;

    TLweSample * temp = new_TLweSample(params);

    ;

    /*
    / There are n as polynomials (index i)
    / Each of these binary polynomials has M coefficients (index j)
    / We decompose each of them into t binary polynomials (index p)
    /
    */

    // Here we can maybe optimize by building an integer polynomial
    // aij from its coefficients aijp and multiplying it directly
    // to &ks[i][j]. Then just subtract it from result.
    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < M; j++)
      {
        // Approximate all the coefficients of
        const int64_t aijbar = as[i][j]+prec_offset;

        for (int p = 0; p < t; p++)
        {
          const int64_t aijp = (aijbar>>(32-(p+1)*basebit)) & mask;

          tLweMulByXai(temp, j, &ks[i][p], params);                              // KS_{i,p} * X^j
          tLweAddMulTo(result, aijp, temp, params);                              // result += KS_{i,p} * aijp * X^j
        }
      }
    }

    // Clean up
    delete temp;
}

void TLweFunctionsExtra::KeySwitchTranslate_fromArray_Special(
    TLweSample * result,
    const TLweSample ** ks,
    const TLweParams * params,
    const int32_t ** as,
    const int n,
    const int t,
    const int basebit,
    const int M1,
    const int M2)
{
    const int base=1<<basebit;
    const int32_t prec_offset=int32_t(1)<<(32-(1+basebit*t)); //precision
    const int mask=base-1;

    TLweSample * temp = new_TLweSample(params);

    for (int i = 0; i < n; i++)
    {
      for (int j = 0; j < M2; j++)
      {
        // Approximate all the coefficients of
        const int64_t aijbar = as[i][j]+prec_offset;

        for (int p = 0; p < t; p++)
        {
          const int64_t aijp = (aijbar>>(32-(p+1)*basebit)) & mask;

          tLweMulByXai(temp, j*M1, &ks[i][p], params);                              // KS_{i,p} * X^(j*M1)
          tLweSubMulTo(result, aijp, temp, params);                              // KS_{i,p} * aijp * X^(j*M1)
        }
      }
    }

    // Clean up
    delete temp;
}

void TLweFunctionsExtra::KeySwitch_for_mult(
    TLweSample * result1,
    TLweSample * result2,
    const TLweKeySwitchKey* ks,
    LweSample ** samples1,
    LweSample ** samples2,
    const int M1,
    const int M2)
{
	int i, j;
    const TLweParams * params = ks->out_params;
    const int n = ks->n;
    const int basebit=ks->basebit;
    const int t=ks->t;
    const int N = params->N;
    
    if (M1*M2 > N)
    {
      std::cout << "ERROR: Number of inputs for the keyswitch too big" << '\n';
      return;
    }

    tLweClear(result1, params);
    tLweClear(result2, params);

    for (i = 0; i < M1; i++)
    {
		result1->b->coefsT[i] = samples1[i]->b;
	}
    
    for (i = 0; i < M2; i++)
    {
		result2->b->coefsT[i*M1] = samples2[i]->b;
	}

    int32_t ** as1 = (int32_t **) malloc(sizeof(int32_t*)*n);
    int32_t ** as2 = (int32_t **) malloc(sizeof(int32_t*)*n);
    for (i = 0; i < n; i++)
    {
		as1[i] = (int32_t*) malloc(sizeof(int32_t)*M1);
		as2[i] = (int32_t*) malloc(sizeof(int32_t)*M2);
	}
    for (i = 0; i < n; i++)
    {
		for (j = 0; j < M1; j++)
		{
			as1[i][j] = samples1[j]->a[i];
		}
		for (j = 0; j < M2; j++)
		{
			as2[i][j] = samples2[j]->a[i];
		}
	}

    TLweFunctionsExtra::KeySwitchTranslate_fromArray_Generic(
        result1,
        (const TLweSample **) ks->ks,
        params,
        (const int32_t **) as1, n, t, basebit, M1);
    
    TLweFunctionsExtra::KeySwitchTranslate_fromArray_Special(
        result2,
        (const TLweSample **) ks->ks,
        params,
        (const int32_t **) as2, n, t, basebit, M1, M2);

    // Clean up
    for (i = 0; i < n; i++)
    {
		free(as1[i]);
		free(as2[i]);
	}
    free(as1);
    free(as2);
}
