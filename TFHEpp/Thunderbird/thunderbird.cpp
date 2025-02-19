#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <random>
#include <tfhe++.hpp>
#include <vector>
#include <stdbool.h>
#include <algorithm>

using namespace std;
using namespace TFHEpp;

// Aliases for different parameters
using iksP = TFHEpp::lvl10param;
using brP = TFHEpp::lvl01param;
using bkP = TFHEpp::lvl02param;
using privksP = TFHEpp::lvl21param;
using sqkP = TFHEpp::lvl11param;

// Global variable test
bool test = false;





template <class P>
void CMUXFFTwithPolynomialMulByXaiMinusOne(TRLWE<P> &acc, const TRGSWFFT<P> &cs, const typename P::T a){
    TRLWE<P> temp;
    for (int k = 0; k < P::k + 1; k++)
        PolynomialMulByXaiMinusOne<P>(temp[k], acc[k], a);
    trgswfftExternalProduct<P>(temp, temp,cs);
    for (int k = 0; k < P::k + 1;k++)
        for (int i = 0; i < P::n; i++)
    acc[k][i] += temp[k][i];

}





template <class P>
void BlindRotate_LUT(TRLWE<typename P::targetP> &res, const typename P::targetP::T *a_bar, vector<TFHEpp::TRGSWFFT<typename P::targetP>> &select, int n){
    for (int i = 0; i < n; i++){
        if (a_bar[i] == 0) continue;
            CMUXFFTwithPolynomialMulByXaiMinusOne<typename P::targetP>(res,select[i],a_bar[i]);
//select[i] doit être un TFHEpp::BootstrappingKeyElementFFT<P>&
//a_bar[i] doit être un int
}

}







//=============================//
//             MISC            //
//=============================//

/**
 * Print array of bits in hex.
 */
void print_plain(vector<uint8_t> &block, string header_text)
{
    uint64_t dec_msb = 0;
    uint64_t dec_lsb = 0;

    uint32_t dec_msb_lsb = 0;
    uint32_t dec_lsb_lsb = 0;

    for(int line = 0; line < 128; line += 32) {
        if(line != 96) {
            for(int i = 0; i < 8; i++) {
                dec_msb = (dec_msb + block[i + line]) << 1;
                dec_msb_lsb = (dec_msb_lsb + block[i + line + 8]) << 1;
                dec_lsb = (dec_lsb + block[i + line + 16]) << 1;
                dec_lsb_lsb = (dec_lsb_lsb + block[i + line + 24]) << 1;
            }
        }
        else 
        {
            for(int i = 0; i < 7; i++) {
                dec_msb = (dec_msb + block[i + line]) << 1;
                dec_msb_lsb = (dec_msb_lsb + block[i + line + 8]) << 1;
                dec_lsb = (dec_lsb + block[i + line + 16]) << 1;
                dec_lsb_lsb = (dec_lsb_lsb + block[i + line + 24]) << 1;
            }
            dec_msb = (dec_msb + block[line + 7]);
            dec_msb_lsb = (dec_msb_lsb + block[line + 15]);
            dec_lsb = (dec_lsb + block[line + 23]);
            dec_lsb_lsb = (dec_lsb_lsb + block[line + 31]);
        }
    }

    dec_msb = (dec_msb << 32) + dec_msb_lsb;
    dec_lsb = (dec_lsb << 32) + dec_lsb_lsb;

    cout << header_text << "0x" << std::hex << dec_msb << dec_lsb <<endl;
}

/**
 * Decrypt an array of ciphers of bits and then print in hex.
 */
void print_dec(vector<TLWE<typename iksP::domainP>> &cipher, SecretKey &sk, string header_text) 
{
    vector<uint8_t> dec(cipher.size());
    dec = bootsSymDecrypt(cipher, sk);
    print_plain(dec, header_text);
}

/**
 * Transform a 128-bits  message into a vector of bits.
 * 
 * As 128 bits is usually not supported we split the message into two halves representing msb and lsb
 * 
 * @param res The result vector of size 128
 * @param x_msb The MSB half
 * @param x_lsb The LSB half
 */
void vector_from_int(vector<uint8_t> &res, uint64_t &x_msb, uint64_t &x_lsb) 
{
    int line = 0;
    for(int i = 0; i < 32; i++) {
        if ((i != 0) && ((i % 8) == 0))
            line = (line + 32) % 128;
        res[line + (i % 8)] = (x_msb >> (63 - i)) & 0b1;
        res[line + 8 + (i % 8)] = (x_msb >> (31 - i)) & 0b1;
        res[line + 16 + (i % 8)] = (x_lsb >> (63 - i)) & 0b1;
        res[line + 24 + (i % 8)] = (x_lsb >> (31 - i)) & 0b1;
    }
}

/**
 * Takes 8 bits in an array and transform it into a byte.
 * 
 * @param block An array of bits
 * @param start Start index in `block`
 */
uint8_t bits_to_byte(vector<uint8_t> &block, int start)
{
    uint8_t res = 0;
    for(int j = start; j < start + 7; j++) 
        res = (res + block[j]) << 1;
    res += block[start + 7];
    return res;
}

//=============================//
//          SUB BYTES          //
//=============================//

/** 
 * AES usual S-Box 
 */
static volatile const uint8_t aes_sbox[] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b,
    0xfe, 0xd7, 0xab, 0x76, 0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 0xb7, 0xfd, 0x93, 0x26,
    0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2,
    0xeb, 0x27, 0xb2, 0x75, 0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 0x53, 0xd1, 0x00, 0xed,
    0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f,
    0x50, 0x3c, 0x9f, 0xa8, 0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 0xcd, 0x0c, 0x13, 0xec,
    0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14,
    0xde, 0x5e, 0x0b, 0xdb, 0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d,
    0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f,
    0x4b, 0xbd, 0x8b, 0x8a, 0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 0xe1, 0xf8, 0x98, 0x11,
    0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f,
    0xb0, 0x54, 0xbb, 0x16,
};

/**
 *  Following Fragata, this function packs up SBox two LUTs T0 and T1 on {0,1}.
 * 
 * * We suppose that each pack(TRLWE) to rotate is of degree N = 1024, that's why we choose to pack 128 bytes together. If N != 1024, adapt the slicing.
 * 
 * @param d0,d1 The 2 packs
 * @param sk The Secret Key (We suppose that it is computed by the user)
 */
void pack_sbox(TRLWE<lvl1param> &d0, TRLWE<lvl1param> &d1, SecretKey &sk) {
    // Initialization
    d0 = TRLWE<lvl1param>();
    d1 = TRLWE<lvl1param>();

    // We're going to split S-box in two halves andbinary decompose each term into a table of size 128*8 = 1024
    vector<uint8_t> t0(1024);
    vector<uint8_t> t1(1024);
    for(int i = 0; i < 128; i++) {
        for(int j = 0; j < 8; j++) {
            t0[i * 8 + j] = (aes_sbox[i] >> (7 - j)) & 0b1;
            t1[i * 8 + j] = (aes_sbox[i + 128] >> (7 - j)) & 0b1;
        }
    }

    // Encrypt both tables t0 and t1
    vector<TLWE<lvl1param>> T0{1024};
    vector<TLWE<lvl1param>> T1{1024};
    T0 = bootsSymEncrypt(t0, sk);
    T1 = bootsSymEncrypt(t1, sk);

    // Packing
    AnnihilateKey<lvl1param> ahk{};
    annihilatekeygen<lvl1param>(ahk, sk);
    TLWE2TRLWEPacking<lvl1param>(d0, T0, ahk);
    TLWE2TRLWEPacking<lvl1param>(d1, T1 , ahk);
}

