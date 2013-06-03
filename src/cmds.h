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

#ifndef CMDS_HEADER
#define CMDS_HEADER

#define CMD_SVER        0
#define CMD_SVERSION    1
#define CMD_LISTCHANNELS 2
#define CMD_QMARK	    3
#define CMD_HELP        4
#define CMD_AWAY        5
#define CMD_DND         6
#define CMD_CHANNEL     7
#define CMD_JOIN        8
#define CMD_J			9
#define CMD_REJOIN      10
#define CMD_RESIGN      11
#define CMD_EMOTE       12
#define CMD_ME          13
#define CMD_SQUELCH     14
#define CMD_IGNORE      15
#define CMD_UNSQUELCH   16
#define CMD_UNIGNORE    17
#define CMD_WHISPER     18
#define CMD_W			19
#define CMD_M			20
#define CMD_MSG         21
#define CMD_WHO         22
#define CMD_WHOAMI      23
#define CMD_WHOIS       24
#define CMD_WHERE       25
#define CMD_WHEREIS     26
#define CMD_FRIENDS     27
#define CMD_F			28
#define CMD_OPTIONS     29
#define CMD_O			30
#define CMD_BAN         31
#define CMD_UNBAN       32
#define CMD_KICK		33
#define CMD_DESIGNATE   34
#define CMD_BEEP        35
#define CMD_NOBEEP      36
#define CMD_STATS       37
#define CMD_ASTAT       38
#define CMD_USERS       39
#define CMD_TIME        40
#define CMD_MAIL        41
#define CMD_CLAN        42
#define CMD_C			43

////////////////////////////////Response strings///////////////////////////////
#define RESPONSE_INVALIDCMD  "That is not a valid command. Type /help or /? for more info."
#define RESPONSE_SVER	 	 "chisai-bnetd v1.0"
#define RESPONSE_NOFRIENDS   "You don't have any friends in your list. "\
	" Use /friends add USERNAME to add a friend to your list."
#define RESPONSE_WHICHCHAN   "Which channel do you want to list?"
#define RESPONSE_NOCHANNEL   "That channel does not exist.\0"\
    "(If you are trying to search for a user, use the /whois command.)\0"
#define RESPONSE_NOTSUPPORT  "Your client software does not support this feature."
#define RESPONSE_NOTLOGON    "That user is not logged on."
#define RESPONSE_TOOMANYREQ  "Too many server requests"
#define RESPONSE_NOTOP		 "You are not a channel operator."
#define RESPONSE_INVALUSER   "Invalid user."
#define RESPONSE_NOOPKICK    "You can't kick a channel operator."
#define RESPONSE_NOOPBAN	 "You can't ban a channel operator."
#define RESPONSE_NOTBANNED   "That user is not banned."
#define RESPONSE_NOPERMIS    "You do not have permission to view that channel."
#define RESPONSE_INVALCHAN	 "Invalid channel name."
#define RESPONSE_CHANRESTR   "That channel is restricted."
#define RESPONSE_BANNED      "You are banned from that channel."
#define RESPONSE_WHATSAY	 "What do you want to say?"
#define RESPONSE_NOCHATPRIV  "This channel does not have chat privileges."
#define RESPONSE_INVALFCMD   "That is not a valid friends command. Type /help friends for more info."
#define RESPONSE_CANTADDSELF "You can't add yourself to your friends list."
#define RESPONSE_TOOMANYFRND "You already have the maximum number of friends in your list.  You will need to remove some of your friends before adding more."
///////////////////////////////////////////////////////////////////////////////


void ParseInternalCommand(char *buf);
void ParseUserCommand(LPSESS sess, char *text);

void CmdTreeInit();
void CmdTreeInsert(const char *key, int data);
int __fastcall CmdGetIndex(unsigned int cmd);
void DSWBalanceTree(LPNODE lpnode);
int RotateNodeLeft(LPNODE lpnode);
int RotateNodeRight(LPNODE lpnode);

#endif //CMDS_HEADER
