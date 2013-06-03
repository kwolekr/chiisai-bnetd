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

#ifndef FRIENDS_HEADER
#define FRIENDS_HEADER

#define FN_JOINGAME 0
#define FN_EXIT		1

#define FCMD_LIST	 0
#define FCMD_ADD	 1
#define FCMD_REMOVE	 2
#define FCMD_MSG	 3
#define FCMD_PROMOTE 4
#define FCMD_DEMOTE  5

void FriendLoad(LPSESS sess);
void FriendCmdHandle(LPSESS sess, char *text);
void FriendNotify(LPSESS sess, const char *fmt, ...);
void FriendListAll(LPSESS sess);
void FriendAdd(LPSESS sess, const char *arg);
void FriendRemove(LPSESS sess, const char *arg);
void FriendMessage(LPSESS sess, const char *text);
int FriendUpdateMutual(LPSESS sess, const char *friendname, int set);
int FriendScan(LPSESS sess, const char *friendname);
void FriendReposition(LPSESS sess, const char *friendname, int newpos, int promo);

#endif //FRIENDS_HEADER