/**
 * Evaluates SBox for 1 byte
 * 
 * According to Fregata : 
 * 
 * - SubBytes is performed at lvl1
 * 
 * - KeySwitch takes a level1 TLWE and outputs a level0 TLWE
 * 
 * @param cipher The state to fill at Level 0
 * @param tgsw An array of tgsw `C0`, ..., `C7` (The bits must have the same order)
 * @param start The start index in cipher
 * @param ek The EvalKey
 */
void lut_mixed_packing(vector<TLWE<typename iksP::domainP>>& cipher,
                       TRLWE<lvl1param> d0, TRLWE<lvl1param> d1,
                       vector<TRGSWFFT<typename privksP::targetP>> &tgsw,
                       size_t start,
                       EvalKey &ek)
{
    TRLWE<typename privksP::targetP> acc{};
    CMUXFFT<typename privksP::targetP>(acc, tgsw[7], d1, d0); // d0 and d1 are at Level 1

    privksP::targetP::T *bara = new privksP::targetP::T[8];
    privksP::targetP::T NX2 = 2 * privksP::targetP::n;
    for (int32_t i = 0; i < 7; i++)
        bara[i] = NX2 - 8 * pow(2, i);
    BlindRotate_LUT<privksP>(acc, bara, tgsw, 7);


    for(int k = 0; k < 8; k++) {
        TLWE<typename iksP::domainP> extracted_lwe{};
        TLWE<typename iksP::targetP> keyswitch_lwe{};
        SampleExtractIndex<typename iksP::domainP>(extracted_lwe, acc, k);
        IdentityKeySwitch<iksP>(keyswitch_lwe, extracted_lwe, *ek.iksklvl10); // Level 1 to 0
        GateBootstrappingTLWE2TLWEFFT<brP>(cipher[start + k], keyswitch_lwe, *ek.bkfftlvl01, μpolygen<lvl1param, lvl1param::μ>());
    }
}

/**
 * SubBytes
 * 
 * Applies `lut_mixed_packing` on all the bytes to compute SubBytes
 * 
 * The algorithm can be found in the slides of Fregata p.15.
 * 
 * @param cipher A Level 1 AES state
 * @param ek EvalKey
 * @return A Level 0 AES state
 */
void sub_bytes(vector<TLWE<typename iksP::domainP>>& res, 
               vector<TRGSWFFT<typename privksP::targetP>>& bootedTGSW, vector<TLWE<lvl1param>>& cipher, 
               TRLWE<lvl1param> d0, TRLWE<lvl1param> d1,
               EvalKey& ek)
{
    for (size_t i = 0; i < cipher.size(); i += 8) {
        vector<TRGSWFFT<typename privksP::targetP>> tgsw{bootedTGSW.begin() + i,bootedTGSW.begin() + i + 8};
        reverse(tgsw.begin(), tgsw.end());
        lut_mixed_packing(res, d0, d1, tgsw, i, ek);
    }
}

/**
 * SubBytes operation in the plaintexts.
 * 
 * It applies SubBytes to the byte at index `start` in `array`
 * 
 * @param array An array of size a multiple of 8 containing bits
 * @param start Start index of the byte
 */
void plain_subbytes(vector<uint8_t> &array, const int start){
    uint8_t index = 0;
    for(int k = 0; k < 7; k++)
        index = (index + array[start + k]) << 1;
    index += array[start + 7];

    uint8_t new_array = aes_sbox[index];
    for(int k = 0; k < 8; k++)
        array[start + k] = (new_array >> (7 - k)) & 0b1;
}

//=============================//
//         SHIFT ROWS          //
//       free to compute       //
//=============================//

void plain_shift_rows(vector<uint8_t> &block){
    std::rotate(block.begin() + 32, block.begin() + 40, block.begin() + 64);
    std::rotate(block.begin() + 64, block.begin() + 80, block.begin() + 96);
    std::rotate(block.begin() + 96, block.begin() + 120, block.end());
}

void shift_rows(vector<TLWE<typename iksP::domainP>>& cipher)
{
    std::rotate(cipher.begin() + 32, cipher.begin() + 40, cipher.begin() + 64);
    std::rotate(cipher.begin() + 64, cipher.begin() + 80, cipher.begin() + 96);
    std::rotate(cipher.begin() + 96, cipher.begin() + 120, cipher.end());
}

//=============================//
//        ADD ROUND KEY        //
//=============================//

/**
 * Array of Rcon values (cf. AES spec.)
 */
static volatile const uint32_t rc[10] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 
    0x20000000, 0x40000000, 0x80000000, 0x1b000000, 0x36000000,
};

/**
 * Initializes all AES-128 Round Keys
 * 
 * @param plain_keys The Array that will store all the keys
 * @param plain_key_msb The 64 MSB of the First Round Key
 * @param plain_key_lsb The 64 LSB of the First Round Key
 */
void init_keys(vector<vector<uint8_t>> &plain_keys, uint64_t &plain_key_msb, uint64_t &plain_key_lsb)
{
    // The first key is the key itself
    vector_from_int(plain_keys[0], plain_key_msb, plain_key_lsb);

    // Compute the next keys
    for(int round = 1; round < 11; round++) 
    {
        // Apply RotWord to the last column
        vector<uint8_t> new_w4(32);
        for(int i = 0; i < 8; i++) {
            new_w4[i]      = plain_keys[round - 1][24 + i];
            new_w4[i + 8]  = plain_keys[round - 1][56 + i];
            new_w4[i + 16] = plain_keys[round - 1][88 + i];
            new_w4[i + 24] = plain_keys[round - 1][120 + i];
        }
        std::rotate(new_w4.begin(), new_w4.begin() + 8, new_w4.end());

        // SubWord = SubBytes
        for(int i = 0; i < 32; i+=8)
            plain_subbytes(new_w4, i);

        // Xor with RCON
        for(int i = 0; i < 32; i++)
            new_w4[i] ^= ((rc[round - 1] >> (31 - i)) & 0b1);

        // Build the new round key
        // First column
        for(int i = 0; i < 8; i++) 
        {
            plain_keys[round][i]      = new_w4[i]      ^ plain_keys[round - 1][i];
            plain_keys[round][i + 32] = new_w4[i + 8]  ^ plain_keys[round - 1][i + 32];
            plain_keys[round][i + 64] = new_w4[i + 16] ^ plain_keys[round - 1][i + 64];
            plain_keys[round][i + 96] = new_w4[i + 24] ^ plain_keys[round - 1][i + 96];
        }

        // Other columns
        for(int column = 8; column < 32; column += 8) 
        {
            for(int i = 0; i < 8; i++) 
            {
                plain_keys[round][column + i]      = plain_keys[round - 1][column + i]      ^ plain_keys[round][column - 8 + i];
                plain_keys[round][column + i + 32] = plain_keys[round - 1][column + i + 32] ^ plain_keys[round][column - 8 + i + 32];
                plain_keys[round][column + i + 64] = plain_keys[round - 1][column + i + 64] ^ plain_keys[round][column - 8 + i + 64];
                plain_keys[round][column + i + 96] = plain_keys[round - 1][column + i + 96] ^ plain_keys[round][column - 8 + i + 96];
            }
        }
    }
}

