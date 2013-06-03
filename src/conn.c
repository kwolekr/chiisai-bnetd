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

/* 
 * conn.c - 
 *    Routines to listen for, create, handle, maintain, and service connections using
 *    kqueue on FreeBSD, NetBSD, DragonFly BSD
 *    epoll on Linux 
 *    /dev/poll on Solaris, OpenSolaris
 *    poll on OpenBSD, BSD/OS, System V, Mac OS X, AIX, Tru64 UNIX, and HP-UX
 *    WSAPoll on Windows NT >=6.0
 *    WSAWaitForMultipleEvents on Windows NT < 6.0
 *    ...and select for anything not specifically supported
 */

#include "main.h"
#include "user.h"
#include "fxns.h"
#include "ht.h"
#include "vector.h"
#include "packets.h"
#include "channel.h"
#include "conn.h"

//#include <signal.h>


LPGENSESS alert_sess;
static struct sockaddr *alert_addr;
SOCKET udp_sck;
SOCKET alert_sck;
SOCKET listen_sck;
pthread_t listen_tid;

LPVECTOR threadpool;

int lowestavailpool = -1;


///////////////////////////////////////////////////////////////////////////////

_inline int InitSocketMultiplexing() {
	int smfd;

	#if defined(_USE_KQUEUE)
		smfd = kqueue();
	#elif defined(_USE_EPOLL)
		smfd = epoll_create(NUM_CONNS_PER_POOL);
	#elif defined(_USE_DEVPOLL)
		smfd = open("/dev/poll", O_RDWR));
		if (smfd < 0)
			return -1;
	#endif
	return smfd;
}


_inline void AddSocket(SOCKET sck, LPCONNPOOL cp) {
	#if defined(_USE_KQUEUE)
		struct kevent ke;

		EV_SET(&ke, sck, EVFILT_READ, EV_ADD, 0, 0, NULL);
		kqueue(cp->smfd, &ke, 1, NULL, 0, NULL);
	#elif defined(_USE_EPOLL)
	#elif defined(_USE_DEVPOLL)
	#elif defined(_USE_POLL)
	#elif defined(_USE_WSAPOLL)
	#elif defined(_USE_WSAEVENTS)
		
	#elif defined(_USE_SELECT)
		FD_SET(sck, &cp->rdfds);
	#endif
}


_inline void RemoveSocket(SOCKET sck, LPCONNPOOL cp) {
	#if defined(_USE_KQUEUE)
		struct kevent ke;

		EV_SET(&ke, sck, EVFILT_READ, EV_REMOVE, 0, 0, NULL);
		kqueue(cp->smfd, &ke, 1, NULL, 0, NULL);
	#elif defined(_USE_EPOLL)
	#elif defined(_USE_DEVPOLL)
	#elif defined(_USE_POLL)
	#elif defined(_USE_WSAPOLL)
	#elif defined(_USE_WSAEVENTS)
		
	#elif defined(_USE_SELECT)
		FD_CLR(sck, &cp->rdfds);
	#endif
}


_inline int PollSockets(LPCONNPOOL cp) {
	int num_fds, i;
	SOCKET fd_highest;
	LPSESS sess;

	#if defined(_USE_KQUEUE)
		num_fds = kevent(cp->smfd, NULL, 0, cp->events, cp->nconns, NULL);
		return num_fds;
		
	#elif defined(_USE_EPOLL)
		num_fds = epoll_wait(cp->smfd, cp->events, cp->nconns, -1);
		return num_fds;
		
	#elif defined(_USE_DEVPOLL)
		num_fds = ioctl(cp->smfd, DP_POLL, &pollstuff);
		return (num_fds >= 0) ? num_fds : -1;

	#elif defined(_USE_POLL)
		num_fds = poll(cp->fds, cp->nconns, -1);
		return num_fds;
		
	#elif defined(_USE_WSAPOLL)
		num_fds = WSAPoll(cp->fdarray, cp->nconns, -1);
		return num_fds;
		
	#elif defined(_USE_WSAEVENTS)
		num_fds = WSAWaitForMultipleEvents(cp->nconns, cp->events, FALSE, WSA_INFINITE, TRUE);
		if (num_fds == WSA_WAIT_FAILED)
			return -1;
		WSAResetEvent(cp->events[num_fds - WSA_WAIT_EVENT_0]);
		return num_fds;

	#elif defined(_USE_SELECT)
		FD_ZERO(&cp->rdfds);
		fd_highest = 0;
		for (i = 0; i != cp->nconns; i++) {
			sess = cp->sessions[i];
			FD_SET(sess->sck, &cp->rdfds);
			if (sess->sck > fd_highest)
				fd_highest = sess->sck;
		}
		//printf("#before select(): nfds %d\n", cp->rdfds.fd_count);
		num_fds = select(fd_highest + 1, &cp->rdfds, NULL, NULL, NULL);
		//printf("#after select(): nfds %d\n", cp->rdfds.fd_count);
		/*if (!num_fds) {
			printf("WARNING: fd_count being manually set!\n");
			cp->rdfds.fd_count = cp->nconns - cp->n_connupdates;
			return 0;
		}*/
		//return num_fds;
		return (num_fds != -1) ? cp->nconns : -1;
	#endif
}


