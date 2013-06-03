/*-
 * Copyright (c) 2009 Ryan Kwolek 
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
 * packets.c - 
 *    Routines to handle incoming/outgoing packets to clients.
 */


#include "main.h"
#include "user.h"
#include "conn.h"
#include "fxns.h"
#include "cmds.h"
#include "chat.h"
#include "ht.h"
#include "logon.h"
#include "nls.h"
#include "srp.h"
#include "profile.h"
#include "friends.h"
#include "channel.h"
#include "pbuffer.h"
#include "packets.h"

int pkhndindex[] = {
    1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 2, 15, 3, 0, 4, 0,
    12, 0, 0, 0, 16, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 17, 5, 18,   0, 14, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 11, 0, 0, 13, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    6, 7, 8, 9, 10, 0, 0, 0,   0, 19, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,	
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0
};

PacketHandler pkthandlers[] = {
    IgnorePacket,
    Parse0x0A,
    Parse0x0C,
    Parse0x0E,

    Parse0x26,
    Parse0x50,
    Parse0x51,
    Parse0x52,
    
	Parse0x53,
    Parse0x54,
	Parse0x3A,
	Parse0x10,

	Parse0x3D,
	Parse0x29,
	Parse0x0B,
	Parse0x14,

	Parse0x25,
	Parse0x27,
	Parse0x59
};

char wc3serversig[128] = {0};


/////////////////////////////////////////////////////////////////


void IgnorePacket(char *data, LPSESS sess) {
    return;
}


void Parse0x0A(char *data, LPSESS sess) {
	/*if (!(sess->pkts & PC_DONECONNECTING)) {
		DisconnectSession(sess);
		return;
	}*/

	UserLogon(sess);
	FriendLoad(sess);

	InsertNTString(sess->name, sess);
	InsertNTString(sess->statstring, sess);
	InsertNTString(sess->username, sess);
	SendPacket(0x0A, sess);
}


void Parse0x0B(char *data, LPSESS sess) {
	int i;

	for (i = 0; i != ARRAYLEN(defchans); i++)
		InsertNTString(defchans[i].name, sess);
	InsertByte(0x00, sess);
	SendPacket(0x0B, sess);
}


void Parse0x0C(char *data, LPSESS sess) {
	LPCHANNEL chan;
	char *targetchan;
	
	targetchan = data + 8;
	switch (*(uint32_t *)(data + 4)) {
		case 0:
			if (!ChannelJoin(targetchan, sess, 1))
				Send0x0F(sess, EID_CHANNELDOESNOTEXIST, 0, "", "");
			break;
		case 1:
		case 6:
			ChannelJoin("Brood War USA-1", sess, 0);
			break;
		case 2:
			chan = HtGetItem(targetchan, channels, TL_CHANNELS);
			if (!chan || ChannelIsAccessible(chan, sess))
				ChannelJoin(targetchan, sess, 0);
			else
				ChannelJoin(BAN_CHANNEL_NAME, sess, 1);
	}
}


void Send0x0F(LPSESS sess, int eid, int flags, const char *user, const char *text) {
	if (sess->sck == -1)
		return;		//for virtual users

	InsertDWORD(eid, sess);
	InsertDWORD(flags, sess);
	InsertDWORD(sess->ping, sess); //////////////////FIXME
	InsertDWORD(0, sess);
	InsertDWORD(0, sess);
	InsertDWORD(0, sess);
	InsertNTString(user, sess);
	InsertNTString(text, sess);
	SendPacket(0x0F, sess);
}


void Parse0x0E(char *data, LPSESS sess) {
	SendTextToChannel(sess, sess->channel, sess->name, data + 4, EID_TALK);
}


void Parse0x10(char *data, LPSESS sess) {
	ChannelLeave(sess);
}


void Parse0x14(char *data, LPSESS sess) {
	if (*(uint32_t *)(data + 4) == 'bnet')
		sess->flags &= ~UF_NOUDP;
}


