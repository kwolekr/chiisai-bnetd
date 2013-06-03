/*-
 * Copyright (c) 2007 Ryan Kwolek 
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
 * pbuffer.c - 
 *    Session-specific buffer for abstracting and simplifying packet construction/sending
 */


#include "main.h"
#include "user.h"
#include "pbuffer.h"


///////////////////////////////////////////////////////////////////////////////


uint32_t *ReserveDWORD(LPSESS sess) {
	uint32_t *ptr = (uint32_t *)(sess->sendbuf + sess->sbufpos);
	sess->sbufpos += sizeof(uint32_t);
	return ptr;
}


void InsertByte(unsigned char data, LPSESS sess) {
	sess->sendbuf[sess->sbufpos] = data;
	sess->sbufpos++;
}


void InsertWORD(uint16_t data, LPSESS sess) {
	*(uint16_t *)(sess->sendbuf + sess->sbufpos) = data;
	sess->sbufpos += sizeof(uint16_t);
}


void InsertDWORD(uint32_t data, LPSESS sess) {
	*(uint32_t *)(sess->sendbuf + sess->sbufpos) = data;
	sess->sbufpos += sizeof(uint32_t);
}


void InsertNTString(const char *data, LPSESS sess) {
	while (*data) {
		sess->sendbuf[sess->sbufpos] = *data;
		sess->sbufpos++;
		data++;
	}
	sess->sendbuf[sess->sbufpos++] = 0;
}


void InsertNonNTString(const char *data, LPSESS sess) {
	while (*data) {
		sess->sendbuf[sess->sbufpos] = *data;
		sess->sbufpos++;
		data++;
	}
}


void InsertData(void *data, unsigned int len, LPSESS sess) {
	memcpy(sess->sendbuf + sess->sbufpos, data, len);
	sess->sbufpos += len;
}


void InsertZero(unsigned int len, LPSESS sess) {
	memset(sess->sendbuf + sess->sbufpos, 0, len);
	sess->sbufpos += len;
}


void SendPacket(unsigned char PacketID, LPSESS sess) {
	//int i;

	sess->sendbuf[0] = 0xFF;
	sess->sendbuf[1] = PacketID;
	*(uint16_t *)(sess->sendbuf + 2) = sess->sbufpos;
	
	#ifdef VERBOSE
		//printf("[%d] Sending 0x%02x...\n", sess->sessid, PacketID);
	#endif
	/*for (i = 0; i != sess->sbufpos; i++)
		printf("%02x ", (unsigned char)sess->sendbuf[i]);
	puts("");*/
	
	if (send(sess->sck, sess->sendbuf, sess->sbufpos, 0) != sess->sbufpos) {
		printf("Lost client %d connection (%d)\n", sess->sessid, err());
		sess->state |= US_ENDSESS;
	}
	sess->sbufpos = 4;
}
