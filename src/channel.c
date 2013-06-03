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
 * channel.c - 
 *    Routines to create, service, and maintain channels
 */


#include "main.h"
#include "fxns.h"
#include "ht.h"
#include "user.h"
#include "cmds.h"
#include "chat.h"
#include "packets.h"
#include "chain.h"
#include "vector.h"
#include "packets.h"
#include "channel.h"

LPVECTOR channels[TL_CHANNELS];

CHANDESC defchans[6] = {
	{BAN_CHANNEL_NAME, CFI_NODESTROY | CF_PUBLIC | CF_SILENT, 65535, 0},
	{"Open Tech Support", CFI_NODESTROY | CF_PUBLIC, 200, 0},
	{"Blizzard Tech Support", CFI_NODESTROY | CF_PUBLIC | CF_MODERATED, 40, 0},
	{"Clan Recruitment", CFI_NODESTROY | CF_PUBLIC, 200, 0},
	{"24424LKN", CFI_NODESTROY | CF_PUBLIC, 200, 0},
	{"Fansite Chat", CFI_NODESTROY | CF_MODERATED, 1000, 0}
};


///////////////////////////////////////////////////////////////////////////////


int ChannelIsAccessible(LPCHANNEL lpchan, LPSESS sess) {
	if (!lpchan) {
		#ifdef VERBOSE
			printf("WARNING: ChannelIsAccessible(NULL, sess) called!\n");
		#endif
		return 0;
	}

	if ((lpchan->flags & CF_RESTRICTED) &&
		!(sess->flags & (UF_REP | UF_ADMIN)))
		return 0;
	if (lpchan->flags & CF_PRODSPECIFIC) {
		if (!(sess->state & lpchan->product))
			return 0;
	}
	if (lpchan->limit && (lpchan->nusers >= lpchan->limit) &&
		!(sess->flags & (UF_REP | UF_ADMIN)) &&
		!(0 /*check if in clan here, clan members can go over the limit*/))
		return 0;

	//check if clan is in cp but user is member

	return 1;
}


LPCHANNEL ChannelCreate(const char *channel, unsigned int flags, unsigned int limit, unsigned int product) {
	LPCHANNEL chan;

	if (!channel)
		return NULL;

	while (*channel == '#')
		channel++;
	
	if (!*channel)
		return NULL;

	if (HtGetItem(channel, channels, TL_CHANNELS)) {
		if (verbose)
			printf("WARNING: channel %s already exists!", channel);
		return NULL;
	}

	chan = malloc(sizeof(CHANNEL));

	chan->flags     = flags;
	chan->limit     = limit;
	chan->product   = product;
	chan->nusers    = 0;
	chan->bans      = NULL;
	chan->firstuser = NULL;
	chan->lastadded = NULL;

	strncpy(chan->name, channel, sizeof(chan->name));
	chan->name[sizeof(chan->name) - 1] = 0;

	if (!strncasecmp(chan->name, MOD_CHAN_PREFIX, sizeof(MOD_CHAN_PREFIX) - 1))
		chan->flags |= CFI_OPCHANNEL;
	else if (!strncasecmp(chan->name, CLAN_CHAN_PREFIX, sizeof(CLAN_CHAN_PREFIX) - 1))
		chan->flags |= CFI_CLANCHANNEL;

	HtInsertItem(chan->name, chan, channels, TL_CHANNELS);

	return chan;
}


void ChannelDestroy(LPCHANNEL chan) {
	if (!chan) {
		if (verbose)
			printf("WARNING: ChannelDestroy(NULL) called!\n");
		return;
	}

	HtRemoveItem(chan->name, channels, TL_CHANNELS);
}


int ChannelJoin(const char *channel, LPSESS sess, int fail_if_nonexistant) {
	LPCHANNEL chan;
	LPSESS user;
	char *asdf;
	int i;

	if (!channel)
		return 0;
	
	while (*channel == '#')
		channel++;

	chan = HtGetItem(channel, channels, TL_CHANNELS);
	if (!chan) {
		if (fail_if_nonexistant) {
			return 0;
		} else {
			chan = ChannelCreate(channel, 0, 40, 0);
			if (!chan) {
				Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_INVALCHAN);
				return 0;
			}
		}
	}

	if (chan == sess->channel)
		return 1;

	if (!ChannelIsAccessible(chan, sess)) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_CHANRESTR);
		return 0;
	}

	if (chan->bans) {
		asdf = HtGetItem(sess->name, chan->bans, TL_BANS);
		if (asdf) {
			Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_BANNED);
			return 0;
		}
	}

	ChannelLeave(sess);

	sess->channelpos = chan->nusers;
	ChainInsertItem(chan, sess, 0);
	chan->nusers++;

	sess->channel = chan;
	Send0x0F(sess, EID_CHANNEL, chan->flags & ~CFI_ALL, "", chan->name);

	if (!(chan->flags & CF_SILENT)) {
		user = chan->firstuser;
		for (i = 0; i != chan->nusers; i++) {
			Send0x0F(sess, EID_SHOWUSER, user->flags, user->name, user->statstring);
			if (i != chan->nusers - 1)
				Send0x0F(user, EID_JOIN, sess->flags, sess->name, sess->statstring);
			user = user->nextuser;
		}
	}

	if (chan->flags & CF_SILENT)
		Send0x0F(sess, EID_INFO, 0, "", RESPONSE_NOCHATPRIV);

	if (!strcasecmp(sess->name, chan->name + sizeof(MOD_CHAN_PREFIX)))
		ChannelGiveOp(sess);

	return 1;
}


