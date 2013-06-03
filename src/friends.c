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
#include "chat.h"
#include "cmds.h"
#include "user.h"
#include "name.h"
#include "channel.h"
#include "packets.h"
#include "friends.h"

const char *friendcmds[] = {
	"list",
	"add",
	"remove",
	"msg",
	"promote",
	"demote"
};


///////////////////////////////////////////////////////////////////////////////


void FriendLoad(LPSESS sess) {
	char query[128];
	LPFRIEND pfriend;
	const char *name;
	const char *pztail;
	sqlite3_stmt *pstmt;
	int len, rc, nrows, done;
	
	len = sprintf(query, "SELECT friend, mutual FROM friends WHERE user=? ORDER BY pos");
	rc  = sqlite3_prepare_v2(db_accounts, query, len, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		printf("sqlite3_prepare_v2() failed in FriendLoad(): %s\n", sqlite3_errmsg(db_accounts));
		return;
	}
	
	sqlite3_bind_text(pstmt, 1, sess->username, -1, SQLITE_STATIC);
	nrows = 0;
	done  = 0;
	while (!done) {
		rc = sqlite3_step(pstmt);
		switch (rc) {
			case SQLITE_ROW:
				nrows++;
				name = sqlite3_column_text(pstmt, 0);
				if (!name) {
					printf("sqlite3_column_text() == NULL!\n");
					sqlite3_finalize(pstmt);
					return;
				}
				
				pfriend = malloc(sizeof(FRIEND));
				pfriend->mutual = sqlite3_column_int(pstmt, 1);
				
				strncpy(pfriend->name, name, sizeof(pfriend->name));
				pfriend->name[sizeof(pfriend->name) - 1] = 0;

				sess->friends = VectorAdd(sess->friends, pfriend);
				break;
			case SQLITE_DONE:
				done = 1;
				break;
			case SQLITE_ERROR:
				printf("sqlite3_step() error: %s\n", sqlite3_errmsg(db_accounts));
				done = 1;
				break;
			default:
				printf("sqlite3_step() returned %d\n", rc);
		}
	}

	sqlite3_finalize(pstmt);
}


void FriendCmdHandle(LPSESS sess, char *text) {
	static const char *const options = "larmpd";
	char *arg, *arg2, *tmp;
	int cmdi, pos;

	arg = strchr(text, ' ');
	if (arg) {
		*arg++ = 0;
		while (*arg == ' ')
			arg++;
	}

	if (!text[1]) {
		tmp = strchr(options, text[0]);
		cmdi = tmp ? (tmp - options) : ARRAYLEN(friendcmds);
	} else {
		for (cmdi = 0; cmdi != ARRAYLEN(friendcmds); cmdi++) {
			if (!strcasecmp(text, friendcmds[cmdi]))
				break;
		}
	}

	if (cmdi == ARRAYLEN(friendcmds)) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_INVALFCMD);
		return;
	}

	switch (cmdi) {
		case FCMD_LIST:
			if (!sess->friends || !sess->friends->numelem) {
				Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOFRIENDS);
				return;
			}
			FriendListAll(sess);
			break;
		case FCMD_ADD:
			FriendAdd(sess, arg);
			break;
		case FCMD_REMOVE:
			FriendRemove(sess, arg);
			break;
		case FCMD_MSG:
			if (!sess->friends || !sess->friends->numelem) {
				Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOFRIENDS);
				return;
			}
			FriendMessage(sess, arg);
			break;
		case FCMD_PROMOTE:
		case FCMD_DEMOTE:
			arg2 = strchr(arg, ' ');
			if (arg2) {
				*arg2++ = 0;
				while (*arg2 == ' ')
					arg++;
				pos = atoi(arg2);
			} else {
				pos = 0;
			}
			FriendReposition(sess, arg, pos, cmdi == FCMD_PROMOTE);
	}
}


