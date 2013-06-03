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

#ifndef MAIN_HEADER
#define MAIN_HEADER

/////configuration
#define VERBOSE
#define NUM_INITIAL_POOLS	8
#define NUM_CONNS_PER_POOL	64
#define FD_SETSIZE			NUM_CONNS_PER_POOL
#define SERVER_PORT			6112
#define ALERT_PORT			6113

#define TL_USERS			4096
#define TL_CHANNELS			1024
#define TL_BANS				256

#define BNCS_PKTDROPLEN		0x2000
#define RBUF_LEN			0x800
#define SBUF_LEN			0x800
#define MAX_NAME_LEN		32
#define MAX_USERNAME_LEN	16
#define MIN_USERNAME_LEN    2
#define MAX_CHANNAME_LEN	64
#define MAX_PROFILEKEY_LEN  64
#define MAX_PROFILEENT_LEN  512
#define CHANNEL_BAN_LIMIT	170

#define MIN_NLSUSERNAME_LEN			  3
#define MIN_NLSUSERNAME_ALPHANUMCHARS 2
#define MAX_NLSUSERNAME_PUNCTCHARS    4

#define IPBLEN_INVALID_FIELD_SIZE 20
#define IPBLEN_INVALID_CDKEY	  2880
#define IPBLEN_RULE_VIOLATION	  60
#define IPBLEN_FLOOD_OUT		  5

#define REALM_NAME_SC		"Local"
#define REALM_NAME_LWR_SC   "local"
#define REALM_NAME_WC		"Zothor"
#define REALM_NAME_LWR_WC   "zothor"
#define BAN_CHANNEL_NAME	"The Void"
#define MOD_CHAN_PREFIX		"op"
#define CLAN_CHAN_PREFIX	"clan"

#if NUM_CONNS_PER_POOL < 4
	#error "NUM_CONNS_PER_POOL must be >= 4."
#endif


/////normal defines
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define verbose (1)
#define _USE_SELECT

//#define _USE_WSAEVENTS

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <ntsecapi.h>
	#include <wincrypt.h>
	#include <winsock2.h>
	#include <intrin.h>

	#pragma comment(lib, "WS2_32.lib")
	#pragma comment(lib, "Crypt32.lib")
	#pragma comment(lib, "sqlite3.lib")
	
	#pragma warning(disable : 4305) //stupid warning about 'int->char truncation'
	#pragma warning(disable : 4996) //stupid warning about POSIX being deprecated

	/*#if (_WIN32_WINNT  >= 	0x600)
		#define _USE_WSAPOLL
	#elif (_WIN32_WINNT >= 0x500)
		#define _USE_WSAEVENTS
	#else
		#define _USE_SELECT
	#endif*/

	#define SHUT_RDWR 2
	#define EWOULDBLOCK WSAEWOULDBLOCK
	
	#define err() (WSAGetLastError())
	#define strcasecmp(x,y) stricmp(x,y)
	#define strncasecmp(x,y,z) strnicmp(x,y,z)
	#define	gettick() GetTickCount()
	#define alloca(x) _alloca(x)
	#define snprintf(x,y,z,l) _snprintf(x,y,z,l)

	typedef __int8 int8_t;
	typedef __int16 int16_t;
	typedef __int32 int32_t;
	typedef __int64 int64_t;

	typedef unsigned __int8 uint8_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int64 uint64_t;

	typedef unsigned long pthread_t;

	typedef DWORD TPRET;
	typedef HANDLE WAITOBJ;

#else
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	//#include <sys/mman.h>
	#include <sys/utsname.h>
	#include <sys/times.h>
	//#include <sys/ucontext.h>
	#include <net/if.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <string.h>
	#include <stdint.h>
	#include <limits.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include <signal.h>
	#include <netdb.h>
	#include <errno.h>
	#include <ctype.h>
	#include <pwd.h>
	#include <sha.h>
	#include <md5.h>

	#define CALLBACK 
	typedef int SOCKET;
	#define _inline inline
	
	#if !defined(__fastcall) && defined(__i386__) 
		#define __fastcall __attribute__((__fastcall__))
	#else
		#undef __fastcall
		#define __fastcall
	#endif
	
	#define err() (errno)
	#define closesocket(x) close(x)
	#define ioctlsocket(x,y,z) ioctl(x,y,(char *)z)

	typedef void *TPRET;
	typedef sem_t WAITOBJ;

