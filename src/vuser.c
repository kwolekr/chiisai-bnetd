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

/*
 * vuser.c -
 *    Routines for maintaining virtual users on the server controlled
 *    locally or remotely for testing purposes
 */

#include "main.h"
#include "vector.h"
#include "ht.h"
#include "conn.h"
#include "user.h"
#include "name.h"
#include "channel.h"
#include "vuser.h"

const char *vusercmds[] = {
	"create",
	"destroy",
	"massjoin",
	"loadchan",
	"floodchan",
	"stop"
};

LPVECTOR vugroups;


///////////////////////////////////////////////////////////////////////////////


void VUserInternalCmd(char *text) {
	LPVUGROUP vugroup;
	//LPSESS *vusers;
	int nvusers, i;
	char *arg, *arg2;

	if (!text) {
		if (!vugroups) {
			printf("No vuser groups active\n");
			return;
		}
		printf("%d VUser groups active:\n identifier\t# users", vugroups->numelem);
		for (i = 0; i != vugroups->numelem; i++) {
			vugroup = vugroups->elem[i];
			printf(" %s\t%d\n", vugroup->vupattern, vugroup->nvusers);
		}
		return;
	}

	arg = strchr(text, ' ');
	if (arg)
		*arg++ = 0;
	
	//format:
	//vuser create username 500
	//vuser create random

	for (i = 0; i != ARRAYLEN(vusercmds); i++) {
		if (!strcasecmp(text, vusercmds[i]))
			break;
	}
	if (i == ARRAYLEN(vusercmds)) {
		printf("ERROR: invalid virtual user subcommand \'%s\'", text);
		return;
	}
	switch (i) {
		case VUCMD_CREATE:
			if (!arg) {
				printf("ERROR: invalid number of parameters\n");
				return;
			}
			arg2 = strchr(arg, ' ');
			if (arg2) {
				*arg2++ = 0;
				nvusers = atoi(arg2);
			} else {
				nvusers = 1;
			}
			vugroup			   = malloc(sizeof(VUGROUP));
			vugroup->timestamp = time(NULL);
			vugroup->nvusers   = nvusers;
			strncpy(vugroup->vupattern, arg, sizeof(vugroup->vupattern));
			vugroup->vupattern[sizeof(vugroup->vupattern) - 1] = 0;
			vugroup->vusers	   = VUserCreate(nvusers, vugroup->vupattern);
			vugroups = VectorAdd(vugroups, vugroup);
			break;
		case VUCMD_DESTROY:
			vugroup = VUserGroupLookup(arg);
			if (vugroup)
				VUserDestroy(vugroup->vusers, vugroup->nvusers);
			break;
		case VUCMD_MASSJOIN:
			vugroup = VUserGroupLookup(arg);
			if (vugroup) {
				arg2 = strchr(arg, ' ');
				if (!arg2) {
					printf("ERROR: invalid number of parameters\n");
					return;
				}
				while (*arg2 == ' ')
					arg2++;
				VUserMassChannel(vugroup->vusers, vugroup->nvusers, arg2);
			}
			break;
		case VUCMD_LOADCHAN:
			//VUserLoadChannel(vu_ltest, nvultest, "test");
			break;
		case VUCMD_FLOODCHAN:
			break;
		case VUCMD_STOP:
			;
	}
}


LPSESS *VUserCreate(int nvusers, const char *namepattern) {
	LPSESS *vusers;
	LPSESS sess;
	int i;

	vusers = malloc(nvusers * sizeof(LPSESS));

	for (i = 0; i != nvusers; i++) {
		sess = malloc(sizeof(SESS));
		memset(sess, 0, sizeof(SESS));

		sess->sck	      = -1;
		sess->cpindex     = -1;
		sess->vecindex    = -1;
		sess->client      = 'STAR';
		sess->clientindex = CI_STAR;
		sess->state       = US_BNCS | US_CLISC;

		if (namepattern) {
			strncpy(sess->username, namepattern, sizeof(sess->username));
			sess->username[sizeof(sess->username) - 1] = 0;
		} else {
			NameRandomGenerate(sess->username);
		}
		UserLogon(sess);
		nusers_bncs++;
		vusers[i] = sess;
	}

	return vusers;
}


void VUserDestroy(LPSESS *vusers, int nvusers) {
	int i;

	for (i = 0; i != nvusers; i++) {
		UserLogoff(vusers[i]);
		free(vusers[i]);
		nusers_bncs--;
	}
}


void VUserMassChannel(LPSESS *vusers, int nvusers, const char *channel) {
	int i;

	for (i = 0; i != nvusers; i++)
		ChannelJoin(channel, vusers[i], 0);
}


LPVUGROUP VUserGroupLookup(const char *vupattern) {
	LPVUGROUP vugroup;
	int i;

	if (!vupattern) {
		printf("ERROR: invalid number of parameters\n");
		return NULL;
	}
	for (i = 0; i != vugroups->numelem; i++) {
		vugroup = vugroups->elem[i];
		if (!strcasecmp(vugroup->vupattern, vupattern))
			return vugroup;
	}
	printf("ERROR: vugroup pattern \'%s\' not allocated\n", vupattern);
	return NULL;
}
