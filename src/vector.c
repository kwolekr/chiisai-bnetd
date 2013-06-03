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
 * vector.c - 
 *    Routines to maintain a dynamic, auto-resizing array with built in counter.
 */

#include "main.h"
#include "vector.h"


///////////////////////////////////////////////////////////////////////////////


LPVECTOR VectorInit(unsigned int size) {
	LPVECTOR vect;
	
	vect = (LPVECTOR)malloc(sizeof(VECTOR) + size * sizeof(void *));
	vect->numelem = 0;
	vect->maxelem = size;
	return vect;
}


LPVECTOR VectorAdd(LPVECTOR vector, void *item) {
	if (!vector)
		vector = VectorInit(VECTOR_DEFAULT_SIZE);

	if (vector->numelem == vector->maxelem) {
		vector->maxelem <<= 1;
		vector = realloc(vector, sizeof(VECTOR) + vector->maxelem * sizeof(void *));
	}
	vector->elem[vector->numelem] = item;
	vector->numelem++;
	return vector;
}


int VectorRemove(LPVECTOR vector, void *item) {
	int i, found = 0;

	if (!vector)
		return 0;

	for (i = 0; i != vector->numelem; i++) {
		if (found)
			vector->elem[i - 1] = vector->elem[i];
		if (vector->elem[i] == item) {
			found = 1;
			vector->numelem--;
		}
	}
	#ifdef VECTOR_TRIM_ARRAYS
		if (vector->numelem < (vector->maxelem >> 2)) {
			vector->maxelem >>= 1;
			vector = realloc(vector, sizeof(VECTOR) + vector->maxelem * sizeof(void *));
		}
	#endif
	return found;
}


void VectorRemoveItem(LPVECTOR vector, int index) {
	int i;

	if (!vector || index >= vector->numelem)
		return;

	for (i = index; i != vector->numelem; i++)
		vector->elem[i] = vector->elem[i + 1];

	vector->numelem--;
}


void VectorClear(LPVECTOR vector) {
	int i;
	
	for (i = 0; i != vector->numelem; i++)
		free(vector->elem[i]);
	vector->numelem = 0;
}


void VectorDelete(LPVECTOR vector) {
	int i;

	for (i = 0; i != vector->numelem; i++)
		free(vector->elem[i]);
	free(vector);
}
