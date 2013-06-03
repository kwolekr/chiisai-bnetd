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

#ifndef CHAT_HEADER
#define CHAT_HEADER

///////////Chat [E]vent packet [ID]s
#define EID_SHOWUSER			 0x01
#define EID_JOIN				 0x02
#define EID_LEAVE				 0x03
#define EID_WHISPER				 0x04
#define EID_TALK				 0x05
#define EID_BROADCAST			 0x06
#define EID_CHANNEL				 0x07
#define EID_USERFLAGS			 0x09
#define EID_WHISPERSENT		 	 0x0A
#define EID_CHANNELFULL			 0x0D
#define EID_CHANNELDOESNOTEXIST  0x0E
#define EID_CHANNELRESTRICTED	 0x0F
#define EID_INFO				 0x12
#define EID_ERROR				 0x13
#define EID_EMOTE				 0x17

int ChatFloodOut(LPSESS sess, unsigned int len);
void SendTextToChannel(LPSESS sess, LPCHANNEL channel, const char *username, char *text, int eid);
void MultiLineSend(LPSESS sess, const char *username, const char *text, int eid);

#endif //CHAT_HEADER
