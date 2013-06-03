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
#include "vector.h"
#include "ht.h"
#include "name.h"

//const char valid[] = "0123456789abcdefghijklmnopqrstuvwxyz!$&'()+-.:;=@[]^_`{|}~";
//const char validnls[] = "0123456789abcdefghijklmnopqrstuvwxyz()-.[]_";
int invalidchars[] = {
	~0x00000000,  // 00-1f:  0000 0000 0000 0000  0000 0000 0000 0000
	~0x2FFF6BD2,  // 20-3f:  0100 1011 1101 0110  1111 1111 1111 0100
	~0xEFFFFFFF,  // 40-5f:  1111 1111 1111 1111  1111 1111 1111 0111
	~0x7FFFFFFF   // 60-7f:  1111 1111 1111 1111  1111 1111 1111 1110
};

int invalidnlschars[] = {
	~0x00000000,  // 00-1f:  0000 0000 0000 0000  0000 0000 0000 0000
	~0x23FF2300,  // 20-3f:  0000 0000 1100 0100  1111 1111 1100 0100
	~0xAFFFFFFE,  // 40-5f:  0111 1111 1111 1111  1111 1111 1111 0101
	~0x07FFFFFE   // 60-7f:  0111 1111 1111 1111  1111 1111 1110 0000
};

///////////////////////////////////////////////////////////////////////////////


int NameInvalidChar(unsigned char chr) {
	return (chr > 0x7F) || (invalidchars[chr >> 5] & (1 << (chr & 0x1F)));
}


int NameNLSInvalidChar(unsigned char chr) {
	return (chr > 0x7F) || (invalidchars[chr >> 5] & (1 << (chr & 0x1F)));
}


int NameIsValid(const char *name) {
	unsigned char chr;
	int i;

	i = 0;
	while (chr = name[i]) {
		if ((chr > 0x7F) || (invalidchars[chr >> 5] & (1 << (chr & 0x1F))))
			return 0;
		i++;
	}
	return i < 16;
}


void NameRandomGenerate(char *buf) {
	int i;
	
	for (i = 0; i != MAX_USERNAME_LEN - 1; i++) {
		buf[i] = (rand() & 0xFF);
	}
	buf[i] = 0;
}


void NameUnsetNumber(LPSESS sess) {
	LPNAMENODESC nndesc;
	char buf[32];
	int index;
	
	strcpy(buf, sess->username);
	strcat(buf, "#");
	nndesc = HtGetItem(buf, users, TL_USERS);
	if (!nndesc)
		return;

	index = (sess->nameno - 1) / BP_BITBLOCK;
	nndesc->bitfield[index] &= ~(1 << ((sess->nameno - 1) & (BP_BITBLOCK - 1)));
	nndesc->popcount--;
	if (!nndesc->popcount) {
		free(nndesc->bitfield);
		HtRemoveItem(buf, users, TL_USERS);
	}
}


int NameGetSetNumber(const char *username) {
	LPNAMENODESC nndesc;
	unsigned int val;
	int i, oldsiz, index;
	char buf[32];
	LPSESS fuser;

	strcpy(buf, username); ////might not be safe, check the source of username
	strcat(buf, "#");

	fuser  = HtGetItem(username, users, TL_USERS);
	nndesc = HtGetItem(buf, users, TL_USERS);

	if (!nndesc) {
		if (!fuser) {
			return 1;
		} else {
			nndesc = malloc(sizeof(NAMENODESC));
			nndesc->arrlen = 2;
			nndesc->bitfield = malloc(2 * sizeof(nndesc->bitfield[0]));
			nndesc->bitfield[0] = 3;
			nndesc->bitfield[1] = 0;
			nndesc->popcount = 2;
			strcpy(nndesc->name, buf);
			HtInsertItem(buf, nndesc, users, TL_USERS);
			return 2;
		}
	} else {
		if (fuser) {
			for (i = 0; i != nndesc->arrlen; i++) {
				val = ~nndesc->bitfield[i];
				if (val) {
					index = bitscanfwd(val);
					nndesc->bitfield[i] |= (1 << index);
					nndesc->popcount++;
					if (nndesc->popcount / BP_BITBLOCK == nndesc->arrlen) {
						oldsiz = nndesc->arrlen * sizeof(nndesc->bitfield[0]);
						nndesc->arrlen <<= 1;
						nndesc->bitfield = realloc(nndesc->bitfield, nndesc->arrlen * sizeof(nndesc->bitfield[0]));
						memset((char *)nndesc->bitfield + oldsiz, 0, oldsiz);
					}
					return (i * BP_BITBLOCK) + index + 1;
				}
			}
			return 0;
		} else {
			nndesc->bitfield[0] |= 1;
			nndesc->popcount++;
			return 1;
		}
	}
}


