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
 
#ifndef FXNS_HEADER
#define FXNS_HEADER

static _inline char lwr(char c) {
	return c | (0x20 & -((c >= 'A') && (c <= 'Z')));
}

void HexToStr(char *in, char *out);
void lcase(char *str);
void lcasecpy(char *__restrict dest, const char *__restrict src);
void ucasecpy(char *__restrict dest, const char *__restrict src);
char *lcasencpy(char *__restrict dest, const char *__restrict src, unsigned int count);
int strilcmp(const char *s1, const char *s2);
#ifdef _WIN32
	char *stpcpy(char *__restrict dest, const char *__restrict src);
	char *stpncpy(char *__restrict dest, const char *__restrict src, unsigned int count);
#endif
int bitscanfwd(unsigned int val);

void SetSessClientInfo(LPSESS sess, uint32_t client);
int GetClientIndex(uint32_t client);

#ifndef _WIN32
	unsigned int gettick();
	int GetUidFromUsername(const char *username);
#endif

int GetNumberOfProcessors();

pthread_t _CreateThread(THREADPROC tproc, void *arg);
void _ExitThread(int exitcode);
void _CancelThread(pthread_t tid, int exitcode);
void _SignalPollingWait(LPCONNPOOL cp);

int _CreateWaitObject(WAITOBJ *waitobj);
int _WaitForObject(WAITOBJ *waitobj, int timeout);
void _SignalWaitObject(WAITOBJ *waitobj);

int BindThreadToProcessor(pthread_t tid, int pnumber);

#ifdef _WIN32
	extern BOOLEAN (WINAPI *lpfnRtlGenRandom)(PVOID, ULONG);
#endif
void RandomFillBuf(unsigned char *buf, int len);
unsigned long RandomGenSecure();

#endif //FXNS_HEADER
