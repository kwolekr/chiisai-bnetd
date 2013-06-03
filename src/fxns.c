/*-
 * Copyright (c) 2008 Ryan Kwolek 
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
 * fxns.c - 
 *    Miscellaneous functions and routines, extensions to the CRT
 */

#include "main.h"
#include "user.h"
#include "conn.h"
#include "fxns.h"

#ifdef _WIN32
	BOOLEAN (WINAPI *lpfnRtlGenRandom)(PVOID, ULONG);
#endif

///////////////////////////////////////////////////////////////////////////////


void HexToStr(char *in, char *out) {
	while (*in) {
		if ((unsigned char)(*in - 'a') <= 'f' - 'a')
			*out = (*in - 'a' + 10) << 4;
		else if ((unsigned char)(*in - '0') <= '9' - '0')
			*out = (*in - '0') << 4;
		else
			return;
		in++;
		if ((unsigned char)(*in - 'a') <= 'f' - 'a')
			*out |= (*in - 'a' + 10);
		else if ((unsigned char)(*in - '0') <= '9' - '0')
			*out |= (*in - '0');
		else
			return;
		in++;
		out++;
	}
}


void lcase(char *str) {
	while (*str) {
		*str = tolower((unsigned char)*str);
		str++;
	}
}


void lcasecpy(char *__restrict dest, const char *__restrict src) {
	while (*dest++ = tolower((unsigned char)*src++));
}


void ucasecpy(char *__restrict dest, const char *__restrict src) {
	while (*dest++ = toupper((unsigned char)*src++));
}


char *lcasencpy(char *__restrict dest, const char *__restrict src, unsigned int count) {
	int i = 0;
	count--;

	while (src[i] && (i != count)) {
		dest[i] = tolower((unsigned char)src[i]);
		i++;
	}
	dest[i] = 0;
	return dest;
}


int strilcmp(const char *s1, const char *s2) {
	while (*s1) {
		int blah = *s1 - tolower((unsigned char)*s2);
		if (blah)
			return blah;
		s1++;
		s2++;
	}
	return *s1 - tolower((unsigned char)*s2);
}


char *_stpcpy(char *__restrict dest, const char *__restrict src) {
	while (*dest++ = *src++);
	return dest - 1;
}


char *_stpncpy(char *__restrict dest, const char *__restrict src, unsigned int count) {
	while (count-- && (*dest++ = *src++));
	dest[-1] = 0;
	return dest - 1;
}


int bitscanfwd(unsigned int val) {
	#if defined __GNUC__
		int index = __builtin_ffs(val);
		//index = __builtin_clz(mask ^ (BP_BITBLOCK - 1));
	#elif defined _MSC_VER && _MSC_VER >= 1400
		unsigned long index;

		_BitScanForward(&index, (unsigned long)val);
	#else
		unsigned int index;

		val = (val ^ (val - 1)) >> 1;
		for (index = 0; val; index++)
			val >>= 1;
	#endif
	return (int)index;
}


///////////////////////////////////////////////////////////////////////////////

void SetSessClientInfo(LPSESS sess, uint32_t client) {
	int ci;

	switch (client) {
		case 'SSHR':
			ci = CI_SSHR;
			goto leg;
		case 'JSTR':
			ci = CI_JSTR;
			goto leg;
		case 'DRTL':
			ci = CI_DRTL;
			goto leg;
		case 'DSHR':
			ci = CI_DSHR;
			goto leg;
		case 'W2BN':
			ci = CI_W2BN;
		leg:
			sess->state |= US_CLILEGACY;
			break;
		case 'STAR':
			ci = CI_STAR;
			goto sc;
		case 'SEXP':
			ci = CI_SEXP;
		sc:
			sess->state |= US_CLISC;
			break;
		case 'D2DV':
			ci = CI_D2DV;
			goto diab;
		case 'D2XP':
			ci = CI_D2XP;
		diab:
			sess->state |= US_CLID2;
			break;
		case 'WAR3':
			ci = CI_WAR3;
			goto wc;
		case 'W3XP':
			ci = CI_W3XP;
		wc:
			sess->state |= US_CLIWC3;
			break;
		default:
			SessionReqDisconnect(sess);
			return;
	}
	sess->clientindex = ci;
}