void ChannelLeave(LPSESS sess) {
	int i;
	LPSESS user;
	LPCHANNEL chan = sess->channel;

	if (!chan)
		return;
	
	ChainRemoveItem(chan, sess);

	sess->channelpos = -1;
	sess->channel	 = NULL;
	sess->nextuser   = NULL;
	sess->lastuser   = NULL;

	chan->nusers--;

	sess->flags &= ~(UF_OP | UF_SPEAKER);
	if (sess->designated)
		ChannelGiveOp(sess->designated);

	if (chan->nusers) {
		if (!(chan->flags & CF_SILENT)) {
			user = chan->firstuser;
			for (i = 0; i != chan->nusers; i++) {
				if (user->designated == sess)
					user->designated = NULL;
				Send0x0F(user, EID_LEAVE, sess->flags, sess->name, "");	
				user = user->nextuser;
			}
		}
	} else {
		if (!chan->flags & CFI_NODESTROY)
			ChannelDestroy(chan);
	}
}


void ChannelAddBan(LPSESS sess, LPCHANNEL chan) {
	char *buf;

	if (!chan->bans) {
		chan->bans = malloc(sizeof(LPVECTOR) * TL_BANS);
		memset(chan->bans, 0, sizeof(LPVECTOR) * TL_BANS);
	}
	if (chan->nbans >= CHANNEL_BAN_LIMIT) {
		chan->nbans = 0;
		HtResetContents(chan->bans, TL_BANS);
	}

	buf = malloc(MAX_NAME_LEN);
	strcpy(buf, sess->name);
	HtInsertItem(buf, buf, chan->bans, TL_BANS);
	chan->nbans++;
}


int ChannelRemoveBan(LPSESS sess, LPCHANNEL chan) {
	int removed;

	if (chan->bans) {
		removed = HtRemoveItem(sess->name, chan->bans, TL_BANS);
		if (removed)
			chan->nbans--;
		return removed;
	}
	return 0;
}


void ChannelGiveOp(LPSESS sess) {
	LPCHANNEL chan;
	LPSESS user;
	int i;

	chan = sess->channel;
	if (!chan)
		return;

	sess->flags |= UF_OP;

	user = chan->firstuser;
	for (i = 0; i != chan->nusers; i++) {
		Send0x0F(user, EID_USERFLAGS, sess->flags, sess->name, "");
		user = user->nextuser;
	}
}


void HandleWhoCmd(LPSESS sess, const char *channel) {
	LPCHANNEL chan;
	LPSESS user;
	char buf[128], *cur, *asdf;
	int nusers;

	if (!channel) {
		if (!sess->channel) {
			Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_WHICHCHAN);
			return;
		}
		chan = sess->channel;
	} else {
		chan = HtGetItem(channel, channels, TL_CHANNELS);
		if (!chan) {
			MultiLineSend(sess, "", RESPONSE_NOCHANNEL, EID_ERROR);
			return;
		}

		if (chan->bans) {
			asdf = HtGetItem(sess->name, chan->bans, TL_BANS);
			if (asdf) {
				Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOPERMIS);
				return;
			}
		}
	}

	sprintf(buf, "Users in channel %s:", chan->name);
	Send0x0F(sess, EID_INFO, chan->flags & ~CFI_ALL, "", buf);

	nusers = chan->nusers;
	user   = chan->firstuser;

	while (nusers > 1) {
		cur = buf;
		if (user->flags & 0x0F) {
			*cur++ = '[';
			ucasecpy(cur, user->name);
			cur += strlen(user->name);
			*cur++ = ']';
		} else {
			strcpy(cur, user->name);
			cur += strlen(user->name);
		}
		user = user->nextuser;
		*cur++ = ',';
		*cur++ = ' ';
		if (user->flags & 0x0F) {
			*cur++ = '[';
			ucasecpy(cur, user->name);
			cur += strlen(user->name);
			*cur++ = ']';
		} else {
			strcpy(cur, user->name);
			cur += strlen(user->name);
		}
		user = user->nextuser;
		Send0x0F(sess, EID_INFO, chan->flags & ~CFI_ALL, "", buf);
		nusers -= 2;
	}

	if (nusers) {
		cur = buf;
		if (user->flags & 0x0F) {
			*cur++ = '[';
			ucasecpy(cur, user->name);
			cur += strlen(user->name);
			*cur++ = ']';
		} else {
			strcpy(cur, user->name);
			cur += strlen(user->name);
		}
		Send0x0F(sess, EID_INFO, chan->flags & ~CFI_ALL, "", buf);
	}
}


void ChannelCreateDefaults() {
	int i, nc;

	nc = 0;
	for (i = 0; i != ARRAYLEN(defchans); i++) {
		if (ChannelCreate(defchans[i].name, defchans[i].flags, defchans[i].limit, defchans[i].product))
			nc++;
	}
	printf(" - %u/%u default channels created\n", nc, (unsigned int)ARRAYLEN(defchans));
}


const char *ChannelGetName(LPCHANNEL chan, int showpriv) {
	if (!chan)
		return REALM_NAME_SC;

	if ((chan->flags & CF_PUBLIC) || showpriv)
		return chan->name;

	return "a private channel";
}