/**
 * AddRoundKey
 * 
 * XOR the given key with the state.
 * 
 * @param result The new AES state
 * @param cipher The actual AES state
 * @param aes_key The round key
 * @param ek The EvalKey
 */
void add_round_key(vector<TLWE<typename iksP::domainP>>& result,
                   vector<TLWE<typename iksP::domainP>>& cipher,
                   vector<TLWE<typename iksP::domainP>>& aes_key, 
                   EvalKey &ek) 
{
    // Add the next round key to the state
   for(size_t i = 0; i < cipher.size(); i++)
        HomXOR(result[i], aes_key[i], cipher[i], ek);
}


//=============================//
//         8 to 32 LUTS        //
//      Combine SBox with      //
//          MixColumns         //
//=============================//

/**
 * SubBytes + MixColumns LUTs
 * 
 * Let c be SBox(i) for all i. Then for all c :
 * 
 * aes_luts[0] contains 2c|c|c|3c
 * aes_luts[1] contains 3c|2c|c|c
 * aes_luts[2] contains c|3c|2c|c
 * aes_luts[3] contains c|c|3c|2c
 *  
 */
static volatile const uint32_t aes_luts[4][256] = {
    {
        0xc66363a5, 0xf87c7c84, 0xee777799, 0xf67b7b8d, 0xfff2f20d, 0xd66b6bbd, 0xde6f6fb1, 0x91c5c554, 
        0x60303050, 0x02010103, 0xce6767a9, 0x562b2b7d, 0xe7fefe19, 0xb5d7d762, 0x4dababe6, 0xec76769a, 
        0x8fcaca45, 0x1f82829d, 0x89c9c940, 0xfa7d7d87, 0xeffafa15, 0xb25959eb, 0x8e4747c9, 0xfbf0f00b, 
        0x41adadec, 0xb3d4d467, 0x5fa2a2fd, 0x45afafea, 0x239c9cbf, 0x53a4a4f7, 0xe4727296, 0x9bc0c05b, 
        0x75b7b7c2, 0xe1fdfd1c, 0x3d9393ae, 0x4c26266a, 0x6c36365a, 0x7e3f3f41, 0xf5f7f702, 0x83cccc4f, 
        0x6834345c, 0x51a5a5f4, 0xd1e5e534, 0xf9f1f108, 0xe2717193, 0xabd8d873, 0x62313153, 0x2a15153f, 
        0x0804040c, 0x95c7c752, 0x46232365, 0x9dc3c35e, 0x30181828, 0x379696a1, 0x0a05050f, 0x2f9a9ab5, 
        0x0e070709, 0x24121236, 0x1b80809b, 0xdfe2e23d, 0xcdebeb26, 0x4e272769, 0x7fb2b2cd, 0xea75759f, 
        0x1209091b, 0x1d83839e, 0x582c2c74, 0x341a1a2e, 0x361b1b2d, 0xdc6e6eb2, 0xb45a5aee, 0x5ba0a0fb, 
        0xa45252f6, 0x763b3b4d, 0xb7d6d661, 0x7db3b3ce, 0x5229297b, 0xdde3e33e, 0x5e2f2f71, 0x13848497, 
        0xa65353f5, 0xb9d1d168, 0x00000000, 0xc1eded2c, 0x40202060, 0xe3fcfc1f, 0x79b1b1c8, 0xb65b5bed, 
        0xd46a6abe, 0x8dcbcb46, 0x67bebed9, 0x7239394b, 0x944a4ade, 0x984c4cd4, 0xb05858e8, 0x85cfcf4a, 
        0xbbd0d06b, 0xc5efef2a, 0x4faaaae5, 0xedfbfb16, 0x864343c5, 0x9a4d4dd7, 0x66333355, 0x11858594, 
        0x8a4545cf, 0xe9f9f910, 0x04020206, 0xfe7f7f81, 0xa05050f0, 0x783c3c44, 0x259f9fba, 0x4ba8a8e3, 
        0xa25151f3, 0x5da3a3fe, 0x804040c0, 0x058f8f8a, 0x3f9292ad, 0x219d9dbc, 0x70383848, 0xf1f5f504, 
        0x63bcbcdf, 0x77b6b6c1, 0xafdada75, 0x42212163, 0x20101030, 0xe5ffff1a, 0xfdf3f30e, 0xbfd2d26d, 
        0x81cdcd4c, 0x180c0c14, 0x26131335, 0xc3ecec2f, 0xbe5f5fe1, 0x359797a2, 0x884444cc, 0x2e171739, 
        0x93c4c457, 0x55a7a7f2, 0xfc7e7e82, 0x7a3d3d47, 0xc86464ac, 0xba5d5de7, 0x3219192b, 0xe6737395, 
        0xc06060a0, 0x19818198, 0x9e4f4fd1, 0xa3dcdc7f, 0x44222266, 0x542a2a7e, 0x3b9090ab, 0x0b888883, 
        0x8c4646ca, 0xc7eeee29, 0x6bb8b8d3, 0x2814143c, 0xa7dede79, 0xbc5e5ee2, 0x160b0b1d, 0xaddbdb76, 
        0xdbe0e03b, 0x64323256, 0x743a3a4e, 0x140a0a1e, 0x924949db, 0x0c06060a, 0x4824246c, 0xb85c5ce4, 
        0x9fc2c25d, 0xbdd3d36e, 0x43acacef, 0xc46262a6, 0x399191a8, 0x319595a4, 0xd3e4e437, 0xf279798b, 
        0xd5e7e732, 0x8bc8c843, 0x6e373759, 0xda6d6db7, 0x018d8d8c, 0xb1d5d564, 0x9c4e4ed2, 0x49a9a9e0, 
        0xd86c6cb4, 0xac5656fa, 0xf3f4f407, 0xcfeaea25, 0xca6565af, 0xf47a7a8e, 0x47aeaee9, 0x10080818, 
        0x6fbabad5, 0xf0787888, 0x4a25256f, 0x5c2e2e72, 0x381c1c24, 0x57a6a6f1, 0x73b4b4c7, 0x97c6c651, 
        0xcbe8e823, 0xa1dddd7c, 0xe874749c, 0x3e1f1f21, 0x964b4bdd, 0x61bdbddc, 0x0d8b8b86, 0x0f8a8a85, 
        0xe0707090, 0x7c3e3e42, 0x71b5b5c4, 0xcc6666aa, 0x904848d8, 0x06030305, 0xf7f6f601, 0x1c0e0e12, 
        0xc26161a3, 0x6a35355f, 0xae5757f9, 0x69b9b9d0, 0x17868691, 0x99c1c158, 0x3a1d1d27, 0x279e9eb9, 
        0xd9e1e138, 0xebf8f813, 0x2b9898b3, 0x22111133, 0xd26969bb, 0xa9d9d970, 0x078e8e89, 0x339494a7, 
        0x2d9b9bb6, 0x3c1e1e22, 0x15878792, 0xc9e9e920, 0x87cece49, 0xaa5555ff, 0x50282878, 0xa5dfdf7a, 
        0x038c8c8f, 0x59a1a1f8, 0x09898980, 0x1a0d0d17, 0x65bfbfda, 0xd7e6e631, 0x844242c6, 0xd06868b8, 
        0x824141c3, 0x299999b0, 0x5a2d2d77, 0x1e0f0f11, 0x7bb0b0cb, 0xa85454fc, 0x6dbbbbd6, 0x2c16163a, 
    },
    {
        0xa5c66363, 0x84f87c7c, 0x99ee7777, 0x8df67b7b, 0x0dfff2f2, 0xbdd66b6b, 0xb1de6f6f, 0x5491c5c5, 
        0x50603030, 0x03020101, 0xa9ce6767, 0x7d562b2b, 0x19e7fefe, 0x62b5d7d7, 0xe64dabab, 0x9aec7676, 
        0x458fcaca, 0x9d1f8282, 0x4089c9c9, 0x87fa7d7d, 0x15effafa, 0xebb25959, 0xc98e4747, 0x0bfbf0f0, 
        0xec41adad, 0x67b3d4d4, 0xfd5fa2a2, 0xea45afaf, 0xbf239c9c, 0xf753a4a4, 0x96e47272, 0x5b9bc0c0, 
        0xc275b7b7, 0x1ce1fdfd, 0xae3d9393, 0x6a4c2626, 0x5a6c3636, 0x417e3f3f, 0x02f5f7f7, 0x4f83cccc, 
        0x5c683434, 0xf451a5a5, 0x34d1e5e5, 0x08f9f1f1, 0x93e27171, 0x73abd8d8, 0x53623131, 0x3f2a1515, 
        0x0c080404, 0x5295c7c7, 0x65462323, 0x5e9dc3c3, 0x28301818, 0xa1379696, 0x0f0a0505, 0xb52f9a9a, 
        0x090e0707, 0x36241212, 0x9b1b8080, 0x3ddfe2e2, 0x26cdebeb, 0x694e2727, 0xcd7fb2b2, 0x9fea7575, 
        0x1b120909, 0x9e1d8383, 0x74582c2c, 0x2e341a1a, 0x2d361b1b, 0xb2dc6e6e, 0xeeb45a5a, 0xfb5ba0a0, 
        0xf6a45252, 0x4d763b3b, 0x61b7d6d6, 0xce7db3b3, 0x7b522929, 0x3edde3e3, 0x715e2f2f, 0x97138484, 
        0xf5a65353, 0x68b9d1d1, 0x00000000, 0x2cc1eded, 0x60402020, 0x1fe3fcfc, 0xc879b1b1, 0xedb65b5b, 
        0xbed46a6a, 0x468dcbcb, 0xd967bebe, 0x4b723939, 0xde944a4a, 0xd4984c4c, 0xe8b05858, 0x4a85cfcf, 
        0x6bbbd0d0, 0x2ac5efef, 0xe54faaaa, 0x16edfbfb, 0xc5864343, 0xd79a4d4d, 0x55663333, 0x94118585, 
        0xcf8a4545, 0x10e9f9f9, 0x06040202, 0x81fe7f7f, 0xf0a05050, 0x44783c3c, 0xba259f9f, 0xe34ba8a8, 
        0xf3a25151, 0xfe5da3a3, 0xc0804040, 0x8a058f8f, 0xad3f9292, 0xbc219d9d, 0x48703838, 0x04f1f5f5, 
        0xdf63bcbc, 0xc177b6b6, 0x75afdada, 0x63422121, 0x30201010, 0x1ae5ffff, 0x0efdf3f3, 0x6dbfd2d2, 
        0x4c81cdcd, 0x14180c0c, 0x35261313, 0x2fc3ecec, 0xe1be5f5f, 0xa2359797, 0xcc884444, 0x392e1717, 
        0x5793c4c4, 0xf255a7a7, 0x82fc7e7e, 0x477a3d3d, 0xacc86464, 0xe7ba5d5d, 0x2b321919, 0x95e67373, 
        0xa0c06060, 0x98198181, 0xd19e4f4f, 0x7fa3dcdc, 0x66442222, 0x7e542a2a, 0xab3b9090, 0x830b8888, 
        0xca8c4646, 0x29c7eeee, 0xd36bb8b8, 0x3c281414, 0x79a7dede, 0xe2bc5e5e, 0x1d160b0b, 0x76addbdb, 
        0x3bdbe0e0, 0x56643232, 0x4e743a3a, 0x1e140a0a, 0xdb924949, 0x0a0c0606, 0x6c482424, 0xe4b85c5c, 
        0x5d9fc2c2, 0x6ebdd3d3, 0xef43acac, 0xa6c46262, 0xa8399191, 0xa4319595, 0x37d3e4e4, 0x8bf27979, 
        0x32d5e7e7, 0x438bc8c8, 0x596e3737, 0xb7da6d6d, 0x8c018d8d, 0x64b1d5d5, 0xd29c4e4e, 0xe049a9a9, 
        0xb4d86c6c, 0xfaac5656, 0x07f3f4f4, 0x25cfeaea, 0xafca6565, 0x8ef47a7a, 0xe947aeae, 0x18100808, 
        0xd56fbaba, 0x88f07878, 0x6f4a2525, 0x725c2e2e, 0x24381c1c, 0xf157a6a6, 0xc773b4b4, 0x5197c6c6, 
        0x23cbe8e8, 0x7ca1dddd, 0x9ce87474, 0x213e1f1f, 0xdd964b4b, 0xdc61bdbd, 0x860d8b8b, 0x850f8a8a, 
        0x90e07070, 0x427c3e3e, 0xc471b5b5, 0xaacc6666, 0xd8904848, 0x05060303, 0x01f7f6f6, 0x121c0e0e, 
        0xa3c26161, 0x5f6a3535, 0xf9ae5757, 0xd069b9b9, 0x91178686, 0x5899c1c1, 0x273a1d1d, 0xb9279e9e, 
        0x38d9e1e1, 0x13ebf8f8, 0xb32b9898, 0x33221111, 0xbbd26969, 0x70a9d9d9, 0x89078e8e, 0xa7339494, 
        0xb62d9b9b, 0x223c1e1e, 0x92158787, 0x20c9e9e9, 0x4987cece, 0xffaa5555, 0x78502828, 0x7aa5dfdf, 
        0x8f038c8c, 0xf859a1a1, 0x80098989, 0x171a0d0d, 0xda65bfbf, 0x31d7e6e6, 0xc6844242, 0xb8d06868, 
        0xc3824141, 0xb0299999, 0x775a2d2d, 0x111e0f0f, 0xcb7bb0b0, 0xfca85454, 0xd66dbbbb, 0x3a2c1616,
    },   
    {
        0x63a5c663, 0x7c84f87c, 0x7799ee77, 0x7b8df67b, 0xf20dfff2, 0x6bbdd66b, 0x6fb1de6f, 0xc55491c5, 
        0x30506030, 0x01030201, 0x67a9ce67, 0x2b7d562b, 0xfe19e7fe, 0xd762b5d7, 0xabe64dab, 0x769aec76, 
        0xca458fca, 0x829d1f82, 0xc94089c9, 0x7d87fa7d, 0xfa15effa, 0x59ebb259, 0x47c98e47, 0xf00bfbf0, 
        0xadec41ad, 0xd467b3d4, 0xa2fd5fa2, 0xafea45af, 0x9cbf239c, 0xa4f753a4, 0x7296e472, 0xc05b9bc0, 
        0xb7c275b7, 0xfd1ce1fd, 0x93ae3d93, 0x266a4c26, 0x365a6c36, 0x3f417e3f, 0xf702f5f7, 0xcc4f83cc, 
        0x345c6834, 0xa5f451a5, 0xe534d1e5, 0xf108f9f1, 0x7193e271, 0xd873abd8, 0x31536231, 0x153f2a15, 
        0x040c0804, 0xc75295c7, 0x23654623, 0xc35e9dc3, 0x18283018, 0x96a13796, 0x050f0a05, 0x9ab52f9a, 
        0x07090e07, 0x12362412, 0x809b1b80, 0xe23ddfe2, 0xeb26cdeb, 0x27694e27, 0xb2cd7fb2, 0x759fea75, 
        0x091b1209, 0x839e1d83, 0x2c74582c, 0x1a2e341a, 0x1b2d361b, 0x6eb2dc6e, 0x5aeeb45a, 0xa0fb5ba0, 
        0x52f6a452, 0x3b4d763b, 0xd661b7d6, 0xb3ce7db3, 0x297b5229, 0xe33edde3, 0x2f715e2f, 0x84971384, 
        0x53f5a653, 0xd168b9d1, 0x00000000, 0xed2cc1ed, 0x20604020, 0xfc1fe3fc, 0xb1c879b1, 0x5bedb65b, 
        0x6abed46a, 0xcb468dcb, 0xbed967be, 0x394b7239, 0x4ade944a, 0x4cd4984c, 0x58e8b058, 0xcf4a85cf, 
        0xd06bbbd0, 0xef2ac5ef, 0xaae54faa, 0xfb16edfb, 0x43c58643, 0x4dd79a4d, 0x33556633, 0x85941185, 
        0x45cf8a45, 0xf910e9f9, 0x02060402, 0x7f81fe7f, 0x50f0a050, 0x3c44783c, 0x9fba259f, 0xa8e34ba8, 
        0x51f3a251, 0xa3fe5da3, 0x40c08040, 0x8f8a058f, 0x92ad3f92, 0x9dbc219d, 0x38487038, 0xf504f1f5, 
        0xbcdf63bc, 0xb6c177b6, 0xda75afda, 0x21634221, 0x10302010, 0xff1ae5ff, 0xf30efdf3, 0xd26dbfd2, 
        0xcd4c81cd, 0x0c14180c, 0x13352613, 0xec2fc3ec, 0x5fe1be5f, 0x97a23597, 0x44cc8844, 0x17392e17, 
        0xc45793c4, 0xa7f255a7, 0x7e82fc7e, 0x3d477a3d, 0x64acc864, 0x5de7ba5d, 0x192b3219, 0x7395e673, 
        0x60a0c060, 0x81981981, 0x4fd19e4f, 0xdc7fa3dc, 0x22664422, 0x2a7e542a, 0x90ab3b90, 0x88830b88, 
        0x46ca8c46, 0xee29c7ee, 0xb8d36bb8, 0x143c2814, 0xde79a7de, 0x5ee2bc5e, 0x0b1d160b, 0xdb76addb, 
        0xe03bdbe0, 0x32566432, 0x3a4e743a, 0x0a1e140a, 0x49db9249, 0x060a0c06, 0x246c4824, 0x5ce4b85c, 
        0xc25d9fc2, 0xd36ebdd3, 0xacef43ac, 0x62a6c462, 0x91a83991, 0x95a43195, 0xe437d3e4, 0x798bf279, 
        0xe732d5e7, 0xc8438bc8, 0x37596e37, 0x6db7da6d, 0x8d8c018d, 0xd564b1d5, 0x4ed29c4e, 0xa9e049a9, 
        0x6cb4d86c, 0x56faac56, 0xf407f3f4, 0xea25cfea, 0x65afca65, 0x7a8ef47a, 0xaee947ae, 0x08181008, 
        0xbad56fba, 0x7888f078, 0x256f4a25, 0x2e725c2e, 0x1c24381c, 0xa6f157a6, 0xb4c773b4, 0xc65197c6, 
        0xe823cbe8, 0xdd7ca1dd, 0x749ce874, 0x1f213e1f, 0x4bdd964b, 0xbddc61bd, 0x8b860d8b, 0x8a850f8a, 
        0x7090e070, 0x3e427c3e, 0xb5c471b5, 0x66aacc66, 0x48d89048, 0x03050603, 0xf601f7f6, 0x0e121c0e, 
        0x61a3c261, 0x355f6a35, 0x57f9ae57, 0xb9d069b9, 0x86911786, 0xc15899c1, 0x1d273a1d, 0x9eb9279e, 
        0xe138d9e1, 0xf813ebf8, 0x98b32b98, 0x11332211, 0x69bbd269, 0xd970a9d9, 0x8e89078e, 0x94a73394, 
        0x9bb62d9b, 0x1e223c1e, 0x87921587, 0xe920c9e9, 0xce4987ce, 0x55ffaa55, 0x28785028, 0xdf7aa5df, 
        0x8c8f038c, 0xa1f859a1, 0x89800989, 0x0d171a0d, 0xbfda65bf, 0xe631d7e6, 0x42c68442, 0x68b8d068, 
        0x41c38241, 0x99b02999, 0x2d775a2d, 0x0f111e0f, 0xb0cb7bb0, 0x54fca854, 0xbbd66dbb, 0x163a2c16,
    },
    {
        0x6363a5c6, 0x7c7c84f8, 0x777799ee, 0x7b7b8df6, 0xf2f20dff, 0x6b6bbdd6, 0x6f6fb1de, 0xc5c55491, 
        0x30305060, 0x01010302, 0x6767a9ce, 0x2b2b7d56, 0xfefe19e7, 0xd7d762b5, 0xababe64d, 0x76769aec, 
        0xcaca458f, 0x82829d1f, 0xc9c94089, 0x7d7d87fa, 0xfafa15ef, 0x5959ebb2, 0x4747c98e, 0xf0f00bfb, 
        0xadadec41, 0xd4d467b3, 0xa2a2fd5f, 0xafafea45, 0x9c9cbf23, 0xa4a4f753, 0x727296e4, 0xc0c05b9b, 
        0xb7b7c275, 0xfdfd1ce1, 0x9393ae3d, 0x26266a4c, 0x36365a6c, 0x3f3f417e, 0xf7f702f5, 0xcccc4f83, 
        0x34345c68, 0xa5a5f451, 0xe5e534d1, 0xf1f108f9, 0x717193e2, 0xd8d873ab, 0x31315362, 0x15153f2a, 
        0x04040c08, 0xc7c75295, 0x23236546, 0xc3c35e9d, 0x18182830, 0x9696a137, 0x05050f0a, 0x9a9ab52f, 
        0x0707090e, 0x12123624, 0x80809b1b, 0xe2e23ddf, 0xebeb26cd, 0x2727694e, 0xb2b2cd7f, 0x75759fea, 
        0x09091b12, 0x83839e1d, 0x2c2c7458, 0x1a1a2e34, 0x1b1b2d36, 0x6e6eb2dc, 0x5a5aeeb4, 0xa0a0fb5b, 
        0x5252f6a4, 0x3b3b4d76, 0xd6d661b7, 0xb3b3ce7d, 0x29297b52, 0xe3e33edd, 0x2f2f715e, 0x84849713, 
        0x5353f5a6, 0xd1d168b9, 0x00000000, 0xeded2cc1, 0x20206040, 0xfcfc1fe3, 0xb1b1c879, 0x5b5bedb6, 
        0x6a6abed4, 0xcbcb468d, 0xbebed967, 0x39394b72, 0x4a4ade94, 0x4c4cd498, 0x5858e8b0, 0xcfcf4a85, 
        0xd0d06bbb, 0xefef2ac5, 0xaaaae54f, 0xfbfb16ed, 0x4343c586, 0x4d4dd79a, 0x33335566, 0x85859411, 
        0x4545cf8a, 0xf9f910e9, 0x02020604, 0x7f7f81fe, 0x5050f0a0, 0x3c3c4478, 0x9f9fba25, 0xa8a8e34b, 
        0x5151f3a2, 0xa3a3fe5d, 0x4040c080, 0x8f8f8a05, 0x9292ad3f, 0x9d9dbc21, 0x38384870, 0xf5f504f1, 
        0xbcbcdf63, 0xb6b6c177, 0xdada75af, 0x21216342, 0x10103020, 0xffff1ae5, 0xf3f30efd, 0xd2d26dbf, 
        0xcdcd4c81, 0x0c0c1418, 0x13133526, 0xecec2fc3, 0x5f5fe1be, 0x9797a235, 0x4444cc88, 0x1717392e, 
        0xc4c45793, 0xa7a7f255, 0x7e7e82fc, 0x3d3d477a, 0x6464acc8, 0x5d5de7ba, 0x19192b32, 0x737395e6, 
        0x6060a0c0, 0x81819819, 0x4f4fd19e, 0xdcdc7fa3, 0x22226644, 0x2a2a7e54, 0x9090ab3b, 0x8888830b, 
        0x4646ca8c, 0xeeee29c7, 0xb8b8d36b, 0x14143c28, 0xdede79a7, 0x5e5ee2bc, 0x0b0b1d16, 0xdbdb76ad, 
        0xe0e03bdb, 0x32325664, 0x3a3a4e74, 0x0a0a1e14, 0x4949db92, 0x06060a0c, 0x24246c48, 0x5c5ce4b8, 
        0xc2c25d9f, 0xd3d36ebd, 0xacacef43, 0x6262a6c4, 0x9191a839, 0x9595a431, 0xe4e437d3, 0x79798bf2, 
        0xe7e732d5, 0xc8c8438b, 0x3737596e, 0x6d6db7da, 0x8d8d8c01, 0xd5d564b1, 0x4e4ed29c, 0xa9a9e049, 
        0x6c6cb4d8, 0x5656faac, 0xf4f407f3, 0xeaea25cf, 0x6565afca, 0x7a7a8ef4, 0xaeaee947, 0x08081810, 
        0xbabad56f, 0x787888f0, 0x25256f4a, 0x2e2e725c, 0x1c1c2438, 0xa6a6f157, 0xb4b4c773, 0xc6c65197, 
        0xe8e823cb, 0xdddd7ca1, 0x74749ce8, 0x1f1f213e, 0x4b4bdd96, 0xbdbddc61, 0x8b8b860d, 0x8a8a850f, 
        0x707090e0, 0x3e3e427c, 0xb5b5c471, 0x6666aacc, 0x4848d890, 0x03030506, 0xf6f601f7, 0x0e0e121c, 
        0x6161a3c2, 0x35355f6a, 0x5757f9ae, 0xb9b9d069, 0x86869117, 0xc1c15899, 0x1d1d273a, 0x9e9eb927, 
        0xe1e138d9, 0xf8f813eb, 0x9898b32b, 0x11113322, 0x6969bbd2, 0xd9d970a9, 0x8e8e8907, 0x9494a733, 
        0x9b9bb62d, 0x1e1e223c, 0x87879215, 0xe9e920c9, 0xcece4987, 0x5555ffaa, 0x28287850, 0xdfdf7aa5, 
        0x8c8c8f03, 0xa1a1f859, 0x89898009, 0x0d0d171a, 0xbfbfda65, 0xe6e631d7, 0x4242c684, 0x6868b8d0, 
        0x4141c382, 0x9999b029, 0x2d2d775a, 0x0f0f111e, 0xb0b0cb7b, 0x5454fca8, 0xbbbbd66d, 0x16163a2c,
    },
};

