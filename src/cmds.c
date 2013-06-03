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
 * cmds.c - 
 *    Internal and external command processor, parser, and executor
 */

#include "main.h"
#include "fxns.h"
#include "vector.h"
#include "ht.h"
#include "user.h"
#include "chat.h"
#include "channel.h"
#include "packets.h"
#include "op.h"
#include "friends.h"
#include "vuser.h"
#include "cmds.h"


LPNODE cmdtree_internal;
LPNODE cmdtree_user;

const char *cmddesc_internal[] = {
	"ver",
	"version",
	"?",
	"help",
	"vuser"
};

const char *cmddesc_user[] = {
	"sver",
	"sversion",
	"listchannels",
	"?",
	"help",
	"away",
	"dnd",

	"channel",
	"join",
	"j",
	"rejoin",
	"resign",

	"emote",
	"me",

	"squelch",
	"ignore",
	"unsquelch",
	"unignore",

	"whisper",
	"w",
	"m",
	"msg",

	"who",
	"whoami",

	"whois",
	"where",
	"whereis",

	"friends",
	"f",

	"options",
	"o",
	
	"ban",
	"unban",
	"kick",
	"designate",

	"beep",
	"nobeep",

	"stats",
	"astat",
	"users",
	"time",
	"mail",

	"clan",
	"c"
};

///////////////////////////////////////////////////////////////////////////////


void ParseInternalCommand(char *text) {
	char *arg;
	int i;

	arg = strchr(text, ' ');
	if (arg)
		*arg++ = 0;
	for (i = 0; i != ARRAYLEN(cmddesc_internal); i++) {
		if (!strcasecmp(text, cmddesc_internal[i]))
			break;
	}
	if (i == ARRAYLEN(cmddesc_internal)) {
		printf("\'%s\' invalid command\n", text);
		return;
	}
	switch (i) {
		case 0: //ver
		case 1: //version
			break;
		case 2: //?
		case 3: //help
			break;
		case 4: //vuser
			VUserInternalCmd(arg);
			break;
		default:
			;
	}
	/*if (!strncmp(buf, "doubleleave", 11)) {
		Send0x0F(NULL, 3, 0, tehsess->realname, "PXES");
		Send0x0F(NULL, 3, 0, tehsess->realname, "PXES");
	}*/
}


void ParseUserCommand(LPSESS sess, char *text) {
	char *arg, *arg2, *tosend;
	int cmdi;

	arg = strchr(text, ' ');
	if (arg) {
		*arg++ = 0;
		while (*arg == ' ')
			arg++;
	}
	lcase(text);

	cmdi = CmdGetIndex(hash((unsigned char *)text));
	if (cmdi == -1 || strcmp(text, cmddesc_user[cmdi])) {
		Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_INVALIDCMD);
		return;
	}

	tosend = NULL;
	switch (cmdi) {
		case CMD_SVER:
		case CMD_SVERSION:
			Send0x0F(sess, EID_INFO, 0, "", RESPONSE_SVER);
			break;
		case CMD_LISTCHANNELS:
			HandleListChanCmd(sess);
			break;
		case CMD_QMARK:
		case CMD_HELP:
			break;
		case CMD_AWAY:
			HandleAwayCmd(sess, arg);
			break;
		case CMD_DND:
			HandleDndCmd(sess, arg);
			break;
		case CMD_CHANNEL:
		case CMD_JOIN:
		case CMD_J:
			ChannelJoin(arg, sess, 0);
			break;
		case CMD_REJOIN: //
		case CMD_RESIGN: //purposely unimplemented
			break;
		case CMD_EMOTE:
		case CMD_ME:
			SendTextToChannel(sess, sess->channel, sess->name, arg, EID_EMOTE);
			break;
		case CMD_SQUELCH:
		case CMD_IGNORE:
			//"%s has been squelched."
			break;
		case CMD_UNSQUELCH:
		case CMD_UNIGNORE:
			//"%s has been unsquelched."
			break;
		case CMD_WHISPER:
		case CMD_W:
		case CMD_M:
		case CMD_MSG:
			if (!arg || !*arg) {
				Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOTLOGON);
				return;
			}
			arg2 = strchr(arg, ' ');
			if (arg2) {
				*arg2++ = 0;
				while (*arg2 == ' ')
					arg2++;
			}
			WhisperUsername(sess, arg, arg2);
			break;
		case CMD_WHO:
			HandleWhoCmd(sess, arg);
			break;
		case CMD_WHOAMI:
			HandleWhoamiCmd(sess);
			break;
		case CMD_WHOIS:
		case CMD_WHERE:
		case CMD_WHEREIS:
			HandleWhoisCmd(sess, arg);
			break;
		case CMD_FRIENDS:
		case CMD_F:
			FriendCmdHandle(sess, arg);
			break;
		case CMD_OPTIONS:
		case CMD_O:
			HandleOptionsCmd(sess, arg);
			break;
		case CMD_BAN:
			BanUser(sess, arg, 1);
			break;
		case CMD_UNBAN:
			BanUser(sess, arg, 0);
			break;
		case CMD_KICK:
			KickUser(sess, arg);
			break;
		case CMD_DESIGNATE:
			DesignateUser(sess, arg);
			break;
		case CMD_BEEP:
		case CMD_NOBEEP:
			Send0x0F(sess, EID_ERROR, 0, "", RESPONSE_NOTSUPPORT);
			break;
		case CMD_STATS:
		case CMD_ASTAT:
			HandleStatsCmd(sess, arg);
			break;
		case CMD_USERS:
			HandleUsersCmd(sess);
			break;
		case CMD_TIME:
			HandleTimeCmd(sess);
			break;
		case CMD_MAIL:
			break;
		case CMD_CLAN:
		case CMD_C:
			break;
		default:
			;
	}
}


