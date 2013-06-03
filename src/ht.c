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

/* 
 * ht.c - 
 *    High performance hashtable routines and general purpose 32 bit hash function
 */


#include "main.h"
#include "fxns.h"
#include "vector.h"
#include "ht.h"


///////////////////////////////////////////////////////////////////////////////


void HtInsertItem(const char *key, void *newentry, LPVECTOR *table, unsigned int tablelen) {
	unsigned int index;
	#ifdef HT_CASE_INSENSITIVE
		char *__key;
		
		__key = alloca(strlen(key) + 1);
		lcasecpy(__key, key);
	#endif
	index = hash((unsigned char *)__key) & (tablelen - 1);

	if (!table[index])
		table[index] = VectorInit(HT_INITIAL_SLOTS);
	table[index] = VectorAdd(table[index], newentry);
}


int HtRemoveItem(const char *key, LPVECTOR *table, unsigned int tablelen) {
	int i;
	unsigned int index;
	#ifdef HT_CASE_INSENSITIVE
		char *__key;
		
		__key = alloca(strlen(key) + 1);
		lcasecpy(__key, key);
	#endif
	
	index = hash((unsigned char *)__key) & (tablelen - 1);

	if (table[index]) {
		for (i = 0; i != table[index]->numelem; i++) {
			if (!strilcmp(__key, (const char *)table[index]->elem[i])) {
				table[index]->numelem--;
				free(table[index]->elem[i]);
				table[index]->elem[i] = table[index]->elem[table[index]->numelem];
				return 1;
			}
		}
	}
	return 0;
}


void *HtUnassociateItem(const char *key, LPVECTOR *table, unsigned int tablelen) {
	int i;
	void *item;
	unsigned int index;
	#ifdef HT_CASE_INSENSITIVE
		char *__key;
		
		__key = alloca(strlen(key) + 1);
		lcasecpy(__key, key);
	#endif
	
	index = hash((unsigned char *)__key) & (tablelen - 1);

	if (table[index]) {
		for (i = 0; i != table[index]->numelem; i++) {
			if (!strilcmp(__key, (const char *)table[index]->elem[i])) {
				table[index]->numelem--;
				item = table[index]->elem[i];
				table[index]->elem[i] = table[index]->elem[table[index]->numelem];
				return item;
			}
		}
	}
	return NULL;
}


void *HtGetItem(const char *key, LPVECTOR *table, unsigned int tablelen) {
	int i;
	unsigned int index;
	#ifdef HT_CASE_INSENSITIVE
		char *__key;
		
		__key = alloca(strlen(key) + 1);
		lcasecpy(__key, key);
	#endif
	index = hash((unsigned char *)__key) & (tablelen - 1);

	if (table[index]) {
		for (i = 0; i != table[index]->numelem; i++) {
			if (!strilcmp(__key, (const char *)table[index]->elem[i]))
				return table[index]->elem[i];
		}
	}
	return NULL;
}


void HtResetContents(LPVECTOR *table, unsigned int tablelen) {
	unsigned int i;

	for (i = 0; i != tablelen + 1; i++) {
		if (table[i]) {
			VectorDelete(table[i]);
			table[i] = NULL;
		}
	}
}


unsigned int __fastcall hash(unsigned char *key) {
    unsigned int hash = 0;

	while (*key) {
		hash += *key;
		hash += (hash << 10);
		hash ^= (hash >> 6);
		key++;
	}
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}
