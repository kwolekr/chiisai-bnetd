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
#include "vector.h"
#include "ht.h"
#include "packets.h"
#include "chat.h"
#include "channel.h"
#include "cmds.h"
#include "op.h"


void KickUser(LPSESS kicker, const char *username) {
	LPCHANNEL chan;
	LPSESS user;
	LPSESS chuser;
	char buf[128];
	int i;

	if (!(kicker->flags & 0x0F)) {
		Send0x0F(kicker, EID_ERROR, 0, "", RESPONSE_NOTOP);
		return;
	}

	if (!username || ((user = HtGetItem(username, users, TL_USERS)) == NULL)) {
		Send0x0F(kicker, EID_ERROR, 0, "", RESPONSE_NOTLOGON);	
		return;
	}

	if (user->channel != kicker->channel) {
		Send0x0F(kicker, EID_ERROR, 0, "", RESPONSE_INVALUSER);
		return;
	}

	if ((user->flags & 0x0F) && (kicker->flags & 0x06)) {
		Send0x0F(kicker, EID_ERROR, 0, "", RESPONSE_NOOPKICK);
		return;
	}

	chan = kicker->channel;
	sprintf(buf, "%s was kicked by %s.", user->name, kicker->name);
	chuser = chan->firstuser;
	for (i = 0; i != chan->nusers; i++) {
		Send0x0F(chuser, EID_INFO, 0, kicker->name, buf);
		chuser = chuser->nextuser;	
	}

	ChannelJoin(BAN_CHANNEL_NAME, user, 0);
}


void BanUser(LPSESS banner, const char *username, int ban) {
	LPCHANNEL chan;
	LPSESS user;
	LPSESS chuser;
	char buf[128];
	int i;

	if (!(banner->flags & 0x0F)) {
		Send0x0F(banner, EID_ERROR, 0, "", RESPONSE_NOTOP);
		return;
	}

	if (!username || ((user = HtGetItem(username, users, TL_USERS)) == NULL)) {
		Send0x0F(banner, EID_ERROR, 0, "", RESPONSE_NOTLOGON);	
		return;
	}

	if (((user->flags & 0x0F) && (banner->flags & 0x06)) &&
		(user->channel == banner->channel)) {
		Send0x0F(banner, EID_ERROR, 0, "", RESPONSE_NOOPBAN);
		return;
	}

	chan = banner->channel;
	if (ban) {
		ChannelAddBan(user, chan);
	} else {
		if (!ChannelRemoveBan(user, chan)) {
			Send0x0F(banner, EID_ERROR, 0, "", RESPONSE_NOTBANNED);
			return;
		}
	}

	sprintf(buf, "%s was %sbanned by %s.", user->name, ban ? "" : "un", banner->name);
	chuser = chan->firstuser;
	for (i = 0; i != chan->nusers; i++) {
		Send0x0F(chuser, EID_INFO, 0, banner->name, buf);
		chuser = chuser->nextuser;	
	}

	if (ban)
		ChannelJoin(BAN_CHANNEL_NAME, user, 0);
}


void DesignateUser(LPSESS sess, const char *username) {
	LPSESS user;
	char buf[64];

	if (!(sess->flags & 0x0F)) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOTOP);
		return;
	}

	user = HtGetItem(username, users, TL_USERS);
	if (!user) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOTLOGON);
		return;
	}
	if (user->channel != sess->channel) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_INVALUSER);
		return;
	}

	sess->designated = user;

	sprintf(buf, "%s is your new designated heir.", user->name);
	Send0x0F(sess, EID_INFO, 0, "", buf);
}
