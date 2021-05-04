#include "sha256.h"

static const u32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

void sha256_update_basic(u512_t data, u32_t hash[8]) {
#pragma HLS INLINE
    /* Temporary Variables*/
    u32_t a, b, c, d, e, f, g, h;

    /* W-series Temporary Variables */
    u32_t w[64];
    u32_t wsig0[64];
    u32_t wsig1[64];
#if BASIC_OPT
#pragma HLS ARRAY_PARTITION variable = w complete dim = 1
#pragma HLS ARRAY_PARTITION variable = wsig0 complete dim = 1
#pragma HLS ARRAY_PARTITION variable = wsig1 complete dim = 1
#endif

    /* Copy Current Hash Value to Temporary Variables */
    a = hash[0], b = hash[1], c = hash[2], d = hash[3];
    e = hash[4], f = hash[5], g = hash[6], h = hash[7];

    /* Assign First 16 W */
ASSIGN_M_0_16:
    for (u32_t i = 0; i < 16; i++) {
#if BASIC_OPT
#pragma HLS UNROLL
#endif
        u32_t tmp_wi = data(511 - i * 32, 480 - i * 32);
        w[i]         = tmp_wi;
        wsig0[i]     = SIG0(tmp_wi);
        wsig1[i]     = SIG1(tmp_wi);
    }

    /* Iterate 64 times */
ITERATE_64:
    for (u32_t i = 0; i < 64; i++) {
#if BASIC_OPT
#pragma HLS PIPELINE
#endif
        /* Temporary Wi */
        u32_t tmp_wi = w[i];

        /* Temporal Summation */
        u32_t t1 = h + EP1(e) + CH(e, f, g) + k[i] + tmp_wi;
        u32_t t2 = EP0(a) + MAJ(a, b, c);

        /* Swap Values*/
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;

        /* Forward Calculation(For Reducing the Critical Path) */
        if (i < 64 - 16) {
            u32_t tmp_wi_16 = wsig1[i + 14] + w[i + 9] + wsig0[i + 1] + tmp_wi;
            w[i + 16]       = tmp_wi_16;
            wsig0[i + 16]   = SIG0(tmp_wi_16);
            wsig1[i + 16]   = SIG1(tmp_wi_16);
        }
    }

    /* Update Hash Value with Temporary Variables */
    hash[0] += a, hash[1] += b, hash[2] += c, hash[3] += d;
    hash[4] += e, hash[5] += f, hash[6] += g, hash[7] += h;
}