/**
 * Packing function for aes_luts
 * 
 * Each tables (4 in total) in `lut_8_to_32` will be packed in 8 TRLWE of 32 TLWE vectors of 32 bits.
 * 
 * * We suppose that each TRLWE to rotate is of degree N = 1024, that's why we choose to pack 32 TLWE of 32 bits. If N != 1024, adapt the slicing.
 * 
 * @param lut_8_to_32 An array that will contain all the packs
 * @param sk The SecretKey (We suppose that the LUT is computed by the user)
 */
void init_luts(vector<vector<TRLWE<lvl1param>>>& lut_8_to_32, SecretKey &sk) {
    // Packing Key
    AnnihilateKey<lvl1param> ahk{};
    annihilatekeygen<lvl1param>(ahk, sk);

    // 4 Tables
    lut_8_to_32 = vector<vector<TRLWE<lvl1param>>>(4);
    for(int i = 0; i < 4; i++) {
        // 8 TRLWE
        lut_8_to_32[i] = vector<TRLWE<lvl1param>>(8);
        for (int j = 0; j < 8; j++) {
            // Each TRLWE is packed from 32 values in the table
            vector<uint8_t> t0(1024);

            for(int k = 0; k < 32; k ++)
                for (int l = 0; l < 32; l++) 
                    t0[k*32 + l] = (aes_luts[i][j*32 + k] >> (31 - l)) & 0b1;
            
            vector<TLWE<lvl1param>> T0{1024};

            T0 = bootsSymEncrypt(t0, sk);
            TLWE2TRLWEPacking<lvl1param>(lut_8_to_32[i][j], T0, ahk);
        }
    }
}