#endif

#if !(__GNUC__ == 2 && __GNUC_MINOR__ == 95)
	#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901 || defined(lint)
		#define __restrict
	#else
		#define __restrict      restrict
	#endif
#endif

//#if __POSIX_VISIBLE < 200809 && !__BSD_VISIBLE
#ifdef _WIN32
	#define stpcpy(x,y) _stpcpy(x,y)
	#define stpncpy(x,y,c) _stpncpy(x,y,c)
#endif

#include <sqlite3.h>
#include <gmp.h>

#define PERR(x) fprintf(stderr, "ERROR in %s:%d: %s!%s failed\n",\
	__FILE__, __LINE__, __FUNCTION__, x)

#define PSQLERR(x) fprintf(stderr, "SQL ERROR in %s:%d: %s!%s failed, %s\n",\
	__FILE__, __LINE__, __FUNCTION__, x, sqlite3_errmsg(db_accounts))

#if defined(_USE_KQUEUE) || defined(_USE_EPOLL) || defined(_USE_DEVPOLL)
	#define _SM_NEED_INIT
#endif

#define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))
#define SWAP2(num) ((((num) >> 8) & 0x00FF) | (((num) << 8) & 0xFF00))
#define SWAP4(num) ((((num) >> 24) & 0x000000FF) | (((num) >> 8) & 0x0000FF00) | (((num) << 8) & 0x00FF0000) | (((num) << 24) & 0xFF000000))


#define SESSTYPE_BNCS	0x01
#define SESSTYPE_BNFTP	0x02
#define SESSTYPE_TELNET 0x03
#define SESSTYPE_COMM	0x06
#define SESSTYPE_ADMIN	0x80
#define SESSTYPE_REP	0x81

#define PC_50 0x01
#define PC_51 0x02
#define PC_52 0x04
#define PC_53 0x08
#define PC_54 0x10
#define PC_0A 0x20

#define	PC_CONNDONENLS (PC_50 | PC_51 | PC_52 | PC_53 | PC_54)

typedef TPRET (CALLBACK *THREADPROC)(void *);

struct _sess;

typedef struct _vector {
	int numelem;
	int maxelem;
	void *elem[0];
} VECTOR, *LPVECTOR;

typedef struct _connpool {
	pthread_t tid;
	#ifdef _SM_NEED_INIT
		int smfd;
	#endif
	WAITOBJ readyevent;
	#if defined(_USE_KQUEUE)
		struct kevent events[NUM_CONNS_PER_POOL];
	#elif defined(_USE_EPOLL)
		struct epoll_events events[NUM_CONNS_PER_POOL];
	#elif defined(_USE_DEVPOLL) || defined(_USE_POLL)
		struct pollfd fds[NUM_CONNS_PER_POOL];
	#elif defined(_USE_WSAPOLL)
		WSAPOLLFD fdarray[NUM_CONNS_PER_POOL];
	#elif defined(_USE_WSAEVENTS)
		WSAEVENT events[NUM_CONNS_PER_POOL];
	#elif defined(_USE_SELECT)
		struct fd_set rdfds;
	#endif
	int nconns;
	struct _sess *sessions[NUM_CONNS_PER_POOL];
} CONNPOOL, *LPCONNPOOL;

typedef struct _node {
	unsigned int key;
	int data;
	struct _node *lchild;
	struct _node *rchild;
} NODE, *LPNODE;

typedef struct _rangelink {
	unsigned short lbound;
	unsigned short ubound;
	struct _rangelink *next;
} RANGELINK, *LPRANGELINK;

