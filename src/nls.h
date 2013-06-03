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

#ifndef NLS_HEADER
#define NLS_HEADER

#define NLSACC_LOGON_OK	      0
#define NLSACC_LOGON_NOTEXIST 1
#define NLSACC_LOGON_WRONGPW  2
#define NLSACC_LOGON_UPGRADE  5
#define NLSACC_LOGON_REQEMAIL 14
#define NLSACC_LOGON_CUSTOM   15
#define NLSACC_LOGON_FAILURE  3

#define NLSACC_CREATE_OK		   0
#define NLSACC_CREATE_EXISTS       4
#define NLSACC_CREATE_TOOSHORT     7
#define NLSACC_CREATE_INVALCHAR    8
#define NLSACC_CREATE_BANNEDWORD   9
#define NLSACC_CREATE_NEALPHANUM   10
#define NLSACC_CREATE_ADJPUNCT     11
#define NLSACC_CREATE_TOOMUCHPUNCT 12

#define SQL_NLSACC_CREATE "INSERT INTO nlsaccounts(id, username, salt, verifier, ip, email, wins, losses, draws,"\
	    " status, account_created, time_logged, last_logon, sex, location, description)"\
	    "VALUES(?, ?, ?, ?, ?, '', 0, 0, 0, 0, ?, 0, 0, '', '', '')"

#define SQL_NLSACC_LOGON "SELECT salt, verifier, id, email, status, wins, losses, draws "\
		"FROM nlsaccounts WHERE username LIKE ?"

int NLSAccountCheckName(const char *name);
int NLSAccountCreate(char *name, unsigned char *salt, unsigned char *verifier, int ip);
int NLSAccountLogon(LPSESS sess, unsigned char *A, unsigned char **salt, unsigned char **serverkey);
int NLSAccountAuthenticate(LPSESS sess, unsigned char *M1, unsigned char *M2);

#endif //NLS_HEADER