void FriendListAll(LPSESS sess) {
	LPFRIEND pfriend;
	LPSESS user;
	char buf[128];
	int i;

	Send0x0F(sess, EID_INFO, 0, "", "Your friends are:");
	for (i = 0; i != sess->friends->numelem; i++) {
		pfriend = sess->friends->elem[i];
		user = HtGetItem(pfriend->name, users, TL_USERS);
		if (user) {
			sprintf(buf, "%d: %s,%s using %s in %s.", i + 1, pfriend->name,
				pfriend->mutual ? " (mutual)" : "", prodname[user->clientindex],
				ChannelGetName(user->channel, pfriend->mutual));
		} else {
			sprintf(buf, "%d: %s, offline", i + 1, pfriend->name);
		}
		Send0x0F(sess, EID_INFO, 0, "", buf);
	}
}


void FriendNotify(LPSESS sess, const char *fmt, ...) {
	LPFRIEND pfriend;
	LPSESS user;
	va_list vl;
	char asdf[256];
	int i;
	
	if (!sess->friends || !sess->friends->numelem)
		return;

	va_start(vl, fmt);
	vsnprintf(asdf, sizeof(asdf), fmt, vl);
	va_end(vl);
	asdf[sizeof(asdf) - 1] = 0;

	for (i = 0; i != sess->friends->numelem; i++) {
		pfriend = sess->friends->elem[i];
		if (pfriend->mutual) {
			user = HtGetItem(pfriend->name, users, TL_USERS);
			if (user)
				Send0x0F(user, EID_WHISPER, sess->flags, sess->name, asdf);
		}
	}
}


void FriendMessage(LPSESS sess, const char *text) {
	LPFRIEND pfriend;
	LPSESS user;
	int i;

	if (!sess->friends || !sess->friends->numelem) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOFRIENDS);
		return;
	}

	Send0x0F(sess, EID_WHISPERSENT, 0, "your friends", text);
	for (i = 0; i != sess->friends->numelem; i++) {
		pfriend = sess->friends->elem[i];
		user = HtGetItem(pfriend->name, users, TL_USERS);
		if (user)
			Send0x0F(user, EID_WHISPER, sess->flags, sess->name, text);
	}
}


void FriendAdd(LPSESS sess, const char *username) {
	const char *pztail;
	sqlite3_stmt *pstmt;
	LPFRIEND pfriend;
	char buf[128];
	int rc;

	if (!strcasecmp(sess->username, username)) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_CANTADDSELF);
		return;
	}
	if (!NameIsValid(username)) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_INVALUSER);
		return;
	}

	if (sess->friends && sess->friends->numelem >= 25) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_TOOMANYFRND);
		return;
	}

	pfriend = malloc(sizeof(FRIEND));
	
	strncpy(pfriend->name, username, sizeof(pfriend->name));
	pfriend->name[sizeof(pfriend->name) - 1] = 0;
	pfriend->mutual = FriendUpdateMutual(sess, pfriend->name, 1);

	sess->friends = VectorAdd(sess->friends, pfriend);

	rc = sqlite3_prepare_v2(db_accounts, 
		"INSERT INTO friends(user, pos, mutual, friend) VALUES(?, ?, ?, ?)", -1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		printf("sqlite3_prepare_v2() failed in FriendAdd(): %s\n", sqlite3_errmsg(db_accounts));
		return;
	}
	sqlite3_bind_text(pstmt, 1, sess->username, -1, SQLITE_STATIC);
	sqlite3_bind_int(pstmt, 2, sess->friends->numelem);
	sqlite3_bind_int(pstmt, 3, pfriend->mutual);
	sqlite3_bind_text(pstmt, 4, pfriend->name, -1, SQLITE_STATIC);
	sqlite3_step(pstmt);
	sqlite3_finalize(pstmt);

	sprintf(buf, "Added %s to your friends list.", pfriend->name);
	Send0x0F(sess, EID_INFO, 0, "", buf);
}