void CmdTreeInit() {
	int i;

	for (i = 0; i != ARRAYLEN(cmddesc_user); i++)
		CmdTreeInsert(cmddesc_user[i], i);

	DSWBalanceTree(cmdtree_user);
}


void CmdTreeInsert(const char *key, int data) {
	LPNODE *tmp    = &cmdtree_user;
	LPNODE lpnode  = malloc(sizeof(NODE));
	lpnode->key    = hash((unsigned char *)key);
	lpnode->data   = data;
	lpnode->lchild = NULL;
	lpnode->rchild = NULL;

	while (*tmp)
		tmp = (lpnode->key > (*tmp)->key) ? &((*tmp)->rchild) : &((*tmp)->lchild);
	*tmp = lpnode;
}


int __fastcall CmdGetIndex(unsigned int cmd) {
	LPNODE *tmp = &cmdtree_user;
	while (*tmp) {
		if (cmd > (*tmp)->key)
			tmp = &((*tmp)->rchild);
		else if (cmd < (*tmp)->key)
			tmp = &((*tmp)->lchild);
		else
			return (*tmp)->data;
	}
	return -1;
}


void DSWBalanceTree(LPNODE lpnode) {
	LPNODE p;
	int nc, i, i2;

	nc = 0;
	p  = lpnode;
	
	//tree to vine
	while (p) {	
		while (RotateNodeRight(p) == 1);
		p = p->rchild;
		nc++;
	}

	//vine to tree
	for (i = nc >> 1; i; i >>= 1) {
		p = lpnode;	
		for (i2 = 0; i2 < i; i2++) {		
			RotateNodeLeft(p);
			p = p->rchild;
		}
	}
}


int RotateNodeLeft(LPNODE lpnode) {
	LPNODE p;
	int data;
	unsigned int key;

	if (!lpnode || !lpnode->rchild)
		return 0;

	p = lpnode->rchild;
	lpnode->rchild = p->rchild;

	p->rchild = p->lchild;
	p->lchild = lpnode->lchild;
	lpnode->lchild = p;

	key  = lpnode->key;
	data = lpnode->data;
	lpnode->key  = p->key;
	lpnode->data = p->data;
	p->key  = key;
	p->data = data;
	return 1;
}


int RotateNodeRight(LPNODE lpnode) {
	LPNODE p;
	int data;
	unsigned int key;

	if (!lpnode || !lpnode->lchild)
		return 0;

	p = lpnode->lchild;
	lpnode->lchild = p->lchild;

	p->lchild = p->rchild;
	p->rchild = lpnode->rchild;
	lpnode->rchild = p;

	key  = lpnode->key;
	data = lpnode->data;
	lpnode->key  = p->key;
	lpnode->data = p->data;
	p->key  = key;
	p->data = data;
	return 1;
}
