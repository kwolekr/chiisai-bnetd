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

#ifndef LOGON_HEADER
#define LOGON_HEADER

#define ACC_LOGON_OK	   0
#define ACC_LOGON_NOTEXIST 1
#define ACC_LOGON_INVALPW  2

#define ACC_CREATE_OK		  0
#define ACC_CREATE_ERROR	  1
#define ACC_CREATE_INVALCHAR  2
#define ACC_CREATE_BANNEDWORD 3
#define ACC_CREATE_EXISTS	  4
#define ACC_CREATE_TOOLONG	  5
#define ACC_CREATE_TOOSHORT	  6

#define SQL_ACC_CREATE "INSERT INTO accounts(id, username, password, ip, email, wins, losses, draws, lwins,"\
		" llosses, ldraws, lrating, status, account_created, time_logged, last_logon, sex, location, description)"\
		" VALUES(?, ?, ?, ?, '', 0, 0, 0, 0, 0, 0, 0, 0, ?, 0, 0, '', '', '')"

#define SQL_ACC_LOGON "SELECT password, id, status, wins, losses, draws, lwins, llosses, ldraws, lrating"\
		" FROM accounts WHERE username LIKE ?"

const char *bannedwords[];
const int nbannedwords;

int LogonAccount(LPSESS sess, char *pwhash, uint32_t ctoken, uint32_t stoken);
int LogonIsPasswordInvalid(const void *pwhash, const void *claimhash, uint32_t ctoken, uint32_t stoken);
int AccountCheckName(const char *name);
int AccountCreate(const char *name, const void *passhash, int ip);

#endif //LOGON_HEADER