/**
 * LUT Eval that evaluates both subBytes and MixColumns simulteanously 
 * 
 * @param result 32 bits of the new AES State
 * @param tgsw 8 TRGSW from the byte we're handling
 * @param lut_8_to_32 A pack from sub array of the global array `aes_luts`
 * @param ek The EvalKey
 */
void lut_8_to_32_eval(vector<TLWE<lvl1param>>& result,
                      vector<TRGSWFFT<typename privksP::targetP>> &tgsw,
                      vector<TRLWE<lvl1param>>& lut_8_to_32,
                      EvalKey &ek) 
{
    
    // Select the TRLWE to rotate from CMUX with C7 C6, C5
    vector<TRLWE<typename privksP::targetP>> tables_c5(4);
    vector<TRLWE<typename privksP::targetP>> tables_c6(2);
    TRLWE<typename privksP::targetP> acc{};
    for(int i = 0; i < 4; i++)
        CMUXFFT<typename privksP::targetP>(tables_c5[i], tgsw[5], lut_8_to_32[(2 * i) + 1], lut_8_to_32[2 * i]);
    for(int i = 0; i < 2; i++)
        CMUXFFT<typename privksP::targetP>(tables_c6[i], tgsw[6], tables_c5[(2 * i) + 1], tables_c5[2 * i]);
    CMUXFFT<typename privksP::targetP>(acc, tgsw[7], tables_c6[1], tables_c6[0]);

    // BlindRotate
    privksP::targetP::T *bara = new privksP::targetP::T[6];
    privksP::targetP::T NX2 = 2 * privksP::targetP::n;
    for (int32_t i = 0; i < 5; i++)
        bara[i] = NX2 - 32 * pow(2, i);
    BlindRotate_LUT<privksP>(acc, bara, tgsw, 5);


    // SampleExtract + KeySwitch + GateBS 0 to 1
    for(int i = 0; i < 32; i++) {
        TLWE<typename iksP::domainP> extracted_lwe{};
        TLWE<typename iksP::targetP> keyswitch_lwe{};
        SampleExtractIndex<typename iksP::domainP>(extracted_lwe, acc, i);
        IdentityKeySwitch<iksP>(keyswitch_lwe, extracted_lwe, *ek.iksklvl10);                                              // Level 1 to Level 0
        GateBootstrappingTLWE2TLWEFFT<brP>(result[i], keyswitch_lwe, *ek.bkfftlvl01, μpolygen<lvl1param, lvl1param::μ>()); // Level 0 to Level 1
    }
}

