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
 * chain.c - 
 *    Complete implementation of a linked list for channel listings
 */

#include "main.h"
#include "chain.h"


///////////////////////////////////////////////////////////////////////////////


void ChainInsertItem(LPCHANNEL chan, LPSESS sess, int first) {
	if (first) {
		if (!chan->lastadded) {
			chan->lastadded = sess;
			sess->nextuser = NULL;
		} else {
			sess->nextuser = chan->firstuser;
			chan->firstuser->lastuser = sess;
		}
		sess->lastuser = NULL;
		chan->firstuser = sess;
	} else {
		sess->nextuser = NULL;
		sess->lastuser = chan->lastadded;
		if (!chan->lastadded)
			chan->firstuser = sess;
		else
			chan->lastadded->nextuser = sess;
		chan->lastadded = sess;
	}
}


void ChainRemoveItem(LPCHANNEL chan, LPSESS sess) {
	if (sess->nextuser)
		((LPSESS)sess->nextuser)->lastuser = sess->lastuser;
	else
		chan->lastadded = sess->lastuser;

	if (sess->lastuser)
		((LPSESS)sess->lastuser)->nextuser = sess->nextuser;
	else
		chan->firstuser = sess->nextuser;
}


void ChainMoveToTop(LPCHANNEL chan, LPSESS sess) {
	if (sess->nextuser)
		((LPSESS)sess->nextuser)->lastuser = sess->lastuser;
	else
		chan->lastadded = sess->lastuser;
	if (sess->lastuser)
		((LPSESS)sess->lastuser)->nextuser = sess->nextuser;
	sess->lastuser  = NULL;
	sess->nextuser  = chan->firstuser;
	chan->firstuser = sess;
}