///////////////////////////////////////////////////////////////////////////////


SOCKET CreateUDPSocket(unsigned long addr, unsigned short port, struct sockaddr_in *name) {
	struct sockaddr_in lname;
	unsigned long on;
	SOCKET s;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (!s) {
		printf("couldn't create UDP socket, errno: %d\n", err());
		return -1;
	}

	if (!name)
		name = &lname;

	memset(&name->sin_zero, 0, sizeof(name->sin_zero));
	name->sin_family	  = AF_INET;
	name->sin_port		  = htons(port);
	name->sin_addr.s_addr = htonl(addr);

	if (bind(s, (const struct sockaddr *)name, sizeof(*name)) == -1) {
		printf("couldn't bind udp socket to network interface (%d)\n", err());
		return 0;
	}

	on = 1;
	ioctlsocket(s, FIONBIO, (u_long *)&on);

	return s;
}


int SetupAlertSocket() {
	alert_sess = malloc(sizeof(GENSESS));
	alert_sess->name[0]    = 0;
	alert_sess->sessid     = -1;
	alert_sess->state      = 0;
	alert_sess->cpindex    = -1;
	alert_sess->vecindex   = 0;
	alert_sess->pkthandler = NULL;
	alert_sess->sck        = CreateUDPSocket(INADDR_LOOPBACK, ALERT_PORT, &alert_sess->addr);
	
	alert_sck  = alert_sess->sck;
	alert_addr = (struct sockaddr *)&alert_sess->addr;
	return (alert_sck != -1);
}


void AlertSocket() {
	int len;
	
	len = sendto(alert_sck, "", 1, 0, alert_addr, sizeof(*alert_addr));
	if (len != 1)
		printf("WARNING: thread alert sendto() returned %d, errno: %d", len, err());
}


int SetupListenSocket(SOCKET *lpsck, pthread_t *lptid) {
	struct sockaddr_in localname;
	SOCKET s;
	int blah;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == -1) {
		printf("couldn't create socket (%d)\n", err());
		return 0;
	}

	blah = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&blah, sizeof(blah)) == -1) {
		printf("couldn't SO_RESUEADDR (%d\n", err());
		return 0;
	}

	memset(&localname, 0, sizeof(localname));
	
	localname.sin_family	  = AF_INET;
	localname.sin_port		  = htons(SERVER_PORT);
	localname.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (const struct sockaddr *)&localname, sizeof(localname)) == -1) {
		printf("couldn't bind to network interface (%d)\n", err());
		return 0;
	}

	*lpsck = s;

	_CreateThread(ListenThreadProc, (void *)s);
	//CloseHandle(CreateThread(NULL, 0, ListenThreadProc, (void *)s, 0, lptid));
	return 1;
}