#if 0

void DupnameAddToFreeRange(LPSESS sess) {
	LPNAMENODESC nndesc;
	LPRANGELINK rl, lastrl, newrl, tmprl;
	char buf[32];
	int number;

	strcpy(buf, sess->username);
	strcat(buf, "#");
	
	nndesc = HtGetItem(buf, users, TL_USERS);
	if (!nndesc) {
		printf("WARNING: DupnameAddToFreeRange called with no sibling NAMENODESC structure!\n");
		return;
	}

	rl = nndesc->nfree;
	if (!rl)
		return;

	lastrl = rl;
	number = sess->nameno;
	while (rl) {
		if (number == rl->ubound + 1) {
			rl->ubound++;
			tmprl = rl->next;
			if (tmprl && (tmprl->lbound == rl->ubound)) { //merge
				rl->ubound = tmprl->ubound;
				rl->next = tmprl->next;
				free(tmprl);
				return;
			}
		}
		if (number == rl->lbound - 1) {
			rl->lbound--;
			if (lastrl->ubound == rl->lbound)) { //merge
				lastrl->ubound = rl->ubound;
				lastrl->next = rl->next;
				free(rl);
				return;
			}
		}
		if ((lastrl->ubound < number) && (rl->lbound > number)) {
			newrl = malloc(sizeof(RANGELINK));
			newrl->lbound = number;
			newrl->ubound = number;
			newrl->next   = rl;
			lastrl->next  = newrl;
			return;
		}
		if ((number >= rl->lbound) && (number <= rl->ubound)) {
			printf("WARNING: DupnameAddToFreeRange called with SESS's number already unset!\n");
			return;
		}
		rl = rl->next;
	}
	newrl = malloc(sizeof(RANGELINK));
	newrl->lbound = number;
	newrl->ubound = number;
	newrl->next   = NULL;
	lastrl->next  = newrl;
}


void DupnameGetNumber(const char *username) {
	LPNAMENODESC nndesc;
	LPRANGELINK	rl;
	LPSESS user;
	char buf[32];
	int number;

	strcpy(buf, username);
	strcat(buf, "#");

	fuser  = HtGetItem(username, users, TL_USERS);
	nndesc = HtGetItem(buf, users, TL_USERS);
	if (!nndesc) {
		if (fuser) {
			nndesc = malloc(sizeof(NNAMENODESC));
			strcpy(nndesc->name, buf);
			nndesc->nameno_highest = 2;
			nndesc->ndupnames      = 2; ///make this atomic
			
			rl = malloc(sizeof(RANGELINK));
			rl->next   = NULL;
			rl->lbound = 0;
			rl->ubount = 0;

			nndesc->nfree = rl;
			return 2;
		} else {
			return 1;
		}
	} else {
		if (nndesc->nameno_highest != nndesc->ndupnames) {
			rl = nndesc->nfree;
			number = rl->lbound++; //make this atomic
			if (rl->lbound == rl->ubound) {
				nndesc->nfree = rl->next;
				free(rl);
			}
			nndesc->ndupnames++; //atomic
			return number;
		} else {
			number = nndesc->nameno_highest;
			nndesc->nameno_highest++;
			nndesc->ndupnames++; //atomic
			return number;
		}
	}
}

#endif
