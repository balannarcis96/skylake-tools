#ifndef UMATH_H
#define UMATH_H

#include <Windows.h>
#include <intrin.h>

#include <math.h>

//const FLOAT	SRandTemp = 1.f;
//extern INT GSRandSeed;
//
FORCEINLINE INT appTruncL(FLOAT F)
{
	return _mm_cvtt_ss2si(_mm_set_ss(F));
}
//
///**
//* Converts a float to an integer value with truncation towards zero.
//* @param F		Floating point value to convert
//* @return			Truncated integer value.
//*/
FORCEINLINE FLOAT appTruncFloatL(FLOAT F)
{
	return (FLOAT)appTruncL(F);
}
//
///**
//* Converts a float to the nearest integer. Rounds up when the fraction is .5
//* @param F		Floating point value to convert
//* @return		The nearest integer to 'F'.
//*/
//FORCEINLINE INT appRound(FLOAT F)
//{
//	// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
//	return _mm_cvt_ss2si(_mm_set_ss(F + F + 0.5f)) >> 1;
//}
//
///**
//* Converts a float to a less or equal integer.
//* @param F		Floating point value to convert
//* @return		An integer less or equal to 'F'.
//*/
//FORCEINLINE INT appFloor(FLOAT F)
//{
//	// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
//	return _mm_cvt_ss2si(_mm_set_ss(F + F - 0.5f)) >> 1;
//}
//
///**
//* Converts a float to a greater or equal integer.
//* @param F		Floating point value to convert
//* @return		An integer greater or equal to 'F'.
//*/
//FORCEINLINE INT appCeil(FLOAT F)
//{
//	// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
//	return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (F + F))) >> 1);
//}
//
//inline FLOAT appCopySign(FLOAT A, FLOAT B) { return _copysign(A, B); }
//inline FLOAT appExp(FLOAT Value) { return expf(Value); }
//inline FLOAT appLoge(FLOAT Value) { return logf(Value); }
//inline FLOAT appFmod(FLOAT Y, FLOAT X) { return fmodf(Y, X); }
extern FLOAT appSin(FLOAT Value);
//inline FLOAT appAsin(FLOAT Value) { return asinf((Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f)); }
//inline FLOAT appCos(FLOAT Value) { return cosf(Value); }
//inline FLOAT appAcos(FLOAT Value) { return acosf((Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f)); }
//inline FLOAT appTan(FLOAT Value) { return tanf(Value); }
//inline FLOAT appAtan(FLOAT Value) { return atanf(Value); }
//inline FLOAT appAtan2(FLOAT Y, FLOAT X) { return atan2f(Y, X); }
//inline FLOAT appSqrt(FLOAT F)
//{
//	// DB: crt's sqrt is ~60% faster than the below code and is defined for near-zero values.
//	return sqrt(F);
//#if 0
//	const FLOAT fZero = 0.0f;
//	const FLOAT fThree = 3.0f;
//	const FLOAT fOneHalf = 0.5f;
//	FLOAT temp;
//
//	__asm
//	{
//		movss	xmm1, [F]
//		rsqrtss xmm0, xmm1			// 1/sqrt estimate (12 bits)
//
//		// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
//		movss	xmm3, [fThree]
//		movss	xmm2, xmm0
//		mulss	xmm0, xmm1			// Y*X0
//		mulss	xmm0, xmm2			// Y*X0*X0
//		mulss	xmm2, [fOneHalf]		// 0.5*X0
//		subss	xmm3, xmm0			// 3-Y*X0*X0
//		mulss	xmm3, xmm2			// 0.5*X0*(3-Y*X0*X0)
//
//		movss	xmm4, [fZero]
//		cmpss	xmm4, xmm1, 4			// not equal
//
//		mulss	xmm3, xmm1			// sqrt(f) = f * 1/sqrt(f)
//
//		andps	xmm3, xmm4			// seet result to zero if input is zero
//
//		movss[temp], xmm3
//	}
//
//	return temp;
//#endif
//}
//inline FLOAT appPow(FLOAT A, FLOAT B) { return powf(A, B); }
//
//inline INT appRand() { return rand(); }
//inline void appRandInit(INT Seed) { srand(Seed); }
//inline FLOAT appFrand() { return rand() / (FLOAT)RAND_MAX; }
//inline void appSRandInit(INT Seed) { GSRandSeed = Seed; }
//inline FLOAT appFractional(FLOAT Value) { return Value - appTruncFloat(Value); }
//
//inline FLOAT appSRand()
//{
//	GSRandSeed = (GSRandSeed * 196314165) + 907633515;
//	//@todo fix type aliasing
//	FLOAT Result;
//	*(INT*)&Result = (*(INT*)&SRandTemp & 0xff800000) | (GSRandSeed & 0x007fffff);
//	return appFractional(Result);
//}
//FORCEINLINE FLOAT appInvSqrt(FLOAT F)
//{
//#if ENABLE_VECTORINTRINSICS
//	static const __m128 fThree = _mm_set_ss(3.0f);
//	static const __m128 fOneHalf = _mm_set_ss(0.5f);
//	__m128 Y0, X0, Temp;
//	FLOAT temp;
//
//	Y0 = _mm_set_ss(F);
//	X0 = _mm_rsqrt_ss(Y0);	// 1/sqrt estimate (12 bits)
//
//							// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
//	Temp = _mm_mul_ss(_mm_mul_ss(Y0, X0), X0);	// (Y*X0)*X0
//	Temp = _mm_sub_ss(fThree, Temp);				// (3-(Y*X0)*X0)
//	Temp = _mm_mul_ss(X0, Temp);					// X0*(3-(Y*X0)*X0)
//	Temp = _mm_mul_ss(fOneHalf, Temp);			// 0.5*X0*(3-(Y*X0)*X0)
//	_mm_store_ss(&temp, Temp);
//
//	return temp;
//
//#elif _WIN64
//	// NOTE: This is only really used when included from C++/CLI source files in 64-bit
//	// @todo win64: Use a more efficient implementation here!
//	return 1.0f / sqrtf(F);
//#else
//	// x86 ASM
//	const FLOAT fThree = 3.0f;
//	const FLOAT fOneHalf = 0.5f;
//	FLOAT temp;
//
//	__asm
//	{
//		movss	xmm1, [F]
//		rsqrtss	xmm0, xmm1			// 1/sqrt estimate (12 bits)
//
//									// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
//		movss	xmm3, [fThree]
//		movss	xmm2, xmm0
//		mulss	xmm0, xmm1			// Y*X0
//		mulss	xmm0, xmm2			// Y*X0*X0
//		mulss	xmm2, [fOneHalf]		// 0.5*X0
//		subss	xmm3, xmm0			// 3-Y*X0*X0
//		mulss	xmm3, xmm2			// 0.5*X0*(3-Y*X0*X0)
//		movss[temp], xmm3
//	}
//	return temp;
//#endif
//}
//inline FLOAT appInvSqrtEst(FLOAT F)
//{
//	return appInvSqrt(F);
//}
//
///**
//* Computes the base 2 logarithm for an integer value that is greater than 0.
//* The result is rounded down to the nearest integer.
//*
//* @param Value the value to compute the log of
//*/
//#pragma intrinsic( _BitScanReverse )
//inline DWORD appFloorLog2(DWORD Value)
//{
//	// Use BSR to return the log2 of the integer
//	DWORD Log2;
//	_BitScanReverse(&Log2, Value);
//	return Log2;
//}

