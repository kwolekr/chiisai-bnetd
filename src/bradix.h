/*-
 * Copyright (c) 2010 Ryan Kwolek 
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

#ifndef BRADIX_HEADER
#define BRADIX_HEADER

////////// Compile-time configuration ///////////
#define RADIX_INITPTRS  2
#define LEN_IPADDR		4
/////////////////////////////////////////////////

//#define RADIX_DONE		0x80


#define BITFIELD_ARRLEN (UCHAR_MAX / (sizeof(unsigned int) * CHAR_BIT))


typedef struct _trienode {
	unsigned char key[LEN_IPADDR];
	//int keylen;
	void *data;
	
	unsigned short keylen;
	unsigned char numsubnodes;
	unsigned char numptrs;

	unsigned int bitfield[BITFIELD_ARRLEN];
	struct _trienode *subnodes[0];
} TNODE, *LPTNODE;

LPTNODE RadixInit();
LPTNODE RadixInsert(LPTNODE rnode, const char *str, void *data, unsigned char flags);
int RadixRemove(LPTNODE rnode, const char *str);
void *RadixSearch(LPTNODE rnode, const char *str);
void **RadixSearchAll(LPTNODE rnode, const char *str, int *nresults);
void *RadixFindMatch(LPTNODE rnode, const char *str);

//LPTNODE _RadixCreateNode(const char *text, unsigned short flags, void *data, int initialptrs);
LPTNODE _RadixCreateNode(const unsigned char *key, unsigned short keylen, void *data, int initialptrs);
LPTNODE _RadixResizeSubnodes(LPTNODE lpnode, int numsnodes);
void _RadixMergeNodes(LPTNODE lpparent, LPTNODE mergeto, LPTNODE mergefrom);
void _RadixScanTreeSize(LPTNODE lpnode, int *num);
void _RadixScanTree(LPTNODE lpnode, void **results, int *num);

#endif //BRADIX_HEADER
