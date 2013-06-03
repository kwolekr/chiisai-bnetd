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

/* 
 * bradix.c - 
 *    Contains routines and support routines for a tree-based binary data 
 *    storage structure (space optimised trie)
 */


#include "main.h"
#include "fxns.h"
#include "bradix.h"


///////////////////////////////////////////////////////////////////////////
//not really usable right now
#if 0
LPTNODE RadixInit() {
	return _RadixCreateNode("", 0, NULL, RADIX_INITPTRS);
}


LPTNODE RadixInsert(LPTNODE rnode, const unsigned char *key, int keylen, void *data, unsigned char flags) {
	int i, j, bfindex;
	unsigned int val, mask;
	char mchr, *tmp, ot;
	const char *tsr;
	LPTNODE tmpnode, lpparent, lpnode, rnnode, ronode;

	if (!rnode || !str)
		return rnode;
	
	lpnode = rnode;
	lpparent = NULL;

	for (i = 0; i != keylen; i++) {
		/*mchr = chrmap[*str];
		if (mchr == -1)
			return rnode;*/
		/*for (j = 0; j != ARRAYLEN(lpnode->bitfield); j++) {
			val = lpnode->bitfield[j];
			if (val)
				bfindex = bitscanfwd(val);
			break;
		}*/
		val  = lpnode->bitfield[(key[i] / BP_BITBLOCK)];
		mask = 1 << (key[i] & 31);
		if (val & mask) {
			tmpnode = _RadixCreateNode(key + i, hmm, data, RADIX_INITPTRS);

			lpnode->index[mchr] = (unsigned char)lpnode->numsubnodes;
			lpnode->subnodes[lpnode->numsubnodes] = tmpnode;
			lpnode->numsubnodes++;
			
			if (lpnode->numsubnodes >= lpnode->numptrs) {
				lpnode->numptrs <<= 1; 
				tmpnode = realloc(lpnode, sizeof(TNODE) + lpnode->numptrs * sizeof(LPTNODE));
				if (lpnode == rnode)
					rnode = tmpnode;

				lpnode = tmpnode;
				if (lpparent)
					lpparent->subnodes[lpparent->index[chrmap[*lpnode->text]]] = lpnode;
			}
			return rnode;
		} else {
			lpparent = lpnode;
			lpnode = lpnode->subnodes[lpnode->index[mchr]];
			
			tmp = lpnode->text;
			tsr = str;
			while (*tmp) {
				if (*tsr != *tmp) {                
					ronode = _RadixCreateNode(tmp, lpnode->flags, lpnode->data, lpnode->numsubnodes);
					ot = *tmp;
					*tmp = 0;

					if (lpnode->numsubnodes) {
						for (j = 0; j != lpnode->numsubnodes; i++)
							ronode->subnodes[j] = lpnode->subnodes[j];
						ronode->numsubnodes = lpnode->numsubnodes;
					}		

					memcpy(ronode->index, lpnode->index, sizeof(lpnode->index));
					memset(lpnode->index, -1, sizeof(lpnode->index));
					lpnode = _RadixResizeSubnodes(lpnode, RADIX_INITPTRS);
					
					lpnode->data = data;
					lpnode->subnodes[0] = ronode;
					lpnode->index[chrmap[ot]] = 0;
					lpnode->numsubnodes++;

					if (lpparent)
						lpparent->subnodes[lpparent->index[chrmap[*lpnode->text]]] = lpnode;

					if (*tsr) {
						rnnode = _RadixCreateNode(tsr, RADIX_DONE, data, RADIX_INITPTRS);
						
						lpnode->flags &= ~RADIX_DONE;
						lpnode->data   = NULL;
						lpnode->subnodes[1] = rnnode;
						lpnode->index[chrmap[*tsr]] = 1;
						lpnode->numsubnodes++;
					}
					return rnode;
				}
				*tsr++;
				*tmp++;
			}
		}
		str = tsr;
	}

	return rnode;
}


int RadixRemove(LPTNODE rnode, const char *str) {
	char mchr, *tmp;
	unsigned char index;
	LPTNODE tmpnode, lpparent, lpnode;
	
	if (!rnode || !str)
		return 0;

	lpnode = NULL;
	tmpnode = rnode;

	while (*str) {
		lpparent = lpnode;
		lpnode = tmpnode;

		mchr = chrmap[*str];
		if (mchr == -1)
			return 0;
			
		index = lpnode->index[mchr];
		if (index == 0xFF)
			return 0;

		tmpnode = lpnode->subnodes[index];

		tmp = tmpnode->text;
		while (*tmp) {
			if (*str != *tmp)
				return 0;
			str++;
			tmp++;
		}
	}

	if (!(tmpnode->flags & RADIX_DONE))
		return 0;

	if (!tmpnode->numsubnodes) {
		lpnode->numsubnodes--;

		lpnode->subnodes[index] = lpnode->subnodes[lpnode->numsubnodes];
		lpnode->index[chrmap[lpnode->subnodes[lpnode->numsubnodes]->text[0]]] = index;
		lpnode->index[mchr] = 0xFF;

		if (lpnode->numsubnodes == 1)
			_RadixMergeNodes(lpparent, lpnode->subnodes[0], lpnode);
	
		free(tmpnode);
	} else {
		tmpnode->flags &= ~RADIX_DONE;

		if (lpnode->numsubnodes == 1)
			_RadixMergeNodes(lpparent, lpnode->subnodes[0], lpnode);
	}
	free(tmpnode->data);
	return 1;
}


