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

#ifndef CONN_HEADER
#define CONN_HEADER

#if defined(_USE_KQUEUE)
#elif defined(_USE_EPOLL)
#elif defined(_USE_DEVPOLL)
#elif defined(_USE_POLL)
#elif defined(_USE_WSAPOLL)
#elif defined(_USE_WSAEVENTS)
#elif defined(_USE_SELECT)
#endif

#if defined(_USE_KQUEUE)
	#define POLL_WAIT_FXN_NAME_STR "kevent"
#elif defined(_USE_EPOLL)
	#define POLL_WAIT_FXN_NAME_STR "epoll_wait"
#elif defined(_USE_DEVPOLL)
	#define POLL_WAIT_FXN_NAME_STR "poll ioctl"
#elif defined(_USE_POLL)
	#define POLL_WAIT_FXN_NAME_STR "poll"
#elif defined(_USE_WSAPOLL)
	#define POLL_WAIT_FXN_NAME_STR "WSAPoll"
#elif defined(_USE_WSAEVENTS)
	#define POLL_WAIT_FXN_NAME_STR "WSAWaitForMultipleEvents"
#elif defined(_USE_SELECT)
	#define POLL_WAIT_FXN_NAME_STR "select"
#endif

#define TP_NEWSCK 0x80000000
#define TP_PID(x) ((x) & ~TP_NEWSCK)

SOCKET CreateUDPSocket(unsigned long addr, unsigned short port, struct sockaddr_in *name);
void AlertSocket();
int SetupAlertSocket();
int SetupListenSocket(SOCKET *lpsck, pthread_t *lptid);
TPRET CALLBACK ListenThreadProc(void *arg);
void ConnPoolAlloc(int tpindex, LPSESS newsess);
TPRET CALLBACK RecvThreadProc(void *arg);
void SessionReceiveType(LPSESS sess);
void SessionReqDisconnect(LPSESS sess);
void SessionDisconnect(LPSESS sess);

extern SOCKET udp_sck;
extern SOCKET alert_sck;
extern SOCKET listen_sck;
extern pthread_t listen_tid;
extern LPVECTOR threadpool;
extern int lowestavailpool;

#endif //CONN_HEADER