/**
 * Apply all 8 to 32 bits LUT Eval on all columns
 * 
 * * We apply ShiftRows simultaneously by selecting the right indexes
 * 
 * @param result The new AES state
 * @param tgsw All the state's TGSW after CircuitBS
 * @param lut_8_to_32 The pack of the global array `aes_luts`
 * @param ek The EvalKey
 */
void all_lut_8_to_32_eval(vector<TLWE<typename iksP::domainP>> &result,
                          const vector<TRGSWFFT<typename privksP::targetP>> &tgsw,
                          vector<vector<TRLWE<lvl1param>>>& lut_8_to_32,
                          EvalKey &ek)
{
    // For each column : we have to compute 4 terms using lut_8_to_32_eval
    for(int column = 0; column < 32; column += 8) 
    {
        // All the TGSW needed (8 for each line (4 lines) = 32)
        vector<vector<TRGSWFFT<typename privksP::targetP>>> sub_tgsw(4);
        sub_tgsw[0] = vector<TRGSWFFT<typename privksP::targetP>>{tgsw.begin() + column, tgsw.begin() + column + 8};
        sub_tgsw[1] = vector<TRGSWFFT<typename privksP::targetP>>{tgsw.begin() + 32 + ((column + 8) % 32), tgsw.begin() + 40 + ((column + 8) % 32)};
        sub_tgsw[2] = vector<TRGSWFFT<typename privksP::targetP>>{tgsw.begin() + 64 + ((column + 16) % 32), tgsw.begin() + 72 + ((column + 16) % 32)};
        sub_tgsw[3] = vector<TRGSWFFT<typename privksP::targetP>>{tgsw.begin() + 96 + ((column + 24) % 32), tgsw.begin() + 104 + ((column + 24) % 32)};

        //Reverse each array to have the right order i -> C_i
        for(vector<TRGSWFFT<typename privksP::targetP>> &elem : sub_tgsw) std::reverse(elem.begin(), elem.end()); 

        // LUT Eval
        vector<vector<TLWE<typename iksP::domainP>>> aux(4);
        for(int j = 0; j < 4; j++) 
        {
            aux[j] = vector<TLWE<typename iksP::domainP>>(32);
            lut_8_to_32_eval(aux[j], sub_tgsw[j], lut_8_to_32[j], ek);
        }

        // XOR all the elements
        vector<TLWE<typename iksP::domainP>> result_xor(32);
        for(int j = 0; j < 32; j++) 
        {
            HomCOPY(result_xor[j],aux[0][j]);
            HomXOR(result_xor[j], result_xor[j], aux[1][j], ek);
            HomXOR(result_xor[j], result_xor[j], aux[2][j], ek);
            HomXOR(result_xor[j], result_xor[j], aux[3][j], ek);
        }

        // Copy the result in the right column
        for(int j = 0; j < 8; j++) {
            result[column + j] = result_xor[j];
            result[column + 32 + j] = result_xor[j + 8];
            result[column + 64 + j] = result_xor[j + 16];
            result[column + 96 + j] = result_xor[j + 24];
        }
    } 
}