void FriendRemove(LPSESS sess, const char *friendname) {
	LPFRIEND pfriend;
	int rc, findex;
	const char *pztail;
	sqlite3_stmt *pstmt;
	char buf[64];

	findex = FriendScan(sess, friendname);
	if (findex == -1) {
		sprintf(buf, "%s was not in your friends list.", friendname);
		Send0x0F(sess, EID_ERROR, 0, "", buf);
		return;
	}
	pfriend = sess->friends->elem[findex];
	VectorRemoveItem(sess->friends, findex);

	FriendUpdateMutual(sess, friendname, 0);

	rc = sqlite3_prepare_v2(db_accounts, 
		"DELETE FROM friends WHERE user=? AND friend=?",
		-1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		printf("sqlite3_prepare_v2() failed in FriendRemove() DELETE: %s\n", sqlite3_errmsg(db_accounts));
		return;
	}
	sqlite3_bind_text(pstmt, 1, sess->username, -1, SQLITE_STATIC);
	sqlite3_bind_text(pstmt, 2, friendname, -1, SQLITE_STATIC);
	rc = sqlite3_step(pstmt);
	sqlite3_finalize(pstmt);

	rc = sqlite3_prepare_v2(db_accounts,
		"UPDATE friends SET pos=pos-1 WHERE user=? AND pos>?",
		-1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		printf("sqlite3_prepare_v2() failed in FriendRemove() UPDATE: %s\n", sqlite3_errmsg(db_accounts));
		return;
	}
	sqlite3_bind_text(pstmt, 1, sess->username, -1, SQLITE_STATIC);
	sqlite3_bind_int(pstmt, 2, findex);
	rc = sqlite3_step(pstmt);
	sqlite3_finalize(pstmt);

	free(pfriend);
	sprintf(buf, "Removed %s from your friends list.", friendname);
	Send0x0F(sess, EID_INFO, 0, "", buf);
}


int FriendScan(LPSESS sess, const char *friendname) {
	LPFRIEND pf;
	int i;

	if (sess->friends) {
		for (i = 0; i != sess->friends->numelem; i++) {
			pf = sess->friends->elem[i];
			if (!strcasecmp(pf->name, friendname))
				return i;
		}
	}
	return -1;
}


int FriendUpdateMutual(LPSESS sess, const char *friendname, int set) {
	LPSESS user;
	LPFRIEND pfriend;
	const char *pztail;
	sqlite3_stmt *pstmt;
	int rc, pos, mutual;

	rc = sqlite3_prepare_v2(db_accounts, 
		"SELECT pos FROM friends WHERE user=? AND friend=?",
		-1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		printf("sqlite3_prepare_v2() failed in FriendUpdateMutual() SELECT: %s\n",
			sqlite3_errmsg(db_accounts));
		return 0;
	}
	sqlite3_bind_text(pstmt, 1, friendname, -1, SQLITE_STATIC);
	sqlite3_bind_text(pstmt, 2, sess->username, -1, SQLITE_STATIC);
	rc = sqlite3_step(pstmt);
	mutual = (rc == SQLITE_ROW);
	if (mutual)
		pos = sqlite3_column_int(pstmt, 0);
	sqlite3_finalize(pstmt);

	if (mutual) {
		rc = sqlite3_prepare_v2(db_accounts,
			"UPDATE friends SET mutual=? WHERE user=? AND friend=?",
			-1, &pstmt, &pztail);
		if (rc != SQLITE_OK) {
			printf("sqlite3_prepare_v2() failed in FriendUpdateMutual() UPDATE: %s\n",
				sqlite3_errmsg(db_accounts));
			return 0;
		}
		sqlite3_bind_int(pstmt, 1, set);
		sqlite3_bind_text(pstmt, 2, friendname, -1, SQLITE_STATIC);
		sqlite3_bind_text(pstmt, 3, sess->username, -1, SQLITE_STATIC);
		rc = sqlite3_step(pstmt);
		if (rc != SQLITE_DONE)
			printf("erm...\n");
		sqlite3_finalize(pstmt);

		user = HtGetItem(friendname, users, TL_USERS);
		if (user) {
			if (!user->friends) {
				printf("WARNING: active user record inconsistent with DB!\n");
				return 0;
			}
			pfriend = user->friends->elem[pos - 1];
			pfriend->mutual = set;
		}
	}

	return mutual;
}


