/*-
 * Copyright (c) 2011 Ryan Kwolek 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this list of
 *     conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list
 *     of conditions and the following disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "main.h"
#include "fxns.h"
#include "hashing.h"
#include "srp.h"

const char modulus[] = {
	0x87, 0xc7, 0x23, 0x85, 0x65, 0xf6, 0x16, 0x12,
	0xd9, 0x12, 0x32, 0xc7, 0x78, 0x6c, 0x97, 0x7e,
	0x55, 0xb5, 0x92, 0xa0, 0x8c, 0xb6, 0x86, 0x21,
	0x03, 0x18, 0x99, 0x61, 0x8b, 0x1a, 0xff, 0xf8
};

const char NLS_I[] = {
	0x6c, 0x0e, 0x97, 0xed, 0x0a, 0xf9, 0x6b, 0xab, 0xb1, 0x58,
	0x89, 0xeb, 0x8b, 0xba, 0x25, 0xa4, 0xf0, 0x8c, 0x01, 0xf8
};

mpz_t N;
gmp_randstate_t gmprand;


///////////////////////////////////////////////////////////////////////////////


void SRPGlobalInit() {
	mpz_init2(N, 256);
	mpz_import(N, 32, -1, sizeof(modulus[0]), 0, 0, modulus);
	
	gmp_randinit_default(gmprand);
	gmp_randseed_ui(gmprand, RandomGenSecure());
}


void SRPCalcB(LPNLS nls, const char *V) {
	mpz_t g;

	mpz_init(nls->b);
	mpz_init2(nls->v, 256);
	mpz_init2(nls->b_src, 256);

	mpz_init_set_ui(g, 0x2F);
	mpz_import(nls->v, 32, -1, 1, 0, 0, V);
	memcpy(nls->V, V, sizeof(nls->V));

	mpz_urandomm(nls->b_src, gmprand, N);
	mpz_powm(nls->b, g, nls->b_src, N);
	mpz_add(nls->b, nls->b, nls->v);
	mpz_mod(nls->b, nls->b, N);
	
	mpz_export(nls->B, (size_t *)0, -1, 1, 0, 0, nls->b);

	mpz_clear(g);
}


void SRPFree(LPNLS nls) {
	mpz_clear(nls->b);
	mpz_clear(nls->b_src);
	mpz_clear(nls->v);
	free(nls);
}


uint32_t SRPCalcU(const unsigned char *B) {
	unsigned char hash[20];
    
	SHA1(B, 32, hash);
	return SWAP4(*(uint32_t *)hash);
}


void SRPCalcS(LPNLS nls, char *result) {
	mpz_t temp, s, v, a;

	mpz_init2(a, 256);
	mpz_init2(v, 256);
	mpz_init(temp);
	mpz_init(s);

	mpz_import(a, 32, -1, 1, 0, 0, nls->A);
	mpz_import(v, 32, -1, 1, 0, 0, nls->V);

	mpz_powm_ui(temp, v, SRPCalcU(nls->B), N);
	mpz_mul(temp, a, temp);
	mpz_powm(s, temp, nls->b_src, N);

	mpz_export(result, (size_t *)0, -1, 1, 0, 0, s);

	mpz_clear(temp);
	mpz_clear(s);
	mpz_clear(v);
	mpz_clear(a);
}


void SRPCalcK(LPNLS nls, char *S) {
	char odds[16], evens[16], oddhash[20], evenhash[20];
	char *saltptr, *oddptr, *evenptr;
	int i;

	saltptr = S;
	oddptr  = odds;
	evenptr = evens;
	for (i = 0; i != 16; i++) {
		*(oddptr++)  = *(saltptr++);
		*(evenptr++) = *(saltptr++);
	}

	SHA1(odds, 16, oddhash);
	SHA1(evens, 16, evenhash);

	saltptr = nls->K;
	oddptr  = oddhash;
	evenptr = evenhash;
	for (i = 0; i != 20; i++) {
		*(saltptr++) = *(oddptr++);
		*(saltptr++) = *(evenptr++);
	}
}	 


void SRPCalcM1(LPNLS nls) {
	unsigned char buf[176];
	unsigned char S[32];

	SRPCalcS(nls, S);
	SRPCalcK(nls, S);

	memcpy(buf, NLS_I, 20);
	memcpy(buf + 20, nls->namehash, 20);
	memcpy(buf + 40, nls->salt, 32);
	memcpy(buf + 72, nls->A, 32);
	memcpy(buf + 104, nls->B, 32);
	memcpy(buf + 136, nls->K, 40);
	SHA1(buf, sizeof(buf), nls->M1);
}


void SRPCalcM2(LPNLS nls, unsigned char *result) {
	unsigned char buf[92];
	
	memcpy(buf, nls->A, 32);
	memcpy(buf + 32, nls->M1, 20);
	memcpy(buf + 52, nls->K, 40);
	SHA1(buf, sizeof(buf), result);
}