void Parse0x50(char *data, LPSESS sess) {
	uint32_t platform, client;
	unsigned short pktlen;

	pktlen = *(uint16_t *)(data + 2);
	if (pktlen < 38 || pktlen > 138) {
		SessionReqDisconnect(sess);
		return;
	}

	sess->ping = 0xFFFFFFFF;
	Send0x25(sess);
	SendUDP0x05Burst(sess);

	if (*(uint32_t *)(data + 4)) { //proto ver
		SessionReqDisconnect(sess);
		return;
	}

	platform = *(uint32_t *)(data + 8);
	client   = *(uint32_t *)(data + 12);
	switch (platform) {
		case 'IX86':
		case 'PMAC':
		case 'XMAC':
			break;
		default:
			SessionReqDisconnect(sess);
			return;
	}
	
	SetSessClientInfo(sess, client);

	sess->platform = platform;
	sess->client   = client;
	sess->verbyte  = *(uint32_t *)(data + 16);

	sess->li.prodlang = *(uint32_t *)(data + 20);
	sess->li.tzbias   = *(uint32_t *)(data + 28);
	sess->li.localeid = *(uint32_t *)(data + 32);
	sess->li.langid   = *(uint32_t *)(data + 36);
	strncpy(sess->li.country_s, data + 40, sizeof(sess->li.country_s));
	strncpy(sess->li.country, data + 41 + strlen(data + 40), sizeof(sess->li.country));


	/*
		(DWORD)		 Logon Type
		(DWORD)		 Server Token
		(DWORD)		 UDPValue**
		(FILETIME)	 MPQ filetime
		(STRING) 	 IX86ver filename
		(STRING) 	 ValueString

		Warcraft III Only:
		(VOID)		 128-byte Server signature
	*/
	
	sess->servertoken = rand();

	InsertDWORD((sess->state & US_CLIWC3) ? 2 : 0, sess);
	InsertDWORD(sess->servertoken, sess);
	InsertDWORD(gettick(), sess);
	InsertDWORD(0, sess); //low
	InsertDWORD(0, sess); //high
	InsertNTString("ver-IX86-01.mpq", sess);
	InsertNTString("", sess); //cr formula
	if (sess->state & US_CLIWC3)
		InsertData(wc3serversig, 128, sess);
	SendPacket(0x50, sess);
}


void Parse0x51(char *data, LPSESS sess) {
	int numkeys, result;
	unsigned short pktlen;
	char info[64];

	*info = 0;
	result = 0;
	
	pktlen = *(unsigned short *)(data + 2);
	if (pktlen < 22 || pktlen > 294) {
		SessionReqDisconnect(sess);
		return;
	}

	sess->clienttoken = *(uint32_t *)(data + 4);
	//*(uint32_t *)(data + 8);  //exe ver
	//*(uint32_t *)(data	+ 12); //checksum
	numkeys = *(uint32_t *)(data + 16);
	if (numkeys < 0 || numkeys > 2) {
		SessionReqDisconnect(sess);
		return;
	}

#if 0
	for (i = 0; i != numkeys; i++) {
		sess->cdkey[i].len     = *(uint32_t *)(data + 24 + i * 36);
		sess->cdkey[i].prodval = *(uint32_t *)(data + 28 + i * 36);
		sess->cdkey[i].pubval  = *(uint32_t *)(data + 32 + i * 36);
		switch (sess->cdkey_len) {
			case 13:
				privval = GetPrivateVal13(sess->cdkey_prodval, sess->cdkey_pubval);
				break;
			case 16:
				privval = GetPrivateVal16(sess->cdkey_prodval, sess->cdkey_pubval);
				break;
			case 26:
				privval = GetPrivateVal26(sess->cdkey_prodval, sess->cdkey_pubval);
				break;
			default:
				DisconnectSession(sess);
				return;
		}
		HashCDKey(blahblah);
		if (memcmp(data + i * 36 + 16, hashbuf, 20)) {
			SetIPBan(sess);				       //invalid CDKey
			result = P51STAT_CDKEY | (i << 4); //0x10 set for exp
		}
	}
#endif

	InsertDWORD(result, sess);
	InsertNTString(info, sess);
	SendPacket(0x51, sess);
}


