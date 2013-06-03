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

#ifndef VUSER_HEADER
#define VUSER_HEADER

#define VUCMD_CREATE	0
#define VUCMD_DESTROY	1
#define VUCMD_MASSJOIN	2
#define VUCMD_LOADCHAN	3
#define VUCMD_FLOODCHAN 4
#define VUCMD_STOP		5

typedef struct {
	char vupattern[16];
	LPSESS *vusers;
	int nvusers;
	time_t timestamp;
} VUGROUP, *LPVUGROUP;

void VUserInternalCmd(char *text);
LPSESS *VUserCreate(int nvusers, const char *namepattern);
void VUserDestroy(LPSESS *vusers, int nvusers);
void VUserMassChannel(LPSESS *vusers, int nvusers, const char *channel);
LPVUGROUP VUserGroupLookup(const char *vupattern);

#endif //VUSER_HEADER