int GetClientIndex(uint32_t client) {
	switch (client) {
		case 'STAR':
			return 0;
		case 'SEXP':
			return 1;
		case 'SSHR':
			return 2;
		case 'JSTR':
			return 3;

		case 'DSHR':
			return 4;
		case 'DRTL':
			return 5;
		case 'D2DV':
			return 6;
		case 'D2XP':
			return 7;

		case 'W2BN':
			return 8;
		case 'WAR3':
			return 9;
		case 'W3XP':
			return 10;
		case 'CHAT':
			return 11;
		default:
			return -1;
	}
}

///////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
int GetUidFromUsername(const char *username) {
	struct passwd *pass;

	if (!username)
		return -1;

	while (pass = getpwent()) {
		if (!strcmp(pass->pw_name, username)) {
			endpwent();		
			return pass->pw_uid;
		}
	}
	endpwent();
	return -1;
}
#endif


int GetNumberOfProcessors() {
	#if defined(_SC_NPROCESSORS_ONLN)
		return sysconf(_SC_NPROCESSORS_ONLN);

	#elif defined(__FreeBSD__) || defined(__APPLE__)
		unsigned int len, count;
		len = sizeof(count);
		return sysctlbyname("hw.ncpu", &count, &len, NULL, 0);

	#elif defined(_GNU_SOURCE)
		return get_nprocs();

	#elif defined(_WIN32)
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;

	#elif defined(PTW32_VERSION) || defined(__hpux)
		return pthread_num_processors_np();

	#else
		return 1;

	#endif
}


///////////////////////////////////////////////////////////////////////////////


pthread_t _CreateThread(THREADPROC tproc, void *arg) {
    pthread_t tid;
	#ifdef _WIN32
		CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tproc, arg, 0, &tid));
	#else
		pthread_create(&tid, NULL, tproc, arg);
	#endif
	return tid;
}


void _ExitThread(int exitcode) {
	#ifdef _WIN32
		ExitThread(exitcode);
	#else
		pthread_exit(&exitcode);
	#endif
}


void _CancelThread(pthread_t tid, int exitcode) {
	#ifdef _WIN32
		/*
		 * IMPORTANT TODO:
		 * TerminateThread() should NOT be used!!
		 * Termination with this function is abrupt and could severely damage the application
		 * (it could hold a critical section within a system library, for instance)
		 * Under Windows 2000, XP, and Server 2003, the stack isn't freed resulting in a leak.
		 *
		 *   GET PROPER EXITING EVENT CODE IN HERE ASAP!
		 *
		 */
		HANDLE hThread;
		hThread = OpenThread(THREAD_TERMINATE, 0, tid);
		if (hThread) {
			TerminateThread(hThread, exitcode);
			CloseHandle(hThread);
		}
	#else
		pthread_cancel(tid);
	#endif
}


///////////////////////////////////////////////////////////////////////////////

int _CreateWaitObject(WAITOBJ *waitobj) {
	#ifdef _WIN32
		*waitobj = CreateEvent(NULL, 0, 0, NULL);
		return *waitobj != NULL;
	#else
		return sem_init(waitobj, 0, 0) == 0;
	#endif
}


