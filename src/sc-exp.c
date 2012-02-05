/*
 * Copyright (c) 2012, Samuel Rivas <samuelrivas@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name the author nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <winscard.h>
#include <scl-log.h>
#include <scl-malloc.h>
#include <scl-assert.h>

#include "sc-exp.h"

/* Static Functions */

/*
 * Out params:
 *  * context: the smartcard context
 *  * card: the smartcard handle
 */
static void connectCard(SCARDCONTEXT *context, SCARDHANDLE *card) {

  LPSTR readers;
  DWORD readersLength, protocol;

  CHECK(SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, context));

  readersLength = 0;
  CHECK(SCardListReaders(*context, NULL, NULL, &readersLength));
  MALLOC(readers, (char *), (sizeof(char) * readersLength));
  CHECK(SCardListReaders(*context, NULL, readers, &readersLength));
  ASSERT(readers[0] != '\0');

  CHECK(SCardConnect(*context, &readers[0], SCARD_SHARE_SHARED,
                     SCARD_PROTOCOL_T0, card, &protocol));
  ASSERT(protocol == SCARD_PROTOCOL_T0);

  free(readers);
  INFO("T0 card connected");
}

static void disconnectCard(SCARDCONTEXT context, SCARDHANDLE card) {
  CHECK(SCardDisconnect(card, SCARD_UNPOWER_CARD));
  CHECK(SCardReleaseContext(context));
}

/*
 * Out params:
 * outString: The stringified output. It must be allocated by the caller (each
 *            input byte requires 3 output bytes, plus the end null
 */
static void stringifyAtr(const BYTE *atr, DWORD len, char *outString) {

  register int i;
  char *p = outString;

  for (i=0, p = outString; i < len; i++, p += 3) {
    snprintf(p, 4, "%02X ", atr[i]);
  }
  p[-1] = '\0';
}

/*
 * Out params:
 * atr: A pointer to ATR bytes. It must be allocated with at least MAX_ATR_SIZE
 *      bytes
 * atrLen: The amount of bytes in the ATR
 */
static void getAtr(SCARDHANDLE card, BYTE *atr, DWORD *atrLen) {

  DWORD protocol, readerNameLen, cardState;
  char readerName[MAX_READERNAME];

  *atrLen = sizeof(BYTE[MAX_ATR_SIZE]);
  readerNameLen = sizeof(readerName);
  CHECK(SCardStatus(card, readerName, &readerNameLen, &cardState, &protocol,
                    atr, atrLen));
}

/* Public Functions */
int main(void) {

  SCARDCONTEXT context;
  SCARDHANDLE card;
  DWORD atrLen;
  BYTE atr[MAX_ATR_SIZE];
  char atrString[MAX_ATR_SIZE * 3 + 1];

  connectCard(&context, &card);

  getAtr(card, atr, &atrLen);
  stringifyAtr(atr, atrLen, atrString);
  INFO("ATR: %s", atrString);

  disconnectCard(context, card);

  return EXIT_SUCCESS;
}
