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

#ifndef CHANNEL_HEADER
#define CHANNEL_HEADER

#define CF_PUBLIC		 0x01
#define CF_MODERATED	 0x02
#define CF_RESTRICTED	 0x04
#define CF_SILENT		 0x08
#define CF_SYSTEM		 0x10
#define CF_PRODSPECIFIC  0x20
#define CF_GLOBALACCESS	 0x1000
#define CF_ALL			 (CF_PUBLIC | CF_MODERATED | CF_RESTRICTED | \
						  CF_SILENT | CF_SYSTEM | CF_PRODSPECIFIC | CF_GLOBALACCESS)

#define CFI_OPCHANNEL	 0x2000
#define CFI_CLANCHANNEL  0x4000
#define CFI_NODESTROY	 0x8000
#define CFI_ALL			 (CFI_OPCHANNEL | CFI_CLANCHANNEL | CFI_NODESTROY)

typedef struct _chandesc {
	const char *name;
	unsigned short flags;
	unsigned short limit;
	unsigned short product;
} CHANDESC, LPCHANDESC;

extern CHANDESC defchans[6];

int ChannelIsAccessible(LPCHANNEL chan, LPSESS sess);
LPCHANNEL ChannelCreate(const char *channel, unsigned int flags, unsigned int limit, unsigned int product);
void ChannelDestroy(LPCHANNEL chan);
int ChannelJoin(const char *channel, LPSESS sess, int fail_if_nonexistant);
void ChannelLeave(LPSESS sess);
void HandleWhoCmd(LPSESS sess, const char *channel);
void ChannelAddBan(LPSESS sess, LPCHANNEL chan);
int ChannelRemoveBan(LPSESS sess, LPCHANNEL chan);
void ChannelGiveOp(LPSESS sess);
void ChannelCreateDefaults();
const char *ChannelGetName(LPCHANNEL chan, int showpriv);

#endif //CHANNEL_HEADER
