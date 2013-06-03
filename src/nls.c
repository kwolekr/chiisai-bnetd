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
#include "fxns.h"
#include "name.h"
#include "user.h"
#include "logon.h"
#include "hashing.h"
#include "srp.h"
#include "nls.h"


///////////////////////////////////////////////////////////////////////////////


int NLSAccountCheckName(const char *name) {
	char buf[16];
	int i, rc, ancount, pcount, apcount;
	sqlite3_stmt *pstmt;
	const char *pztail;

	i       = 0;
	ancount = 0;
	apcount = 0;
	pcount  = 0;
	while (name[i]) {
		if (NameNLSInvalidChar((unsigned char)name[i]))
			return NLSACC_CREATE_INVALCHAR;

		if (isalnum((unsigned char)name[i])) {
			ancount++;
			apcount = 0;
		} else {
			if (!i)
				return NLSACC_CREATE_INVALCHAR;
			pcount++;
			apcount++;
		}

		buf[i] = tolower((unsigned char)name[i]);
		i++;
		if (i == MAX_USERNAME_LEN - 1)
			return NLSACC_CREATE_TOOSHORT;
	}
	buf[i] = 0;

	if (ancount < MIN_NLSUSERNAME_ALPHANUMCHARS)
		return NLSACC_CREATE_NEALPHANUM;

	if (apcount > 1)
		return NLSACC_CREATE_ADJPUNCT;

	if (pcount > MAX_NLSUSERNAME_PUNCTCHARS)
		return NLSACC_CREATE_TOOMUCHPUNCT;
	
	if (i < MIN_NLSUSERNAME_LEN)
		return NLSACC_CREATE_TOOSHORT;

	for (i = 0; i != nbannedwords; i++) {
		if (strstr(buf, bannedwords[i]))
			return NLSACC_CREATE_BANNEDWORD;
	}

	rc = sqlite3_prepare_v2(db_accounts, "SELECT id FROM nlsaccounts WHERE username LIKE ?", -1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		PSQLERR("sqlite3_prepare_v2");
		sqlite3_finalize(pstmt);
		return NLSACC_CREATE_EXISTS;
	}
	sqlite3_bind_text(pstmt, 1, name, -1, SQLITE_STATIC);
	rc = sqlite3_step(pstmt);
	sqlite3_finalize(pstmt);
	return (rc != SQLITE_ROW) ? NLSACC_CREATE_OK : NLSACC_CREATE_EXISTS;
}


int NLSAccountCreate(char *name, unsigned char *salt, unsigned char *verifier, int ip) {
	sqlite3_stmt *pstmt;
	const char *pztail;
	int id, rc;

	sqlite3_prepare_v2(db_accounts, "SELECT MAX(id) FROM nlsaccounts", -1, &pstmt, &pztail);
	rc = sqlite3_step(pstmt);
	if (rc != SQLITE_ROW) {
		PSQLERR("sqlite3_step");
		sqlite3_finalize(pstmt);
		return ACC_CREATE_ERROR;
	}
	id = sqlite3_column_int(pstmt, 0) + 1;
	sqlite3_finalize(pstmt);

	rc = sqlite3_prepare_v2(db_accounts, SQL_NLSACC_CREATE, -1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		PSQLERR("sqlite3_prepare_v2");
		return ACC_CREATE_ERROR;
	}

	sqlite3_bind_int(pstmt, 1, id);
	sqlite3_bind_text(pstmt, 2, name, -1, SQLITE_STATIC);
	sqlite3_bind_blob(pstmt, 3, salt, 32, SQLITE_STATIC);
	sqlite3_bind_blob(pstmt, 4, verifier, 32, SQLITE_STATIC);
	sqlite3_bind_int(pstmt, 5, ip);
	sqlite3_bind_int(pstmt, 6, (int)time(NULL));

	rc = sqlite3_step(pstmt);
	if (rc != SQLITE_DONE)
		PSQLERR("sqlite3_step");

	sqlite3_finalize(pstmt);
	return (rc == SQLITE_DONE) ? ACC_CREATE_OK : ACC_CREATE_ERROR;
}


int NLSAccountLogon(LPSESS sess, unsigned char *A, unsigned char **salt, unsigned char **serverkey) {
	sqlite3_stmt *pstmt;
	const char *pztail;
	const unsigned char *accsalt, *verifier, *email;
	char username[MAX_USERNAME_LEN];
	int rc;
	LPNLS nls;

	if (!sess || !A || !salt || !serverkey)
		return NLSACC_LOGON_FAILURE;

	if (sess->nls) {
		printf("WARNING: NLSAccountLogon called with existing NLS context, clearing\n");
		SRPFree(sess->nls);
		sess->nls = NULL;
	}

	sqlite3_prepare_v2(db_accounts, SQL_NLSACC_LOGON, -1, &pstmt, &pztail);
	sqlite3_bind_text(pstmt, 1, sess->username, -1, SQLITE_STATIC);
	rc = sqlite3_step(pstmt);
	if (rc != SQLITE_ROW) {
		if (rc != SQLITE_DONE)
			PSQLERR("sqlite3_step");
		sqlite3_finalize(pstmt);
		return NLSACC_LOGON_NOTEXIST;
	}
	
	accsalt			   = sqlite3_column_blob(pstmt, 0);
	verifier		   = sqlite3_column_blob(pstmt, 1);
	sess->accid		   = sqlite3_column_int(pstmt, 2);
	email			   = sqlite3_column_text(pstmt, 3);
	sess->state       |= sqlite3_column_int(pstmt, 4);
	sess->stats.wins   = sqlite3_column_int(pstmt, 5);
	sess->stats.losses = sqlite3_column_int(pstmt, 6);
	sess->stats.draws  = sqlite3_column_int(pstmt, 7);
	if (!accsalt || !verifier || !email) {
		sqlite3_finalize(pstmt);
		return NLSACC_LOGON_FAILURE;
	}

	if (!email[0])
		sess->state |= US_NEEDEMAIL;

	nls = malloc(sizeof(NLS));
	memset(nls, 0, sizeof(NLS));

	ucasecpy(username, sess->username);
	SHA1(username, strlen(username), nls->namehash);	
	SRPCalcB(nls, verifier);
	memcpy(nls->A, A, 32);
	memcpy(nls->salt, accsalt, 32);
	*salt      = nls->salt;
	*serverkey = nls->B;
	sess->nls  = nls;

	sqlite3_finalize(pstmt);
	return NLSACC_LOGON_OK;
}


int NLSAccountAuthenticate(LPSESS sess, unsigned char *M1, unsigned char *M2) {

	if (!M1 || !M2)
		return NLSACC_LOGON_FAILURE;

	if (!sess->nls) {
		printf("WARNING: NLSAccountAuthenticate called with sess->nls == NULL!\n");
		return NLSACC_LOGON_FAILURE;
	}

	SRPCalcM1(sess->nls);
	if (memcmp(M1, sess->nls->M1, sizeof(sess->nls->M1))) {
		memset(M2, 0, 20);
		return NLSACC_LOGON_WRONGPW;
	}
	SRPCalcM2(sess->nls, M2);
	
	if (sess->state & US_ACCCLOSED)
		return NLSACC_LOGON_WRONGPW;

	if (sess->state & US_NEEDEMAIL)
		return NLSACC_LOGON_REQEMAIL;

	return NLSACC_LOGON_OK;
}
