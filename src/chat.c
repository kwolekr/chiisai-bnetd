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
#include "user.h"
#include "packets.h"
#include "channel.h"
#include "cmds.h"
#include "chat.h"


///////////////////////////////////////////////////////////////////////////////


int ChatFloodOut(LPSESS sess, unsigned int len) {
	return 0;
}


void SendTextToChannel(LPSESS sess, LPCHANNEL channel, const char *username, char *text, int eid) {
	LPSESS user;
	int i;

	if (sess) {
		if (!sess->connected)
			return;

		if (ChatFloodOut(sess, strlen(text))) {
			UserIPBan(sess, 5);
			return;
		}

		if (eid == EID_TALK && *text == '/') {
			ParseUserCommand(sess, text + 1);
			return;
		}
	}

	if (sess && eid == EID_EMOTE)
		Send0x0F(sess, EID_EMOTE, sess->flags, sess->name, text);

	if (!(channel->flags & CF_SILENT)) {
		if ((sess && channel->nusers > 1) || !sess) {
			user = channel->firstuser;
			for (i = 0; i != channel->nusers; i++) {
				if (sess) {
					if (i != sess->channelpos) {
						if (!((user->state & US_IGPUB) && (channel->flags & CF_PUBLIC)) &&
							!((user->state & US_IGPRIV) && !(channel->flags & CF_PUBLIC)))
							Send0x0F(user, eid, sess->flags, username, text);
					}
				} else {
					Send0x0F(user, eid, 0, username, text);
				}
				user = user->nextuser;
			}
		} else {
			Send0x0F(sess, EID_INFO, 0, "", "No one hears you.");
		}
	}
}


void MultiLineSend(LPSESS sess, const char *username, const char *text, int eid) {
	const char *curr;

	curr = text;
	while (curr[0]) {
		Send0x0F(sess, eid, 0, username, curr);
		curr += strlen(curr) + 1;
	}
}


void UserWhisper(LPSESS sess, LPSESS userto, const char *text) {
	Send0x0F(sess, EID_WHISPERSENT, userto->flags, userto->name, text);
	if (!(userto->state & US_IGW))
		Send0x0F(userto, EID_WHISPER, sess->flags, sess->name, text);
}


void WhisperUsername(LPSESS sess, const char *userto, const char *text) {
	char buf[256];
	LPSESS user;

	if (!text || !*text) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_WHATSAY);
		return;
	}

	user = HtGetItem(userto, users, TL_USERS);
	if (!user) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOTLOGON);
		return;
	}

	if (user->state & US_AWAY) {
		sprintf(buf, "%s is away (%s)", user->name, user->awaymsg);
		Send0x0F(sess, EID_INFO, user->flags, user->name, buf);
	}

	if (user->state & US_DND) {
		sprintf(buf, "%s is refusing messages (%s)", user->name, user->dndmsg);
		Send0x0F(sess, EID_INFO, user->flags, user->name, buf);
	} else {
		UserWhisper(sess, user, text);
	}
}