int _WaitForObject(WAITOBJ *waitobj, int timeout) {
	#ifdef _WIN32
		int waitresult;

		if (timeout == -1)
			timeout = INFINITE;
		waitresult = WaitForSingleObject(*waitobj, timeout);
		return (waitresult == WAIT_TIMEOUT) ? 1 : waitresult;
	#else
		struct timespec ts;
		struct timeval tp;

		if (timeout != -1) {
			gettimeofday(&tp, NULL);
			ts.tv_sec  = (timeout / 1000) + tp.tv_sec;
			ts.tv_nsec = ((timeout % 1000) * 1000000) + (tp.tv_usec * 1000);

			if (sem_timedwait(waitobj, &ts) == -1)
				return (errno == ETIMEDOUT) ? 1 : -1;
			else
				return 0;
		} else {
			return sem_wait(waitobj);
		}
	#endif
} 


//http://kcachegrind.sourceforge.net/html/Bugs.html
void _SignalWaitObject(WAITOBJ *waitobj) {
	#ifdef _WIN32
		SetEvent(*waitobj);
	#else
		sem_post(waitobj);
	#endif
}


void _SignalPollingWait(LPCONNPOOL cp) {
	#ifdef _WIN32
		//WSACancelBlockingCall();
		//WSAGetLastError();
		//sendto(alert_sck, "", sizeof(""), 0, cp->blah, sizeof(cp->blah));
	#else
		raise(SIGUSR1);
	#endif
}


/*void _ResetWaitObject(WAITOBJ waitobj) {
	#ifdef _WIN32
		ResetEvent(waitobj);
	#else
	#endif
}*/



int BindThreadToProcessor(pthread_t tid, int pnumber) {
#if defined(_WIN32)
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, 0, tid);
	if (!hThread)
		return 0;

	int success = SetThreadAffinityMask(hThread, 1 << pnumber) != 0;

	CloseHandle(hThread);
	return success;
#elif (defined(__FreeBSD__) && (__FreeBSD_version >= 702106)) \
	|| defined(__linux) || defined(linux)
	cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
	CPU_SET(pnumber, &cpuset);
	return pthread_setaffinity_np(tid, sizeof(cpuset), &cpuset) == 0;
#elif defined(__sun) || defined(sun)
	return processor_bind(P_LWPID, MAKE_LWPID_PTHREAD(tid), 
						pnumber, NULL) == 0;
#elif defined(_AIX)
	return bindprocessor(BINDTHREAD, (tid_t)tid, pnumber) == 0;
#elif defined(__hpux) || defined(hpux)
	pthread_spu_t answer;

	return pthread_processor_bind_np(PTHREAD_BIND_ADVISORY_NP,
									&answer, pnumber, tid) == 0;
#elif defined(__APPLE__)
	struct thread_affinity_policy tapol;
	
	thread_port_t threadport = pthread_mach_thread_np(tid);
	tapol.affinity_tag = pnumber + 1;
	return thread_policy_set(threadport, THREAD_AFFINITY_POLICY,
			(thread_policy_t)&tapol, THREAD_AFFINITY_POLICY_COUNT) == KERN_SUCCESS;
#else
	return 0;
#endif
}


#ifndef _WIN32

unsigned int gettick() {
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

#endif


void RandomFillBuf(unsigned char *buf, int len) {
	int i;

	for (i = 0; i != len / sizeof(int); i++)
		*(int *)(buf + i * sizeof(int)) = rand();
}


unsigned long RandomGenSecure() {
	unsigned long r;

	#ifdef _WIN32
		#if WINVER >= 0x501
			if (lpfnRtlGenRandom && lpfnRtlGenRandom(&r, sizeof(unsigned long)))
				return r;
			else
				return (GetTickCount() ^ GetCurrentProcessId());
		#else
				return (GetTickCount() ^ GetCurrentProcessId());
		#endif
	#else
		FILE *f;

		f = fopen("/dev/urandom", "rb");
		if (!f) {
			f = fopen("/dev/random", "rb");
			if (!f) {
				srand(time(NULL));
				return (unsigned long)rand();
			}
		}
		if (fread(&r, sizeof(unsigned long), 1, f) != 1) {
			fclose(f);
			srand(time(NULL));
			return (unsigned long)rand();
		}
		fclose(f);
		return r;
	#endif
}
