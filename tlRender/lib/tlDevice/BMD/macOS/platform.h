/* -LICENSE-START-
 ** Copyright (c) 2013 Blackmagic Design
 **
 ** Permission is hereby granted, free of charge, to any person or organization
 ** obtaining a copy of the software and accompanying documentation covered by
 ** this license (the "Software") to use, reproduce, display, distribute,
 ** execute, and transmit the Software, and to prepare derivative works of the
 ** Software, and to permit third-parties to whom the Software is furnished to
 ** do so, all subject to the following:
 **
 ** The copyright notices in the Software and this entire statement, including
 ** the above license grant, this restriction and the following disclaimer,
 ** must be included in all copies of the Software, in whole or in part, and
 ** all derivative works of the Software, unless such copies or derivative
 ** works are solely in the form of machine-executable object code generated by
 ** a source language processor.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 ** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 ** FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 ** SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 ** FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 ** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 ** DEALINGS IN THE SOFTWARE.
 ** -LICENSE-END-
 */

// Mac definitions

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <pthread.h>
#include <sys/time.h>
#include <string>
#include <stdio.h>
#include "DeckLinkAPI.h"
#include <list>

#define INT8_UNSIGNED uint8_t
#define INT32_UNSIGNED uint32_t
#define INT32_SIGNED int32_t
#define INT64_UNSIGNED uint64_t
#define INT32_SIGNED int32_t
#define INT64_SIGNED int64_t

#define BOOL bool

#define STRINGOBJ CFStringRef
#define STRINGPREFIX(x) CFSTR(x)
#define STRINGCOPY(x) CFStringCreateCopy(kCFAllocatorDefault, (x))
#define STRINGFREE(x) CFRelease(x)

#define MUTEX pthread_mutex_t
#define CONDITION pthread_cond_t

#define IID_IUnknown CFUUIDGetUUIDBytes(IUnknownUUID)

HRESULT Initialize();
HRESULT GetDeckLinkIterator(IDeckLinkIterator** deckLinkIterator);
HRESULT GetDeckLinkDiscoveryInstance(IDeckLinkDiscovery** deckLinkDiscovery);

void MutexInit(MUTEX* mutex);
void MutexLock(MUTEX* mutex);
void MutexUnlock(MUTEX* mutex);
void MutexDestroy(MUTEX* mutex);

// string helpers
void StringFromCharArray(STRINGOBJ* newStr, const char* charPtr);
void StringToCharArray(
    STRINGOBJ bmdStr, char* charArray, unsigned int arrayLength);
void StringToStdString(STRINGOBJ bmdStr, std::string& stdStr);

// atomic operators
INT32_SIGNED AtomicIncrement(volatile INT32_SIGNED* value);
INT32_SIGNED AtomicDecrement(volatile INT32_SIGNED* value);

bool operator==(const REFIID& lhs, const REFIID& rhs);
