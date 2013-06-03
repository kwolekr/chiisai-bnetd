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

#ifndef USER_HEADER
#define USER_HEADER

///////////[U]ser [F]lags (visible externally)
#define UF_REP		0x0001
#define UF_OP		0x0002
#define UF_SPEAKER	0x0004
#define UF_ADMIN	0x0008
#define UF_NOUDP	0x0010
#define UF_SQUELCH  0x0020
#define UF_GUEST	0x0040
#define UF_BEEP		0x0100
#define UF_PGLPLR	0x0200
#define UF_PGLOFF	0x0400
#define UF_KBKPLR	0x0800
#define UF_WGCOFF	0x1000
#define UF_KBKSINGL 0x2000
#define UF_KBKPLR2	0x8000
#define UF_KBKBEGIN 0x10000
#define UF_KBKWHITE 0x20000
#define UF_GAMEROOM 0x40000
#define UF_GFOFF	0x100000
#define UF_GFPLR	0x200000
#define UF_PGLPLR2	0x2000000

///////////Internal [U]ser [S]tate flags
///per-session flags
#define US_CLILEGACY  0x1   ///client class
#define US_CLISC	  0x2   ///
#define US_CLIWC3	  0x4   ///
#define US_CLID2	  0x8   ///
#define US_BNCS		  0x10  ///connection type - might be unnecessary, mirrors sess->sesstype
#define US_BNFTP	  0x20  ///
#define US_TELNET	  0x40  ///
#define US_SYNC	      0x80  ///
#define US_ADMIN	  0x100 ///
#define US_REP		  0x200 ///
#define US_AWAY		  0x400 ///chat modes
#define US_DND		  0x800 ///

//permanent flags
#define US_MUTED	  0x1000
#define US_VOIDED     0x2000
#define US_PERMAVOID  0x4000
#define US_ACCCLOSED  0x8000
//					  0x10000 nibble reserved for
//					  0x20000 account closure reason.
//					  0x40000
//					  0x80000
#define US_IGW		  0x100000  ///options
#define US_IGPUB	  0x200000  ///
#define US_IGPRIV	  0x400000  ///
#define US_FRIENDMAIL 0x800000  ///
#define US_CLANMAIL	  0x1000000 ///
#define US_PERMANENT  0x1FFF000

#define US_NEEDEMAIL  0x2000000

//connection specific flags
#define US_FIRSTSEND 0x40000000
#define US_ENDSESS	 0x80000000

#define CI_STAR 0
#define CI_SEXP 1
#define CI_SSHR 2
#define CI_JSTR 3
#define CI_DSHR 4
#define CI_DRTL 5
#define CI_D2DV 6
#define CI_D2XP 7
#define CI_W2BN 8
#define CI_WAR3 9
#define CI_W3XP 10
#define CI_CHAT 11


extern int nusers_prod[12];
extern int ngames_prod[12];
extern int totalgames;
extern const char *prodname[12];

void UserLogon(LPSESS sess);
void UserLogoff(LPSESS sess);

void UserIPBan(LPSESS sess, int duration);

void HandleTimeCmd(LPSESS sess);
void HandleUsersCmd(LPSESS sess);
void HandleWhoisCmd(LPSESS sess, const char *arg);
void HandleWhoamiCmd(LPSESS sess);

void WhisperUser(LPSESS sess, LPSESS userto, const char *text);
void WhisperUsername(LPSESS sess, const char *userto, const char *text);

void HandleAwayCmd(LPSESS sess, const char *arg);
void HandleDndCmd(LPSESS sess, const char *arg);
void HandleStatsCmd(LPSESS sess, const char *user);
void HandleListChanCmd(LPSESS sess);
void HandleOptionsCmd(LPSESS sess, const char *arg);

void UserGenerateStatstring(char *dest, LPSESS sess);

#endif //USER_HEADER