void plain_lut_8_to_32_eval(vector<uint8_t> &block)
{

    // This will store all the new columns
    vector<uint32_t> res_xor(4);

    // For each column
    for(int i = 0; i < 32; i += 8) {
        // Compute bytes
        uint8_t i0 = bits_to_byte(block, i);
        uint8_t i1 = bits_to_byte(block, 32 + ((i + 8)  % 32));
        uint8_t i2 = bits_to_byte(block, 64 + ((i + 16) % 32));
        uint8_t i3 = bits_to_byte(block, 96 + ((i + 24) % 32));

        // Apply 8-to-32 bit LUT
        uint32_t s0 = aes_luts[0][i0];
        uint32_t s1 = aes_luts[1][i1];
        uint32_t s2 = aes_luts[2][i2];
        uint32_t s3 = aes_luts[3][i3];

        // XOR each 32 bits LUT output
        res_xor[i / 8] = s0 ^ s1 ^ s2 ^ s3;
    }

    // Replace the current state with the new state
    uint64_t msb = res_xor[0]; 
    uint64_t lsb = res_xor[2]; 
    msb = (msb << 32) + res_xor[1];
    lsb = (lsb << 32) + res_xor[3];
    vector_from_int(block, msb, lsb);
}

//=============================//
//            MAIN             //
//    with test subfunctions   //
//=============================//

// ! VERIFIER TOUTES LES INITS