void Parse0x29(char *data, LPSESS sess) {
	uint32_t ctoken, stoken;
	int response;

	ctoken = *(uint32_t *)(data + 4);
	stoken = *(uint32_t *)(data + 8);

	response = LogonAccount(sess, data + 12, ctoken, stoken);

	InsertDWORD(!response, sess);
	SendPacket(0x29, sess);
}


void Parse0x3A(char *data, LPSESS sess) {
	char buf[64], *str;
	uint32_t ctoken, stoken;
	int response;

	ctoken = *(uint32_t *)(data + 4);
	stoken = *(uint32_t *)(data + 8);
	strncpy(sess->username, data + 32, sizeof(sess->username));

	response = LogonAccount(sess, data + 12, ctoken, stoken);
	if (response > 2) {
		sprintf(buf, "Your account has been closed (%d).", response);
		str = buf;
	} else {
		str = "";
	}

	InsertDWORD(response, sess);
	InsertNTString(str, sess);
	SendPacket(0x3A, sess);
}


void Parse0x3D(char *data, LPSESS sess) {
	int response;

	response = AccountCheckName(data + 24);
	if (!response) {
		printf("creating account %s!\n", data + 24);
		response = AccountCreate(data + 24, data + 4, sess->addr.sin_addr.s_addr);
	}

	InsertDWORD(response, sess);
	InsertNTString("", sess);
	SendPacket(0x3D, sess);
}


void SendUDP0x05Burst(LPSESS sess) {
	uint32_t packet[2] = {0x05, 'bnet'};

	sess->flags |= UF_NOUDP;
	sendto(udp_sck, (const char *)packet, sizeof(packet), 0,
		(const struct sockaddr *)&sess->addr, sizeof(sess->addr));
	sendto(udp_sck, (const char *)packet, sizeof(packet), 0,
		(const struct sockaddr *)&sess->addr, sizeof(sess->addr));
	sendto(udp_sck, (const char *)packet, sizeof(packet), 0,
		(const struct sockaddr *)&sess->addr, sizeof(sess->addr));
}


void Send0x25(LPSESS sess) { //broadcast again every 20 seconds
	unsigned int t = gettick();

	sess->lastping = t;
	InsertDWORD((uint32_t)t, sess);
	SendPacket(0x25, sess);
}


void Parse0x25(char *data, LPSESS sess) {
	unsigned int ping;

	if (sess->ping == -1 && !sess->connected) {
		ping = gettick() - sess->lastping;
		if (ping < 0x80000000)
			sess->ping = ping;
	}
}


void Parse0x52(char *data, LPSESS sess) {
	int status;
	
	status = NLSAccountCheckName(data + 68);
	if (!status) {
		printf("creating NLS account %s!\n", data + 68);
		status = NLSAccountCreate(data + 68, data + 4,
					data + 36, sess->addr.sin_addr.s_addr);
	}

	InsertDWORD(status, sess);
	SendPacket(0x52, sess);
}


void Parse0x53(char *data, LPSESS sess) {
	int status;
	char *salt, *serverkey;

	strncpy(sess->username, data + 36, sizeof(sess->username));

	status = NLSAccountLogon(sess, data + 4, &salt, &serverkey);

	InsertDWORD(status, sess);
	if (status == NLSACC_LOGON_OK) {
		InsertData(salt, 32, sess);
		InsertData(serverkey, 32, sess);
	} else {
		InsertZero(64, sess);
	}
	SendPacket(0x53, sess);
}


void Parse0x54(char *data, LPSESS sess) {
	char M2[20];
	int status;

	status = NLSAccountAuthenticate(sess, data + 4, M2);

	if (sess->nls) {
		SRPFree(sess->nls);
		sess->nls = NULL;
	}

	InsertDWORD(status, sess);
	InsertData(M2, sizeof(M2), sess);
	InsertNTString("", sess);
	SendPacket(0x54, sess);
}


void Parse0x59(char *data, LPSESS sess) {
	//EmailSet(data + 4
}