//// get the associated key for a specific item in the trie
void *RadixSearch(LPTNODE rnode, const char *str) {
	LPTNODE lpnode, tmpnode;
	char mchr, *tmp;
	unsigned char nmchar;

	if (!rnode || !str)
		return NULL;

	lpnode = rnode;
	while (*str) {
		mchr = chrmap[*str];
		if (mchr == -1)
			return NULL;

		nmchar = lpnode->index[mchr]; 
		if (nmchar == 0xFF)
			return NULL;

		tmpnode = lpnode->subnodes[nmchar];
		if (!tmpnode)
			return NULL;

		tmp = tmpnode->text;
		while (*tmp) {
			if (*str != *tmp)
				return NULL;
			str++;
			tmp++;
		}
		lpnode = tmpnode;
	}

	return (lpnode->flags & RADIX_DONE) ? lpnode->data : NULL;
}


//// return all items in the tree
void **RadixSearchAll(LPTNODE rnode, const char *str, int *nresults) {
	LPTNODE lpnode, tmpnode;
	char mchr, *tmp;
	unsigned char index;
	void **results;
	int num;

	if (!rnode || !str || !nresults)
		return NULL;

	*nresults = 0;
	
	lpnode = rnode;
	while (*str) {
		mchr = chrmap[*str];
		if (mchr == -1)
			return NULL;

		index = lpnode->index[mchr];
		if (index == 0xFF)
			return NULL;

		tmpnode = lpnode->subnodes[index];
		tmp = tmpnode->text;
		while (*str && *tmp) {
			if (*str != *tmp)
				return NULL;
			str++;
			tmp++;
		}
		lpnode = tmpnode;
	}
	
	_RadixScanTreeSize(lpnode, nresults);
	if (!*nresults)
		return NULL;

	results = malloc(*nresults * sizeof(void *));
	num		= 0;
	_RadixScanTree(lpnode, results, &num);

	return results;
}


//// find the node matching a needle
void *RadixFindMatch(LPTNODE rnode, const char *str) {
	LPTNODE lpnode, tmpnode;
	char mchr, *tmp;
	unsigned char nmchar;

	if (!rnode || !str)
		return NULL;

	lpnode = rnode;
	while (lpnode->numsubnodes && !(lpnode->flags & RADIX_DONE)) {
		mchr = chrmap[*str];
		if (mchr == -1)
			return NULL;

		nmchar = lpnode->index[mchr]; 
		if (nmchar == 0xFF)
			return NULL;

		tmpnode = lpnode->subnodes[nmchar];
		if (!tmpnode)
			return NULL;

		tmp = tmpnode->text;
		while (*tmp) {
			if (*str != *tmp)
				return NULL;
			str++;
			tmp++;
		}
		lpnode = tmpnode;
	}

	return (lpnode->flags & RADIX_DONE) ? lpnode->data : NULL;
}


LPTNODE _RadixCreateNode(const unsigned char *key, unsigned short keylen, void *data, int initialptrs) {
	LPTNODE rnode	   = malloc(sizeof(TNODE) + initialptrs * sizeof(LPTNODE));

	rnode->data		   = data;
	rnode->keylen	   = keylen;
	rnode->numsubnodes = 0;
	rnode->numptrs	   = initialptrs;

	memcpy(rnode->key, key, keylen);
	memset(rnode->bitfield, 0, BITFIELD_ARRLEN * sizeof(rnode->bitfield[0]));

	return rnode;
}


LPTNODE _RadixResizeSubnodes(LPTNODE lpnode, int numsnodes) {
	lpnode				= realloc(lpnode, sizeof(TNODE) + numsnodes * sizeof(LPTNODE));
	lpnode->numptrs		= numsnodes;
	lpnode->numsubnodes = 0;
	return lpnode;
}


void _RadixMergeNodes(LPTNODE lpparent, LPTNODE mergeto, LPTNODE mergefrom) {
	char temp[32], mchr;
	unsigned char index;

	mchr = chrmap[mergefrom->text[0]];
	if (mchr == -1)
		return;

	index = lpparent->index[mchr];
	if (index == 0xFF)
		return;

	lpparent->subnodes[index] = mergeto;

	strncpy(temp, mergeto->text, sizeof(temp));
	temp[sizeof(temp) - 1] = 0;
	strncpy(mergeto->text, mergefrom->text, sizeof(mergeto->text));
	mergeto->text[sizeof(mergeto->text) - 1] = 0;
	strncat(mergeto->text, temp, sizeof(mergeto->text));
	mergeto->text[sizeof(mergeto->text) - 1] = 0;
}


void _RadixScanTreeSize(LPTNODE lpnode, int *num) {
	int i;

	if (lpnode->flags & RADIX_DONE)
		(*num)++;
	for (i = 0; i != lpnode->numsubnodes; i++)
		_RadixScanTreeSize(lpnode->subnodes[i], num);
}


void _RadixScanTree(LPTNODE lpnode, void **results, int *num) {
	int i;

	if (lpnode->flags & RADIX_DONE) {
		results[*num] = lpnode->data;
		(*num)++;
	}
	for (i = 0; i != lpnode->numsubnodes; i++)
		_RadixScanTree(lpnode->subnodes[i], results, num);
}
#endif