typedef struct _localeinfo {
	int prodlang;
	int tzbias;
	int localeid;
	int langid;
	char country_s[8];
	char country[32];
} LOCALEINFO, *LPLOCALEINFO;

typedef struct _clanmember {
	char username[MAX_USERNAME_LEN];
	unsigned int rank;
	unsigned int joindate;
} CLANMEMBER, *LPCLANMEMBER;

typedef struct _nls {
    char namehash[20];
	mpz_t b;
	mpz_t b_src;
	mpz_t v;
	char salt[32];
	char A[32];
	char B[32];
	char V[32];
	char K[40];
	char M1[20];
} NLS, *LPNLS;

typedef struct _clan {
	uint32_t name;
	LPVECTOR members;
} CLAN, *LPCLAN;

typedef struct _friend {
	char name[32];
	short mutual;
} FRIEND, *LPFRIEND;

typedef struct _stats {
	int wins;
	int losses;
	int draws;
} STATS, *LPSTATS;

typedef struct _lstats {
	int wins;
	int losses;
	int draws;
	int rating;
} LADDERSTATS, *LPLADDERSTATS;

typedef struct _channel {
	char name[MAX_CHANNAME_LEN];
	unsigned short flags;
	unsigned short product;
	unsigned short limit;
	unsigned short nusers;
	int nbans;
	LPVECTOR *bans;
	struct _sess *firstuser;
	struct _sess *lastadded;
} CHANNEL, *LPCHANNEL;

typedef struct _gensess {
	char name[MAX_NAME_LEN];
	struct sockaddr_in addr;
	SOCKET sck;
	unsigned int sessid;
	unsigned int state;
	unsigned int sesstype;
	int cpindex;
	int vecindex;
	void (__fastcall *pkthandler)(struct _sess *);
} GENSESS, *LPGENSESS;

typedef struct _sess {
	char name[MAX_NAME_LEN];
	struct sockaddr_in addr;
	SOCKET sck;
	unsigned int sessid;
	unsigned int state;
	unsigned int sesstype;
	int cpindex;
	int vecindex;
	void (__fastcall *pkthandler)(struct _sess *);

	int connected;
	char *recvbuf;
	unsigned int rbufpos;
	char *sendbuf;
	unsigned int sbufpos;

	unsigned int pkts;
	uint32_t flags;
	
	uint32_t platform;
	uint32_t client;
	int clientindex; 
	uint32_t verbyte;
	LOCALEINFO li;
	uint32_t clienttoken;
	uint32_t servertoken;
	uint32_t ping;
	unsigned int lastping;

	LPNLS nls;
	int accid;
	unsigned short nameno;
	char username[16];
	char statstring[64];

	char awaymsg[128];
	char dndmsg[128];

	struct _sess *designated;
	LPCHANNEL channel;
	int channelpos;

	STATS stats;
	LADDERSTATS lstats;

	LPVECTOR friends;

	struct _sess *nextuser;
	struct _sess *lastuser;
} SESS, *LPSESS;

typedef struct _namenodesc {
	char name[MAX_NAME_LEN];
	int popcount;
	int arrlen;
	unsigned int *bitfield;
} NAMENODESC, *LPNAMENODESC;

#define BP_BITBLOCK (CHAR_BIT * sizeof(((LPNAMENODESC)NULL)->bitfield[0]))

void LoadDefaultConfig();
void ParseCmdLine(int argc, char *argv[]);
void __fastcall BNCSPacketHandler(LPSESS sess);
void __fastcall BNFTPPacketHandler(LPSESS sess);
void __fastcall TelnetPacketHandler(LPSESS sess);

extern WAITOBJ maininit;

extern LPVECTOR users[TL_USERS];
extern LPVECTOR channels[TL_CHANNELS];

extern sqlite3* db_accounts;

extern int nreqs;
extern int nsessions;

extern int nusers_bncs;
extern int nusers_bnftp;
extern int nusers_telnet;
extern int nusers_sync;
extern int nusers_admin;
extern int nusers_reps;

#endif //MAIN_HEADER