/**
* Counts the number of leading zeros in the bit representation of the value
*
* @param Value the value to determine the number of leading zeros for
*
* @return the number of zeros before the first "on" bit
*/
//inline DWORD appCountLeadingZeros(DWORD Value)
//{
//	if (Value == 0) return 32;
//	return 31 - appFloorLog2(Value);
//}


extern DOUBLE GSecondsPerCycle;
extern ULONGLONG GNumTimingCodeCalls;


#define DEFINED_appSeconds 1
/**
* Returns time in seconds. Origin is arbitrary.
*
* @return time in seconds (arbitrary origin)
*/
inline DOUBLE appSeconds()
{
	LARGE_INTEGER Cycles;
	QueryPerformanceCounter(&Cycles);
	// Add big number to make bugs apparent where return value is being passed to FLOAT
	return Cycles.QuadPart * GSecondsPerCycle + 16777216.0;
}

#define DEFINED_appCycles 1
/**
* Current high resolution cycle counter. Origin is arbitrary.
*
* @return current value of high resolution cycle counter - origin is aribtrary
*/
inline DWORD appCycles()
{
#if !FINAL_RELEASE
	GNumTimingCodeCalls++;
#endif
	LARGE_INTEGER Cycles;
	QueryPerformanceCounter(&Cycles);
	return Cycles.QuadPart;
}

extern "C" void* __cdecl _alloca(size_t);
#define appAlloca(size) ((size==0) ? 0 : _alloca((size+7)&~7))

/**
* Enforces strict memory load/store ordering across the memory barrier call.
*/
FORCEINLINE void appMemoryBarrier()
{
#if defined _M_IX86 || _WIN64
	// Do nothing on x86; the spec requires load/store ordering even in the absence of a memory barrier.
#else
#error Unknown platform for appMemoryBarrier implementation.
#endif
}

#endif