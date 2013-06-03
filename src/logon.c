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
 * logon.c - 
 *    Functions for account creation, logon, and maintenance.
 */

#include "main.h"
#include "fxns.h"
#include "conn.h"
#include "user.h"
#include "name.h"
#include "packets.h"
#include "hashing.h"
#include "logon.h"

const char *bannedwords[] = {
	"fuck",
	"shit",
	"piss",
	"cunt",
	"bitch",
	"bastard",
	"blizzard",
	REALM_NAME_LWR_SC,
	REALM_NAME_LWR_WC
};

const int nbannedwords = sizeof(bannedwords) / sizeof(bannedwords[0]);


///////////////////////////////////////////////////////////////////////////////


int AccountCheckName(const char *name) {
	char buf[16];
	int i, rc;
	sqlite3_stmt *pstmt;
	const char *pztail;

	i = 0;
	while (name[i]) {
		if (NameInvalidChar((unsigned char)name[i]))
			return ACC_CREATE_INVALCHAR;
		buf[i] = tolower((unsigned char)name[i]);
		i++;
		if (i == MAX_USERNAME_LEN - 1)
			return ACC_CREATE_TOOLONG;
	}
	buf[i] = 0;
	
	if (i < MIN_USERNAME_LEN)
		return ACC_CREATE_TOOSHORT;

	for (i = 0; i != ARRAYLEN(bannedwords); i++) {
		if (strstr(buf, bannedwords[i]))
			return ACC_CREATE_BANNEDWORD;
	}

	rc = sqlite3_prepare_v2(db_accounts, "SELECT id FROM accounts WHERE username LIKE ?", -1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		PSQLERR("sqlite3_prepare_v2");
		sqlite3_finalize(pstmt);
		return ACC_CREATE_EXISTS;
	}
	sqlite3_bind_text(pstmt, 1, name, -1, SQLITE_STATIC);
	rc = sqlite3_step(pstmt);
	sqlite3_finalize(pstmt);
	return (rc != SQLITE_ROW) ? ACC_CREATE_OK : ACC_CREATE_EXISTS;
}


int AccountCreate(const char *name, const void *passhash, int ip) {
	sqlite3_stmt *pstmt;
	const char *pztail;
	int id, rc;

	sqlite3_prepare_v2(db_accounts, "SELECT MAX(id) FROM accounts", -1, &pstmt, &pztail);
	rc = sqlite3_step(pstmt);
	if (rc != SQLITE_ROW) {
		PSQLERR("sqlite3_step");
		sqlite3_finalize(pstmt);
		return ACC_CREATE_ERROR;
	}
	id = sqlite3_column_int(pstmt, 0) + 1;

	rc = sqlite3_prepare_v2(db_accounts, SQL_ACC_CREATE, -1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		PSQLERR("sqlite3_prepare_v2");
		return ACC_CREATE_ERROR;
	}

	sqlite3_bind_int(pstmt, 1, id);
	sqlite3_bind_text(pstmt, 2, name, -1, SQLITE_STATIC);
	sqlite3_bind_blob(pstmt, 3, passhash, 20, SQLITE_STATIC);
	sqlite3_bind_int(pstmt, 4, ip);
	sqlite3_bind_int(pstmt, 5, (int)time(NULL));

	rc = sqlite3_step(pstmt);
	if (rc != SQLITE_DONE)
		PSQLERR("sqlite3_step");

	sqlite3_finalize(pstmt);
	return (rc == SQLITE_DONE) ? ACC_CREATE_OK : ACC_CREATE_ERROR;
}


int LogonAccount(LPSESS sess, char *pwhash, uint32_t ctoken, uint32_t stoken) {
	sqlite3_stmt *pstmt;
	const char *pztail;
	const void *pw;
	int id, status, result, rc;

	result = ACC_LOGON_OK;

	sqlite3_prepare_v2(db_accounts, SQL_ACC_LOGON, -1, &pstmt, &pztail);
	rc = sqlite3_step(pstmt);
	switch (rc) {
		case SQLITE_ROW:
			pw     = sqlite3_column_blob(pstmt, 0);
			id     = sqlite3_column_int(pstmt, 1);
			status = sqlite3_column_int(pstmt, 2);
			
			if (LogonIsPasswordInvalid(pw, pwhash, ctoken, stoken))
				result = ACC_LOGON_INVALPW;
			if (status & US_ACCCLOSED)
				return (status & 0xF0000) >> 16;

			sess->accid  = id;
			sess->state |= status;
			sess->stats.wins    = sqlite3_column_int(pstmt, 3);
			sess->stats.losses  = sqlite3_column_int(pstmt, 4);
			sess->stats.draws   = sqlite3_column_int(pstmt, 5);
			sess->lstats.wins   = sqlite3_column_int(pstmt, 6);
			sess->lstats.losses = sqlite3_column_int(pstmt, 7);
			sess->lstats.draws  = sqlite3_column_int(pstmt, 8);
			sess->lstats.rating = sqlite3_column_int(pstmt, 9);
			break;
		case SQLITE_DONE:
			result = ACC_LOGON_NOTEXIST;
			break;
		default:
			PSQLERR("sqlite3_step");
	}

	sqlite3_finalize(pstmt);
	return result;
}


int LogonIsPasswordInvalid(const void *pwhash, const void *claimhash, uint32_t ctoken, uint32_t stoken) {
	char tmpbuf[28], result[20];

	*(uint32_t *)tmpbuf       = ctoken;
	*(uint32_t *)(tmpbuf + 4) = stoken;
	memcpy(tmpbuf + 8, pwhash, 20);
	BSHA1(tmpbuf, 28, (uint32_t *)result);
	return memcmp(result, claimhash, 20);
}