TPRET CALLBACK ListenThreadProc(void *arg) {
	int tpindex, addrlen;
	struct sockaddr addr;
	LPSESS newsess;
	LPCONNPOOL cp;
	unsigned long on;
	SOCKET acceptsck, s = (SOCKET)arg;

	on = 1;
	printf(" - listening on port %d\n", SERVER_PORT);
	_SignalWaitObject(&maininit);

	while (1) {
		cp = NULL;

		listen(s, SOMAXCONN);
		addrlen = sizeof(addr);
		acceptsck = accept(s, &addr, &addrlen);
		nreqs++;
		
		newsess = malloc(sizeof(SESS));
		memcpy(&newsess->addr, &addr, sizeof(addr));
		newsess->sck    = acceptsck;
		newsess->sessid = nsessions;
		newsess->state  = 0;
		newsess->name[0] = 0;
		nsessions++;
		
		ioctlsocket(newsess->sck, FIONBIO, (u_long *)&on);

		tpindex = threadpool->numelem - 1;

		if (tpindex == lowestavailpool) {
			if (tpindex != -1) {
				cp = threadpool->elem[lowestavailpool];
				cp->nconns++;
				//cp->n_connupdates++;
			}
			if (tpindex == -1 || cp->nconns == NUM_CONNS_PER_POOL) {
				tpindex++;
				ConnPoolAlloc(tpindex, newsess);
				lowestavailpool = tpindex;
				cp = threadpool->elem[lowestavailpool];
			}
			cp->sessions[cp->nconns - 1] = newsess;
		} else {
			///////////TODO: fix this!!!!!!
			cp = threadpool->elem[lowestavailpool];
			cp->sessions[cp->nconns++] = newsess;
			//cp->n_connupdates++;
			
		}
		//cp->tid |= TP_NEWSCK;
		_SignalWaitObject(&cp->readyevent);
		newsess->cpindex = lowestavailpool;
		newsess->vecindex = cp->nconns - 1;
		AlertSocket();
		//_SignalPollingWait(cp);
	}
}


///////////////////////////////////////////////////////////////////////////////


void ConnPoolAlloc(int tpindex, LPSESS newsess) {
	/*#ifdef _WIN32
		LPGENSESS sigcp;
		struct sockaddr_in name;
	#endif*/
	LPCONNPOOL newcp;
	//unsigned long on = 1;

	newcp = malloc(sizeof(CONNPOOL));
	newcp->nconns        = 2;
	newcp->sessions[0]   = (LPSESS)alert_sess;
	newcp->sessions[1]   = newsess;
	//newcp->n_connupdates = 0;
	threadpool = VectorAdd(threadpool, newcp);

	#if defined(_USE_EPOLL)
	#elif defined(_USE_DEVPOLL)
	#elif defined(_USE_POLL)
	#elif defined(_USE_WSAEVENTS)
	#elif defined(_USE_SELECT)
		FD_ZERO(&newcp->rdfds);
	#endif

	#if _SM_NEED_INIT
		newcp->smfd = InitSocketMultiplexing();
		if (newcp->smfd == -1) {
			printf("Couldn't open socket multiplexing file descriptor!\n");
			free(newcp);
			return;
		}
	#endif

	AddSocket(alert_sck, newcp);
	AddSocket(newsess->sck, newcp);

	if (!_CreateWaitObject(&newcp->readyevent)) {
		printf("[T%02d] Couldn't create wait object, err %d\n", tpindex, err());
		return;
	}
	newcp->tid = _CreateThread(RecvThreadProc, (void *)tpindex);
}


TPRET CALLBACK RecvThreadProc(void *arg) {
	LPCONNPOOL cp;
	LPSESS sess;
	int index, i, num_fds, rval;
	char rfbuf;

	index = (int)arg;
	cp = (LPCONNPOOL)(threadpool->elem[index]);

	while (1) {
		rval = _WaitForObject(&cp->readyevent, 10 * 60 * 1000);
		if (rval == 1) {
			printf("Deallocating conn pool %d\n", index);
			if (index == lowestavailpool)
				lowestavailpool = threadpool->numelem - 1;
			threadpool->elem[index] = threadpool->elem[threadpool->numelem];
			threadpool->numelem--;
			free(cp);
			return 1;
		} else if (rval == -1) {
			printf("[T%02d] _WaitForObject() fail (%d)", index, err());
			return (TPRET)-1;
		} else {
			#ifdef _WIN32
				ResetEvent(cp->readyevent);
			#endif
			//_ResetWaitObject(cp->readyevent);
		}

		do {
			num_fds = PollSockets(cp);
			if (num_fds == -1) {
				printf("[T%02d] " POLL_WAIT_FXN_NAME_STR "() fail (%d)\n", index, err());
				return (TPRET)-1;
			}

			for (i = 0; i != num_fds; i++) {
				sess = cp->sessions[i];
				if (sess->state & US_ENDSESS) {
					SessionDisconnect(sess);
					i--;
					num_fds--;
				} else {
					if (FD_ISSET(sess->sck, &cp->rdfds)) {
						if (i) {
							if (sess->state & US_FIRSTSEND)
								sess->pkthandler(sess);
							else
								SessionReceiveType(sess);
						} else {
							if (recvfrom(alert_sck, &rfbuf, sizeof(rfbuf), 0, NULL, NULL) != 1)
								printf("recvfrom() failure on pool %d!\n", index);
						}
					}
				}
			}
		} while (num_fds);
	}
	return NULL;
}


