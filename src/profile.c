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
#include "user.h"
#include "pbuffer.h"
#include "profile.h"


const char *profile_keys[] = {
	"System",
	"DynKey",
	"profile",
	"record"
};

PVPAIR profile_system_values[] = {
	{"Account Created", "account_created"},
	{"Last Logon", "time_logged"},
	{"Last Logoff", NULL},
	{"Time Logged", "last_logon"},
	{NULL, NULL}
};

PVPAIR profile_profile_values[] = {
	{"sex", "sex"},
	{"age", NULL},
	{"location", "location"},
	{"description", "description"},
	{NULL, NULL}
};

PVPAIR profile_record_values[] = {
	{"GAME\\0\\wins", "wins"},
	{"GAME\\0\\losses", "losses"},
	{"GAME\\0\\disconnects", "draws"},
	{"GAME\\0\\last GAME", NULL},
	{"GAME\\0\\last GAME result", NULL},
	{NULL, NULL}
};

LPPVPAIR profile_key_values[] = {
	profile_system_values,
	profile_record_values,
	profile_profile_values,
	profile_record_values
};


///////////////////////////////////////////////////////////////////////////////


void Parse0x26(char *data, LPSESS sess) {
	unsigned int naccts, nkeys, i, len;
	char *cacct, **accts, *ckey, **keys, *sbufcur;
	uint32_t reqid;
	char query[512];

	naccts = *(uint32_t *)(data + 4);
	nkeys  = *(uint32_t *)(data + 8);
	reqid  = *(uint32_t *)(data + 12);
	
	if (naccts != 1 && !(sess->flags & 0x09))
		return;

	if (!naccts || naccts > 32) {
		UserIPBan(sess, IPBLEN_INVALID_FIELD_SIZE);
		return;
	}

	accts = alloca(naccts * sizeof(char *));
	keys  = alloca(nkeys  * sizeof(char *));

	cacct = data + 16;
	for (i = 0; i != naccts; i++) {
		accts[i] = cacct;
		len = strlen(cacct);
		if (len >= MAX_NAME_LEN) {
			UserIPBan(sess, IPBLEN_INVALID_FIELD_SIZE);
			return;
		}
		cacct += len + 1;
	}
	
	ckey = cacct;
	for (i = 0; i != nkeys; i++) {
		keys[i] = ckey;
		len = strlen(ckey);
		if (len >= MAX_PROFILEKEY_LEN) {
			UserIPBan(sess, IPBLEN_INVALID_FIELD_SIZE);
			return;
		}
		ckey += len + 1;
	}
	
	InsertDWORD(naccts, sess);
	InsertDWORD(nkeys, sess);
	InsertDWORD(reqid, sess);

	if (ProfileBuildQueryStr(keys, nkeys, query, sizeof(query), 0)) {
		for (i = 0; i != naccts; i++) {
			sbufcur = sess->sendbuf + sess->sbufpos;
			len = ProfileRead(*accts[i] ? accts[i] : sess->username, query,
				      nkeys, sbufcur, sizeof(sess->sendbuf) - sess->sbufpos);
			sess->sbufpos += len;
		}
	} else {
		len = nkeys * naccts;
		memset(sess->sendbuf + sess->sbufpos, 0, len);
		sess->sbufpos += len;
	}
	SendPacket(0x26, sess);
}


void Parse0x27(char *data, LPSESS sess) {
	unsigned int naccts, nkeys, i, len;
	char *cacct, **accts, *ckey, **keys, *cnval, **nvals;
	char query[512];

	naccts = *(uint32_t *)(data + 4);
	nkeys  = *(uint32_t *)(data + 8);
	
	if (naccts != 1 && !(sess->flags & 0x09))
		return;
	
	if (!naccts || naccts > 32) {
		UserIPBan(sess, IPBLEN_INVALID_FIELD_SIZE);
		return;
	}

	accts = alloca(naccts * sizeof(char *));
	keys  = alloca(nkeys  * sizeof(char *));
	nvals = alloca(nkeys  * sizeof(char *));

	cacct = data + 12;
	for (i = 0; i != naccts; i++) {
		accts[i] = cacct;
		len = strlen(cacct);
		if (len >= MAX_NAME_LEN) {
			UserIPBan(sess, IPBLEN_INVALID_FIELD_SIZE);
			return;
		}
		cacct += len + 1;
	}
	
	ckey = cacct;
	for (i = 0; i != nkeys; i++) {
		keys[i] = ckey;
		len = strlen(ckey);
		if (len >= MAX_PROFILEKEY_LEN) {
			UserIPBan(sess, IPBLEN_INVALID_FIELD_SIZE);
			return;
		}
		ckey += len + 1;
	}

	cnval = ckey;
	for (i = 0; i != nkeys; i++) {
		nvals[i] = cnval;
		len = strlen(cnval);
		if (len >= MAX_PROFILEENT_LEN) {
			UserIPBan(sess, IPBLEN_INVALID_FIELD_SIZE);
			return;
		}
		cnval += len + 1;
	}

	if (!ProfileBuildQueryStr(keys, nkeys, query, sizeof(query), 1))
		return;

	for (i = 0; i != naccts; i++) {
		if (!*accts[i]) {
			cacct = sess->username;
		} else {
			if (!(sess->flags & 0x9))
				return;
		}
		ProfileWrite(cacct, query, nvals, nkeys);
	}
}


