/*-
 * Copyright (c) 2009 Ryan Kwolek 
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
 * main.c - 
 *    Lolz
 */


#include "main.h"
#include "ht.h"
#include "conn.h"
#include "fxns.h"
#include "srp.h"
#include "vector.h"
#include "channel.h"
#include "packets.h"
#include "cmds.h"

WAITOBJ maininit;
LPVECTOR users[TL_USERS];

char db_accounts_filename[256];
sqlite3 *db_accounts;

int nreqs;
int nsessions;

int nusers_bncs;
int nusers_bnftp;
int nusers_telnet;
int nusers_sync;
int nusers_admin;
int nusers_reps;


///////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[]) {
	char buf[256];
	int len, errc;
	#ifdef _WIN32
		HMODULE hLib;
		WSADATA wsadata;

		WSAStartup(0x0202, &wsadata);

		hLib = LoadLibrary("advapi32.dll");
		if (hLib)
			lpfnRtlGenRandom = (BOOLEAN (WINAPI *)(PVOID, ULONG))GetProcAddress(hLib, "SystemFunction036");
	#endif

	printf(" === chiisai-bnetd v1.0 ===\n\tinitializing...\n");
	_CreateWaitObject(&maininit);

	srand((unsigned int)time(NULL));

	LoadDefaultConfig();
	ParseCmdLine(argc, argv);
	CmdTreeInit();
	ChannelCreateDefaults();
	SRPGlobalInit();

	threadpool = VectorInit(NUM_INITIAL_POOLS);

	errc = sqlite3_open(db_accounts_filename, &db_accounts);
	if (errc != SQLITE_OK) {
		printf("sqlite3_open(%s) failed, err %d: %s\n",
			db_accounts_filename, errc, sqlite3_errmsg(db_accounts));
		return 1;
	}
	printf(" - account database opened\n");

	if (!SetupAlertSocket()) {
		printf("SetupAlertSocket() failed!\n");
		return 1;
	}
	printf(" - alert socket created\n");

	udp_sck = CreateUDPSocket(INADDR_ANY, SERVER_PORT, NULL);

	if (!SetupListenSocket(&listen_sck, &listen_tid)) {
		printf("SetupListenSocket() failed!\n");
		return 1;
	}

	_WaitForObject(&maininit, -1);
	printf("\tinitialization complete.\n");
	while (1) {
		fprintf(stdout, REALM_NAME_SC "> ");
		if (fgets(buf, sizeof(buf), stdin)) {
			len = strlen(buf);
			if (buf[len - 1] == '\n')
				buf[len - 1] = 0;
			ParseInternalCommand(buf);
		}
	}
	sqlite3_close(db_accounts);
	return 0;
}


void LoadDefaultConfig() {
	strcpy(db_accounts_filename, "accounts.db");
}


void ParseCmdLine(int argc, char *argv[]) {
	//stub;
}


void __fastcall BNCSPacketHandler(LPSESS sess) {
	unsigned int recvlen, bnetlen;
	char *temp;

	recvlen = recv(sess->sck, sess->recvbuf, sizeof(sess->recvbuf), 0);
	if (!recvlen) {
		//printf("graceful disconnection\n");
		SessionReqDisconnect(sess);
		return;
	} else if (recvlen == -1) {
		printf("recv() == %d, err() == %d\n", recvlen, err());
		if (err() != EWOULDBLOCK)
			SessionReqDisconnect(sess);
		return;
	}

	temp = sess->recvbuf;
	while (recvlen >= 4) {
		if ((unsigned char)*temp != (unsigned char)0xFF) {
			printf("warning! packet header mismatch!\n");
			break;
		}
		bnetlen = *(uint16_t *)(temp + 2);
		if (bnetlen > recvlen) {
			printf("Expecting %d more bytes...\n", bnetlen - recvlen);
			return;
		} else if (bnetlen > BNCS_PKTDROPLEN) {
			SessionReqDisconnect(sess);
		}
		#ifdef VERBOSE
			//if (temp[1])
			//	printf("[BNET] Received 0x%02x!\n", temp[1]);
			if (pkhndindex[temp[1]])
				(*pkthandlers[pkhndindex[(unsigned char)temp[1]] - 1])(temp, sess);
			else
				printf("[BNET] Unhandled packet 0x%02x!\n", (unsigned char)temp[1]);
		#else
			(*pkthandlers[pkhndindex[(unsigned char)temp[1]] - 1])(temp, sess);
		#endif
		recvlen -= bnetlen;
		temp    += bnetlen;
	}
}


void __fastcall BNFTPPacketHandler(LPSESS sess) {


}


void __fastcall TelnetPacketHandler(LPSESS sess) {


}
