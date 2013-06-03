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
#include "cmds.h"
#include "fxns.h"
#include "ht.h"
#include "srp.h"
#include "chat.h"
#include "name.h"
#include "conn.h"
#include "channel.h"
#include "packets.h"
#include "friends.h"
#include "user.h"

int nusers_prod[12];
int ngames_prod[12];
int totalgames;

const char *prodname[12] = {
	"Starcraft",
	"Starcraft Broodwar",
	"Starcraft Shareware",
	"Japanese Starcraft",

	"Diablo Shareware",
	"Diablo Retail",
	"Diablo II",
	"Diablo II Expansion",

	"Warcraft II Battle.Net Edition",
	"Warcraft III",
	"Warcraft III: The Frozen Throne",

	"Chat"
};

const char *statstrs[] = {
	"RATS 0 0 0 0 0 0 0 0 RATS",
	"PXES 0 0 0 0 0 0 0 0 PXES",
	"RHSS 0 0 0 0 0 0 0 0 RHSS",
	"RTSJ 0 0 0 0 0 0 0 0 RTSJ",
	"RHSD",
	"LTRD",
	"VD2D",
	"PX2D",
	"NB2W 2000 6 59 0 0 2000 2200 5 NB2W",
	"3RAW 1R3W 0 nlaC",
	"PX3W 1R3W 0 nlaC",
	"TAHC"
};


///////////////////////////////////////////////////////////////////////////////


void UserLogon(LPSESS sess) {
	int number;

	number = NameGetSetNumber(sess->username);
	if (number == 1)
		strcpy(sess->name, sess->username);
	else 
		sprintf(sess->name, "%s#%d", sess->username, number);

	sess->nameno = number;
	HtInsertItem(sess->name, sess, users, TL_USERS); 
	UserGenerateStatstring(sess->statstring, sess);
	sess->connected = 1;
	nusers_prod[sess->clientindex]++;

	if (sess->sck != -1)
		FriendNotify(sess, "Your friend %s has entered Battle.net.", sess->name);
}


void UserLogoff(LPSESS sess) {
	ChannelLeave(sess);

	if (sess->sck != -1) {
		FriendNotify(sess, "Your friend %s has exited Battle.net.", sess->name);
		if (sess->friends) {
			VectorDelete(sess->friends);
			sess->friends = NULL;
		}
		if (sess->nls) {
			SRPFree(sess->nls);
			sess->nls = NULL;
		}
	}

	if (sess->connected) {
		nusers_prod[sess->clientindex]--;
		NameUnsetNumber(sess);
		HtUnassociateItem(sess->name, users, TL_USERS);
	}
}


void UserIPBan(LPSESS sess, int duration) {

	SessionReqDisconnect(sess);
	return; //////////////////////////////////////////////////////
}


void HandleTimeCmd(LPSESS sess) {
	char buf[256];
	time_t rt;
	struct tm *timeptr;

	time(&rt);
	timeptr = localtime(&rt);
	strftime(buf, sizeof(buf), "Battle.net time: %a %b %d  %I:%M %p", timeptr);
	Send0x0F(sess, EID_INFO, 0, "", buf);

	timeptr->tm_hour -= sess->li.tzbias / 60;
	strftime(buf, sizeof(buf), "Your local time: %a %b %d  %I:%M %p", timeptr);
	Send0x0F(sess, EID_INFO, 0, "", buf);
}


void HandleWhoisCmd(LPSESS sess, const char *arg) {
	LPSESS user;
	const char *channel;
	char buf[256], asdf[128];

	if (!arg)
		goto notfound;

	if (!strcasecmp(sess->username, arg)) {
		HandleWhoamiCmd(sess);
		return;
	}

	user = HtGetItem(arg, users, TL_USERS);
	if (!user) {
		notfound:
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOTLOGON);
		return;
	}

	if (user->channel) {
		if (user->channel->flags & CF_PUBLIC) {
			sprintf(asdf, "the channel %s", user->channel->name);
			channel = asdf;
		} else {
			channel = "a private channel";
		}
	} else {
		channel = REALM_NAME_SC;
	}

	sprintf(buf, "%s is using %s in %s.", user->name, prodname[sess->clientindex], channel);
	Send0x0F(sess, EID_INFO, user->flags, "", buf);

	if (user->state & US_AWAY) {
		sprintf(buf, "%s is away (%s)", user->name, user->awaymsg);
		Send0x0F(sess, EID_INFO, user->flags, "", buf);
	}
	if (user->state & US_DND) {
		sprintf(buf, "%s is refusing messages (%s)", user->name, user->awaymsg);
		Send0x0F(sess, EID_INFO, user->flags, "", buf);
	}
}


void HandleWhoamiCmd(LPSESS sess) {
	const char *channel;
	char buf[256], asdf[128];

	if (sess->channel) {
		if (sess->channel->flags & CF_PUBLIC) {
			sprintf(asdf, "the channel %s", sess->channel->name);
			channel = asdf;
		} else {
			channel = "a private channel";
		}
	} else {
		channel = REALM_NAME_SC;
	}

	sprintf(buf, "You are %s, using %s in %s.", sess->name, prodname[sess->clientindex], channel);
	Send0x0F(sess, EID_INFO, sess->flags, "", buf);

	if (sess->state & US_AWAY) {
		sprintf(buf, "You are away (%s)", sess->awaymsg);
		Send0x0F(sess, EID_INFO, sess->flags, "", buf);
	}
	if (sess->state & US_DND) {
		sprintf(buf, "You are refusing messages (%s)", sess->awaymsg);
		Send0x0F(sess, EID_INFO, sess->flags, "", buf);
	}
}