int ProfileRead(const char *account, const char *query, int nkeys, char *buf, unsigned int buflen) {
	const char *pztail;
	sqlite3_stmt *pstmt;
	int i, rc, len, written;
	char *cur, *tmp;
	const char *str;

	if (!nkeys)
		return 0;

	rc = sqlite3_prepare_v2(db_accounts, query, -1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		printf("sqlite3_prepare_v2() failed in ProfileRead() SELECT: %s\n", sqlite3_errmsg(db_accounts));
		return 0;
	}
	
	sqlite3_bind_text(pstmt, 1, account, -1, SQLITE_STATIC);
	if ((rc = sqlite3_step(pstmt)) != SQLITE_ROW) {
		printf("sqlite3_step() returned %d, %s\n", rc, sqlite3_errmsg(db_accounts));
		sqlite3_finalize(pstmt);
		return 0;
	}

	written = 0;
	cur = buf;
	for (i = 0; i != nkeys; i++) {
		if (!buflen) {
			sqlite3_finalize(pstmt);
			return 0;
		}
		str = sqlite3_column_text(pstmt, i);
		if (!str) {
			printf("err: ProfileRead!sqlite3_column_text() == NULL!\n");
			sqlite3_finalize(pstmt);
			return 0;
		}
		tmp = stpncpy(cur, str, buflen) + 1;
		len = tmp - cur;
		written += len;
		buflen  -= len;
		cur = tmp;
	}
	sqlite3_finalize(pstmt);

	return written;
}


void ProfileWrite(const char *account, const char *query, char **newvals, int nkeys) {
	const char *pztail;
	sqlite3_stmt *pstmt;
	int i, rc;

	rc = sqlite3_prepare_v2(db_accounts, query, -1, &pstmt, &pztail);
	if (rc != SQLITE_OK) {
		printf("sqlite3_prepare_v2() failed in ProfileWrite() UPDATE: %s\n", sqlite3_errmsg(db_accounts));
		return;
	}
	
	for (i = 0; i != nkeys; i++)
		sqlite3_bind_text(pstmt, i + 1, newvals[i], -1, SQLITE_STATIC);

	sqlite3_bind_text(pstmt, nkeys + 1, account, -1, SQLITE_STATIC);

	if ((rc = sqlite3_step(pstmt)) != SQLITE_DONE)
		printf("sqlite3_step() returned %d, %s\n", rc, sqlite3_errmsg(db_accounts));

	sqlite3_finalize(pstmt);	
}


int ProfileBuildQueryStr(char **keys, int nkeys, char *query, unsigned int buflen, int update) {
	const char *col;
	char *current;
	int i, qlen, len;

	if (update) {
		strcpy(query, "UPDATE accounts SET ");
		current = query + (sizeof("UPDATE accounts SET ") - 1);
	} else {
		strcpy(query, "SELECT ");
		current = query + (sizeof("SELECT ") - 1);
	}

	qlen = buflen - (current - query);

	for (i = 0; i != nkeys; i++) {
		col = ProfileGetColumnNameFromKey(keys[i]);
		if (!col)
			return 0;

		len = strlen(col);

		if (len + 3 >= qlen) {
			printf("WARNING: insufficient buffer size in ProfileBuildQueryStr\n");
			return 0;
		}
		strcpy(current, col);
		current += len;
		qlen    -= len;
		if (update) {
			*(int32_t *)current = ',?=';
			current += 3;
			qlen    -= 3;
		} else {
			*(int16_t *)current = ',';
			current++;
			qlen--;
		}
	}

	strncpy(current - 1, update ? " WHERE username=?" : " FROM accounts WHERE username=?", qlen);
	query[buflen - 1] = 0;
	//puts(query);//////////DEBUG
	return 1;
}


const char *ProfileGetColumnNameFromKey(char *keyname) {
	LPPVPAIR keyvalues;
	char *valuename;
	int i;

	valuename = strchr(keyname, '\\');
	if (valuename)
		*valuename++ = 0;
	else
		return NULL;

	for (i = 0; i != ARRAYLEN(profile_keys); i++) {
		if (!strcmp(keyname, profile_keys[i]))
			break;
	}
	if (i == ARRAYLEN(profile_keys))
		return NULL;

	valuename[-1] = '\\';

	keyvalues = profile_key_values[i];

	for (i = 0; keyvalues[i].valuename; i++) {
		if (!strcmp(valuename, keyvalues[i].valuename))
			break;
	}
	if (!keyvalues[i].valuename)
		return NULL;
	
	if (!keyvalues[i].column)
		printf("WARNING: client requested unimplemented profile entry!\n");

	return keyvalues[i].column;
}