int main(int argc, char* argv[])
{
    if (argc == 2 && strcmp(argv[1], "--test") == 0) 
        test = true;

    constexpr uint32_t block_size = 128;

    // Secret Key
    SecretKey* sk = new SecretKey();

    // Eval Key
    EvalKey ek;
    ek.emplacebkfft<brP>(*sk);          // For Gate BS
    ek.emplacebkfft<bkP>(*sk);          // For CB
    ek.emplaceiksk<iksP>(*sk);          // For KS
    ek.emplaceprivksk4cb<privksP>(*sk); // For CB

    // AES KEY : In our tests, we use the same key
    uint64_t plain_key_msb = 0x2b7e151628aed2a6;
    uint64_t plain_key_lsb = 0xabf7158809cf4f3c;

    // MESSAGE
    /*
        plain  = 0x3243f6a8885a308d 313198a2e0370734
        cipher = 0x3925841d02dc09fb dc118597196a0b32
    */
    // uint64_t plain_block_msb = 0x3243f6a8885a308d;
    // uint64_t plain_block_lsb = 0x313198a2e0370734;

    /*
        plain  = 0x6bc1bee22e409f96 e93d7e117393172a
        cipher = 3ad77bb40d7a3660a8 9ecaf32466ef97
    */
    // uint64_t plain_block_msb = 0x6bc1bee22e409f96;
    // uint64_t plain_block_lsb = 0xe93d7e117393172a;

    /*
        plain  = 0xae2d8a571e03ac9c 9eb76fac45af8e51
        cipher = 0xf5d3d58503b9699d e785895a96fdbaaf
    */
    uint64_t plain_block_msb = 0xae2d8a571e03ac9c;
    uint64_t plain_block_lsb = 0x9eb76fac45af8e51;


    // Initialize an AES Key 
    if (test) cout << "======== Round Keys ==========" << endl;
    vector<vector<uint8_t>> plain_keys(11, vector<uint8_t>(128));
    
    init_keys(plain_keys, plain_key_msb, plain_key_lsb);

    if(test)
        for(int i = 0; i < 11; i++) print_plain(plain_keys[i], "rk[" + to_string(i) + "] = ");
    
    // Generate all the round keys
    vector<vector<TLWE<typename iksP::domainP>>> aes_keys(11);
    for(int i = 0; i < 11; i++) {
        aes_keys[i] = vector<TLWE<typename iksP::domainP>>(128);
        aes_keys[i] = bootsSymEncrypt(plain_keys[i], *sk);
    }

    TRLWE<lvl1param> d0;
    TRLWE<lvl1param> d1;
    pack_sbox(d0, d1, *sk); // d0 and d1 for SubBytes at the end

    vector<vector<TRLWE<lvl1param>>> lut_8_to_32;
    init_luts(lut_8_to_32, *sk); // All 8_to_32 LUTs

    // Initialize an AES block and encrypt it
    vector<uint8_t> aes_block(block_size);
    vector_from_int(aes_block, plain_block_msb, plain_block_lsb);

    vector<TLWE<typename iksP::domainP>> c_aes_block(block_size);
    c_aes_block = bootsSymEncrypt<typename iksP::domainP>(aes_block, *sk);

    if(test) {
        cout << endl;
        cout << "======== Encrypt Block ==========" << endl;
        print_plain(aes_block, "input = ");
        print_dec(c_aes_block, *sk, "dec   = "); // Test Decrypt
        cout << endl;
    }

    // First AddRoundKey 
    auto start_aes = chrono::system_clock::now();
    auto start_round0 = chrono::system_clock::now();
    if (test) cout << "======== AddRoundKey ==========" << endl;
    vector<TLWE<typename iksP::domainP>> c_aes_ark(block_size);
    add_round_key(c_aes_ark, c_aes_block, aes_keys[0], ek);
    if (test) {
        print_plain(plain_keys[0], "r_key = ");
        for(int i = 0; i < 128; i++) 
            aes_block[i] ^= plain_keys[0][i];
        cout << endl;
        print_plain(aes_block, "state = ");
        print_dec(c_aes_ark, *sk, "dec   = ");
        cout << endl;
    }
    auto stop_round0 = chrono::system_clock::now();
    auto time_round0 = chrono::duration_cast<std::chrono::milliseconds>(stop_round0 - start_round0).count();
    cout << std::dec << "t_0 = " << time_round0 << "ms" << endl << endl;

    // 9 Rounds of SubByte, ShiftRows, MixColumns, AddRoundKey
    for(int r = 1; r < 10; r++) {
        auto start_roundi = chrono::system_clock::now();
        if (test) cout << "========== Round " << r << " ==========" << endl;
        // Computes CB for all Bytes
        vector<TRGSWFFT<lvl1param>> bootedTGSW(block_size);
        for (int i = 0; i < block_size; i++) {
            CircuitBootstrappingFFT<iksP, bkP, privksP>(bootedTGSW[i], c_aes_ark[i], ek);
        }
        if (test) cout << "*  CB Computed." << endl << endl;

        // Apply lut_8_to_32_eval to every columns to output a new AES state
        vector<TLWE<typename iksP::domainP>> c_aes_sbmc(block_size);
        all_lut_8_to_32_eval(c_aes_sbmc, bootedTGSW, lut_8_to_32, ek);
        if (test) {
            cout << "* 8 to 32 bits LUT :" << endl;
            plain_lut_8_to_32_eval(aes_block);
            print_plain(aes_block, "state = ");
            print_dec(c_aes_sbmc, *sk, "dec   = ");
            cout << endl;
        }

        // AddRoundKey
        add_round_key(c_aes_ark, c_aes_sbmc, aes_keys[r], ek); 
        if (test) {
            cout << "* AddRoundKey :" << endl;
            print_plain(plain_keys[r], "r_key = ");
            for(int i = 0; i < 128; i++)
                aes_block[i] ^= plain_keys[r][i];
            cout << endl;
            print_plain(aes_block, "state = ");
            print_dec(c_aes_ark, *sk, "dec   = ");
            cout << endl;

        }
        auto stop_roundi = chrono::system_clock::now();
        auto time_roundi = chrono::duration_cast<std::chrono::milliseconds>(stop_roundi - start_roundi).count();
        cout << std::dec << "t" << r << " = " << time_roundi << "ms" << endl << endl;
    }

    auto start_round10 = chrono::system_clock::now();
    if(test) cout << "========== Round 10 ==========" << endl;
    // Computes CB for all Bytes
    vector<TRGSWFFT<typename privksP::targetP>> bootedTGSW(block_size);
    for (int i = 0; i < block_size; i++)
        CircuitBootstrappingFFT<iksP, bkP, privksP>(bootedTGSW[i],c_aes_ark[i], ek);
    if (test) cout << "* CB Computed." << endl<< endl;

    // SubBytes that will be stored in c_aes_sb
    sub_bytes(c_aes_block, bootedTGSW, c_aes_ark, d0, d1, ek);
    
    if (test) {
        cout << "* SubBytes :" << endl;
        for(int i = 0; i < 128; i += 8)
            plain_subbytes(aes_block, i);
        print_plain(aes_block, "state = ");
        print_dec(c_aes_block, *sk, "dec   = ");
        cout << endl;
    }

    // Shift Rows
    shift_rows(c_aes_block);
    if (test) {
        cout << "* ShiftRows :" << endl;
        plain_shift_rows(aes_block);
        print_plain(aes_block, "state = ");
        print_dec(c_aes_ark, *sk, "dec   = ");
        cout << endl;
    }

    // Add Round Key
    add_round_key(c_aes_ark, c_aes_block, aes_keys[10], ek); 
    if (test) {
        cout << "* AddRoundKey :" << endl;
        print_plain(plain_keys[10], "r_key = ");
        for(int i = 0; i < 128; i++)
            aes_block[i] ^= plain_keys[10][i];
        cout << endl;
        print_plain(aes_block, "state = ");
        print_dec(c_aes_ark, *sk, "dec   = ");
        cout << endl;
    }
    auto stop_round10 = chrono::system_clock::now();
    auto time_round10 = chrono::duration_cast<std::chrono::milliseconds>(stop_round10 - start_round10).count();
    cout << std::dec << "t_10 = " << time_round10 << "ms" << endl << endl;

    // Decrypt and check the result
    if (test) {
        cout << "========== Results ==========" << endl;
        print_plain(aes_block, "state = ");
    }
    print_dec(c_aes_ark, *sk, "dec   = ");
    cout << endl;
    
    auto stop_aes = chrono::system_clock::now();
    auto time_aes = chrono::duration_cast<std::chrono::milliseconds>(stop_aes - start_aes).count();
    cout << std::dec << "t_10 = " << time_aes << "ms" << endl << endl;

    delete (sk);
}