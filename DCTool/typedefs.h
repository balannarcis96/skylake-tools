#pragma once

#define uint8 unsigned char

#define uint16 unsigned short

#define uint32 unsigned long

#define u_int64 unsigned long long

#define int8 char

#define int16 short

#define int32 long

#define _int64 long long

#define STRING const char*

#define ulong unsigned long

#define uint unsigned int

#define wchar uint16

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define degreesToRadians(angleDegrees) ((angleDegrees) * (M_PI / 180.0f))
#define radiansToDegrees(angleRadians) ((angleRadians) * (180.0f / M_PI))

#include <string.h>
#define strE(a,b) (strcmp(a,b) == 0)

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

#define __PI 3.14159265359

#define TO_RADIANS(x) (((x) * __PI) / 180)
#define TO_DEGREES(x) (((x) * 180) / __PI)
//
//typedef ULONG BOOL;
//typedef ULONGLONG ULONGLONG;
//typedef LONGLONG LONGLONG;

typedef char ANSICHAR;
typedef char SBYTE;
typedef short SWORD;
typedef unsigned long long QWORD;
typedef wchar_t				UNICHAR;	// A unicode character. normally a signed type.

typedef int					PTRINT;		// Integer large enough to hold a pointer.
typedef unsigned int		UPTRINT;	// Unsigned integer large enough to hold a pointer.

template< class T > inline T Align(const T Ptr, int Alignment)
{
	return (T)(((PTRINT)Ptr + Alignment - 1) & ~(Alignment - 1));
}

inline void appMemzero(void* Dest, int Count) { memset(Dest, 0, Count); }
inline void appMemcpy(void* Dest, const void* Src, int Count) { memcpy(Dest, Src, Count); }