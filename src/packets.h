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

#ifndef PACKETS_HEADER
#define PACKETS_HEADER

typedef void (*PacketHandler)(char *data, LPSESS sess);

extern int pkhndindex[];
extern PacketHandler pkthandlers[];

void IgnorePacket(char *data, LPSESS sess);
void Parse0x50(char *data, LPSESS sess);
void Parse0x51(char *data, LPSESS sess);
void Parse0x52(char *data, LPSESS sess);
void Parse0x53(char *data, LPSESS sess);
void Parse0x54(char *data, LPSESS sess);
void Parse0x0A(char *data, LPSESS sess);
void Parse0x0C(char *data, LPSESS sess);
void Parse0x0E(char *data, LPSESS sess);
void Parse0x3A(char *data, LPSESS sess);
void Parse0x10(char *data, LPSESS sess);
void Send0x0F(LPSESS sess, int eid, int flags, const char *user, const char *text);
void Parse0x3D(char *data, LPSESS sess);
void Parse0x29(char *data, LPSESS sess);
void Parse0x0B(char *data, LPSESS sess);
void Parse0x14(char *data, LPSESS sess);
void SendUDP0x05Burst(LPSESS sess);
void Send0x25(LPSESS sess);
void Parse0x25(char *data, LPSESS sess);
void Parse0x59(char *data, LPSESS sess);

#endif //PACKETS_HEADER
