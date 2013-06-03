/*-
 * Copyright (c) 2008 Ryan Kwolek 
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
#include "hashing.h"

///////////////////////////////////////////////////////////////////////////////


void BSHA1(const void *src, unsigned int len, uint32_t *result) {
    uint32_t a = 0x67452301lu, b = 0xefcdab89lu;
    uint32_t c = 0x98badcfelu, d = 0x10325476lu;
    uint32_t e = 0xc3d2e1f0lu, g, *lpdwBuffer; 
    unsigned char bBuffer[320] = {0};
	int i;

	memcpy(bBuffer, src, len);
    lpdwBuffer = (uint32_t *)bBuffer;
    for (i = 0; i < 80; ++i) {
		if (i < 64)
			lpdwBuffer [i + 16] = ROL(1, (lpdwBuffer[i] ^ lpdwBuffer[i + 8] ^ lpdwBuffer[i + 2] ^ lpdwBuffer[i + 13]) % 32);
		if (i < 20)
			g = lpdwBuffer[i] + ROL(a, 5) + e + ((b & c) | (~b & d)) + 0x5a827999lu;
		else if (i < 40)
			g = (d ^ c ^ b) + e + ROL(g, 5) + lpdwBuffer[i] + 0x6ed9eba1lu;
		else if (i < 60)
			g = lpdwBuffer[i] + ROL(g, 5) + e + ((c & b) | (d & c) | (d & b)) - 0x70e44324lu;
		else
			g = (d ^ c ^ b) + e + ROL(g, 5) + lpdwBuffer[i] - 0x359d3e2alu;
		e = d;
		d = c;
		c = ROL(b, 30);
		b = a;
		a = g;
    }
    result[0] = 0x67452301lu + g;
    result[1] = 0xefcdab89lu + b;
    result[2] = 0x98badcfelu + c;
    result[3] = 0x10325476lu + d;
    result[4] = 0xc3d2e1f0lu + e;
}


void SHA1(const char *source, unsigned int len, char *output) {
	#ifdef _WIN32
		unsigned long hashlen = 20;
		CryptHashCertificate((unsigned long)NULL, 0, 0, (const unsigned char *)source,
			(unsigned int)len, (unsigned char *)output, &hashlen);
	#else
		char temp[64];
		SHA1_Data((unsigned char *)source, (unsigned int)len, temp);
		HexToStr(temp, output);
	#endif
}


void MD5(const char *source, unsigned int len, char *output) {
	#ifdef _WIN32
		unsigned long hashlen = 16;
		CryptHashCertificate((unsigned long)NULL, CALG_MD5, 0, (const unsigned char *)source,
			(unsigned long)len, (unsigned char *)output, &hashlen);
	#else
		char temp[64];
		MD5Data((unsigned char *)source, (unsigned int)len, (unsigned char *)temp);
		HexToStr(temp, output);
	#endif
}