void FriendReposition(LPSESS sess, const char *friendname, int newpos, int promo) {
	const char *pztail;
	sqlite3_stmt *pstmt;
	LPFRIEND pfriend;
	int i, rc, len, findex;
	int onepos, nfriends;
	char buf[256], asdf[32], *tmp;

	if (newpos > 25 || newpos < 0) {
		sprintf(buf, "Can't %smote to that position.", promo ? "pro" : "de");
		Send0x0F(sess, EID_ERROR, 0, "", buf);
		return;	
	}
	findex = FriendScan(sess, friendname);
	if (findex == -1) {
		sprintf(buf, "%s was not in your friends list.", friendname);
		Send0x0F(sess, EID_ERROR, 0, "", buf);
		return;
	}
	onepos = !newpos;
	nfriends = sess->friends->numelem;
	
	newpos = newpos ? (promo ? newpos - 1 : nfriends - newpos) : (findex + (promo ? -1 : 1));

	if (newpos >= nfriends)
		newpos = nfriends - 1;
	else if (newpos < 0)
		newpos = 0;

	pfriend = sess->friends->elem[findex];

	if (findex != newpos) { 
		if (newpos < findex) {
			for (i = findex; i != newpos; i--)
				sess->friends->elem[i] = sess->friends->elem[i - 1];
			len = sprintf(buf, 
				"UPDATE friends SET pos=pos+1 WHERE user=? AND pos>=%d AND pos<%d",
				newpos + 1, findex + 1);
		} else {
			for (i = findex; i != newpos; i++)
				sess->friends->elem[i] = sess->friends->elem[i + 1];
			len = sprintf(buf,
				"UPDATE friends SET pos=pos-1 WHERE user=? AND pos>%d AND pos<=%d",
				findex + 1, newpos + 1);
		}
		sess->friends->elem[newpos] = pfriend;
		
		rc = sqlite3_prepare_v2(db_accounts, buf, len, &pstmt, &pztail);
		if (rc != SQLITE_OK) {
			printf("sqlite3_prepare_v2() failed in FriendReposition() UPDATE1: %s\n", sqlite3_errmsg(db_accounts));
			return;
		}
		sqlite3_bind_text(pstmt, 1, sess->username, -1, SQLITE_STATIC);
		sqlite3_step(pstmt);
		sqlite3_finalize(pstmt);

		rc = sqlite3_prepare_v2(db_accounts, "UPDATE friends SET pos=? WHERE user=? AND friend=?", -1, &pstmt, &pztail);
		if (rc != SQLITE_OK) {
			printf("sqlite3_prepare_v2() failed in FriendReposition() UPDATE2: %s\n", sqlite3_errmsg(db_accounts));
			return;
		}
		sqlite3_bind_int(pstmt, 1, newpos + 1);
		sqlite3_bind_text(pstmt, 2, sess->username, -1, SQLITE_STATIC);
		sqlite3_bind_text(pstmt, 3, friendname, -1, SQLITE_STATIC);
		sqlite3_step(pstmt);
		sqlite3_finalize(pstmt);
	}
	
	newpos = promo ? newpos + 1 : nfriends - newpos;
	if (onepos) {
		sprintf(buf, "%smoted %s %s one position in your friends list.",
			promo ? "Pro" : "De", friendname, promo ? "up" : "down");
	} else {
		if (newpos == 1) {
			tmp = "the";
		} else {
			sprintf(asdf, "position %d from the", newpos);
			tmp = asdf;
		}
		sprintf(buf, "%smoted %s to %s %s of your friends list.", 
			promo ? "Pro" : "De", friendname, tmp, promo ? "top" : "bottom");
	}
	Send0x0F(sess, EID_INFO, 0, "", buf);
}