void HandleUsersCmd(LPSESS sess) {
	char buf[256];
	int ci;
	int totalusers;

	ci = sess->clientindex;
	totalusers = nusers_bncs + nusers_telnet + nusers_reps;
	sprintf(buf, "There are currently %d users playing %d games of %s, "\
		"and %d users playing %d games on Battle.net.",
		nusers_prod[ci], ngames_prod[ci], prodname[ci], totalusers, totalgames);
	Send0x0F(sess, EID_INFO, 0, "", buf);
}


void UserGenerateStatstring(char *dest, LPSESS sess) {
	strcpy(dest, statstrs[sess->clientindex]);
/*
[11:00:29 AM] PXES 0 0 0 0 0 0 0 0 PXES
[11:00:29 AM] PX2DEurope,sarcophage,„€
[11:00:29 AM] NB2W 0 0 0 0 0 0 0 0 NB2W

for Starcraft, only normal_wins are written, rest are zeros

ex:

[11:15:37 AM] Pro_Tech's record:
[11:15:37 AM] Normal games: 59-0-1
[11:15:37 AM] Ladder games: 20-0-0 (rating 2000)
[11:15:37 AM] IronMan games: 20-0-0 (rating 2200)
[11:00:29 AM] NB2W 2000 6 59 0 0 2000 2200 5 NB2W

[11:00:29 AM] VD2D
*/

	/*sprintf("%s %d %d %d %d %d %d %d %d %s",
		client,
		ladder_rating, discs(6), normal_wins, normal_losses,
		unknown, ladder_rating, ironman_Rating, unknown(5),
		icon);*/
}

void HandleAwayCmd(LPSESS sess, const char *arg) {
	char buf[128];

	if (arg) {
		sess->state |= US_AWAY;
		strncpy(sess->awaymsg, arg, sizeof(sess->awaymsg));
		sess->awaymsg[sizeof(sess->awaymsg) - 1] = 0;
	} else {
		sess->state ^= US_AWAY;
		if (sess->state & US_AWAY)
			strcpy(sess->awaymsg, "Not available");
	}
	sprintf(buf, "You are %s marked as being away.",
		(sess->state & US_AWAY) ? "now" : "no longer");
	Send0x0F(sess, EID_INFO, 0, "", buf); 
}


void HandleDndCmd(LPSESS sess, const char *arg) {
	char buf[128];

	if (arg) {
		sess->state |= US_DND;
		strncpy(sess->dndmsg, arg, sizeof(sess->dndmsg));
		sess->dndmsg[sizeof(sess->dndmsg) - 1] = 0;
	} else {
		sess->state ^= US_DND;
		if (sess->state & US_DND)
			strcpy(sess->awaymsg, "Not available");
	}
	sprintf(buf, "Do Not Disturb mode %s.",
		(sess->state & US_DND) ? "engaged" : "cancelled");
	Send0x0F(sess, EID_INFO, 0, "", buf); 
}


void HandleStatsCmd(LPSESS sess, const char *user) {
	Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_TOOMANYREQ);
}


void HandleListChanCmd(LPSESS sess) {
	LPCHANNEL chan;
	char buf[128];
	int i, j;

	for (i = 0; i != TL_CHANNELS + 1; i++) {
		if (channels[i]) {
			for (j = 0; j != channels[i]->numelem; j++) {
				chan = channels[i]->elem[j];
				sprintf(buf, "%s (%d) 0x%04x", chan->name, chan->nusers, chan->flags & ~CFI_ALL);
				Send0x0F(sess, EID_INFO, 0, "", buf);
			}
		}
	}
}



void HandleOptionsCmd(LPSESS sess, const char *arg) {
	char buf[128];
	int no, flag;

	if (!arg) {
		sprintf(buf, "%sing private messages from non-friends.",
			(sess->state & US_IGW) ? "Ignor" : "Allow");
		Send0x0F(sess, EID_INFO, 0, "", buf);

		sprintf(buf, "%sing messages from non-friends in public channels.",
			(sess->state & US_IGPUB) ? "Ignor" : "Allow");
		Send0x0F(sess, EID_INFO, 0, "", buf);

		sprintf(buf, "%sing messages from non-friends in private channels.",
			(sess->state & US_IGPRIV) ? "Ignor" : "Allow");
		Send0x0F(sess, EID_INFO, 0, "", buf);

		sprintf(buf, "%sllowing e-mails from fellow clan members, including clan announcements.",
			(sess->state & US_FRIENDMAIL) ? "A" : "Not a");
		Send0x0F(sess, EID_INFO, 0, "", buf);

		sprintf(buf, "%sllowing e-mails from users on my friends list.",
			(sess->state & US_CLANMAIL) ? "A" : "Not a");
		Send0x0F(sess, EID_INFO, 0, "", buf);
	} else {
		flag = 0;
		no   = 0;
		if ((arg[0] == 'u' && arg[1] == 'n') ||
			(arg[0] == 'n' && arg[1] == 'o'))
			no = 1;
		//completely unfinished!! TODO: implement these
		//"ignorewhispers"
		//"ignorepublic"
		//"ignoreprivate"
		//"friendmail"
		//"clanmail"
		if (no)
			sess->state &= ~flag;
		else
			sess->state |= flag;
	}
}