void SessionReceiveType(LPSESS sess) {
	unsigned char sesstype;
	int retval;

	retval = recv(sess->sck, &sesstype, sizeof(unsigned char), 0);
	if (retval != 1) {
		printf("SessionReceiveType recv() == %d! errno: %d\n", retval, err());
		SessionReqDisconnect(sess);
		return;
	}

	sess->sesstype = sesstype;
	switch (sesstype) {
		case SESSTYPE_BNCS: //bncs
			nusers_bncs++;

			memset((char *)sess + sizeof(GENSESS), 0, sizeof(SESS) - sizeof(GENSESS));

			sess->recvbuf = malloc(RBUF_LEN);
			sess->sendbuf = malloc(SBUF_LEN);
			
			sess->sbufpos = 4;
			sess->pkthandler = BNCSPacketHandler;
			sess->state |= US_BNCS;
			break;
		case SESSTYPE_BNFTP: //bnftp
			nusers_bnftp++; /////not finished
			sess->pkthandler = BNFTPPacketHandler;
			sess->state |= US_BNFTP;
			break;
		case SESSTYPE_TELNET: //telnet
			nusers_telnet++;
			memset(sess + sizeof(GENSESS), 0, sizeof(SESS) - sizeof(GENSESS));
			sess->pkthandler = TelnetPacketHandler;
			sess->state |= US_TELNET;
			break;
		case SESSTYPE_COMM: //inter-node communication
			nusers_sync++;
			sess->state |= US_SYNC;
			/////TODO
			break;
		case SESSTYPE_ADMIN: //administrative tasks
			nusers_admin++;
			sess->state |= US_ADMIN;
			/////TODO
			break;
		case SESSTYPE_REP: //rep/sysop logon
			nusers_reps++;
			memset(sess + sizeof(GENSESS), 0, sizeof(SESS) - sizeof(GENSESS));
			sess->pkthandler = BNCSPacketHandler;
			sess->state |= US_REP;
			break;
		default:
			printf("Connection attempt with bogus session type 0x%02x terminated.\n", sesstype);
			SessionReqDisconnect(sess);
			return;
	}
	sess->state |= US_FIRSTSEND;
}


void SessionReqDisconnect(LPSESS sess) {
	sess->state |= US_ENDSESS;
	shutdown(sess->sck, SHUT_RDWR);
}


void SessionDisconnect(LPSESS sess) {
	LPCONNPOOL cp;

	cp = threadpool->elem[sess->cpindex];
	closesocket(sess->sck);

	switch (sess->sesstype) {
		case SESSTYPE_BNCS:
			nusers_bncs--;
			goto bncscase;
		case SESSTYPE_TELNET:
			nusers_telnet--;
			goto bncscase;
		case SESSTYPE_REP:
			nusers_reps--;
		bncscase:
			UserLogoff(sess);
			break;
		case SESSTYPE_BNFTP:
			nusers_bnftp--;
			break;
		case SESSTYPE_COMM:
			nusers_sync--;
			break;
		case SESSTYPE_ADMIN:
			nusers_admin--;
	}
	cp->nconns--;
	//RemoveSocket(sess->sck, cp);
	cp->sessions[cp->nconns]->vecindex = sess->vecindex;
	cp->sessions[sess->vecindex] = cp->sessions[cp->nconns];
	if (sess->cpindex < lowestavailpool)
		lowestavailpool = sess->cpindex;

	free(sess->sendbuf);
	free(sess->recvbuf);
	free(sess);
	// TODO: Consider memory pooling with a simple
	// stack consisting of free sessions.
}
