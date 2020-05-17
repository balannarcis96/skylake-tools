#pragma once 

#include <fvec.h>

#include "SMathSSE.h"

// Constants.
#undef  PI
#define PI 					(3.1415926535897932f)
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536f)
#define INDEX_NONE			-1 

// Aux constants.
#define INV_PI				(0.31830988618f)
#define HALF_PI				(1.57079632679f)

// Magic numbers for numerical precision.
#define DELTA				(0.00001f)

#define RadToDeg			(180.0f / PI)
#define DegToRad			(PI / 180.0f)
#define RadToUnit			(32768.0f / PI)
#define UnitToRad			(PI / 32768.0f)
#define DegToUnit			(32768.0f / 180.0f)
#define UnitToDeg			(180.0f / 32768.0f)

/**
* Convenience type for referring to axis by name instead of number.
*/
enum EAxis
{
	AXIS_None = 0,
	AXIS_X = 1,
	AXIS_Y = 2,
	AXIS_Z = 4,
	AXIS_XY = AXIS_X | AXIS_Y,
	AXIS_XZ = AXIS_X | AXIS_Z,
	AXIS_YZ = AXIS_Y | AXIS_Z,
	AXIS_XYZ = AXIS_X | AXIS_Y | AXIS_Z,
	//alias over Axis YZ since it isn't used when the z-rotation widget is being used
	AXIS_ZROTATION = AXIS_YZ
};

template<class T> FORCEINLINE T Abs(const T A)
{
	return (A >= (T)0) ? A : -A;
}
template<class T> FORCEINLINE T Sgn(const T A)
{
	return (A > 0) ? 1 : ((A < 0) ? -1 : 0);
}
template<class T> FORCEINLINE T Max(const T A, const T B)
{
	return (A >= B) ? A : B;
}
template<class T> FORCEINLINE T Min(const T A, const T B)
{
	return (A <= B) ? A : B;
}
template<class T> FORCEINLINE T Max3(const T A, const T B, const T C)
{
	return Max(Max(A, B), C);
}
template<class T> FORCEINLINE T Min3(const T A, const T B, const T C)
{
	return Min(Min(A, B), C);
}
template<class T> FORCEINLINE T Square(const T A)
{
	return A * A;
}
template<class T> FORCEINLINE T Clamp(const T X, const T Min, const T Max)
{
	return X < Min ? Min : X < Max ? X : Max;
}

FORCEINLINE float Abs(const float A)
{
	return fabsf(A);
}

/*----------------------------------------------------------------------------
Math functions.
----------------------------------------------------------------------------*/

const float	SRandTemp = 1.f;
extern INT GSRandSeed;

/**
* Converts a float to an integer with truncation towards zero.
* @param F		floating point value to convert
* @return		Truncated integer.
*/
FORCEINLINE INT appTrunc(float F)
{
	return _mm_cvtt_ss2si(_mm_set_ss(F));
}

/**
* Converts a float to an integer value with truncation towards zero.
* @param F		floating point value to convert
* @return			Truncated integer value.
*/
FORCEINLINE float appTruncfloat(float F)
{
	return (float)appTrunc(F);
}

/**
* Converts a float to the nearest integer. Rounds up when the fraction is .5
* @param F		floating point value to convert
* @return		The nearest integer to 'F'.
*/
FORCEINLINE INT appRound(float F)
{
	// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
	return _mm_cvt_ss2si(_mm_set_ss(F + F + 0.5f)) >> 1;
}

/**
* Converts a float to a less or equal integer.
* @param F		floating point value to convert
* @return		An integer less or equal to 'F'.
*/
FORCEINLINE INT appFloor(float F)
{
	// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
	return _mm_cvt_ss2si(_mm_set_ss(F + F - 0.5f)) >> 1;
}

/**
* Converts a float to a greater or equal integer.
* @param F		floating point value to convert
* @return		An integer greater or equal to 'F'.
*/
FORCEINLINE INT appCeil(float F)
{
	// Note: the x2 is to workaround the rounding-to-nearest-even-number issue when the fraction is .5
	return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (F + F))) >> 1);
}

FORCEINLINE float appCopySign(float A, float B) { return std::copysign(A, B); }
FORCEINLINE float appExp(float Value) { return expf(Value); }
FORCEINLINE float appLoge(float Value) { return logf(Value); }
FORCEINLINE float appFmod(float Y, float X) { return fmodf(Y, X); }
FORCEINLINE float appSin(float Value) { return sinf(Value); }
FORCEINLINE float appAsin(float Value) { return asinf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
FORCEINLINE float appCos(float Value) { return cosf(Value); }
FORCEINLINE float appAcos(float Value) { return acosf((Value < -1.f) ? -1.f : ((Value < 1.f) ? Value : 1.f)); }
FORCEINLINE float appTan(float Value) { return tanf(Value); }
FORCEINLINE float appAtan(float Value) { return atanf(Value); }
FORCEINLINE float appAtan2(float Y, float X) { return atan2f(Y, X); }
FORCEINLINE float appSqrt(float Value);
FORCEINLINE float appPow(float A, float B) { return powf(A, B); }
FORCEINLINE BOOL appIsNaN(float A) { return _isnan(A) != 0; }
FORCEINLINE BOOL appIsFinite(float A) { return _finite(A); }
FORCEINLINE INT appRand() { return rand(); }
FORCEINLINE void appRandInit(INT Seed) { srand(Seed); }
FORCEINLINE float appFrand() { return rand() / (float)RAND_MAX; }
FORCEINLINE void appSRandInit(INT Seed) { GSRandSeed = Seed; }
FORCEINLINE float appFractional(float Value) { return Value - appTruncfloat(Value); }

inline float appSRand()
{
	GSRandSeed = (GSRandSeed * 196314165) + 907633515;
	//@todo fix type aliasing
	float Result;
	*(INT*)&Result = (*(INT*)&SRandTemp & 0xff800000) | (GSRandSeed & 0x007fffff);
	return appFractional(Result);
}

//
// MSM: Fast float inverse square root using SSE.
// Accurate to within 1 LSB.
//
FORCEINLINE float appInvSqrt(float F)
{
	//#if ENABLE_VECTORINTRINSICS
	//	static const __m128 fThree = _mm_set_ss(3.0f);
	//	static const __m128 fOneHalf = _mm_set_ss(0.5f);
	//	__m128 Y0, X0, Temp;
	//	float temp;
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
		// NOTE: This is only really used when included from C++/CLI source files in 64-bit
		// @todo win64: Use a more efficient implementation here!
	return 1.0f / sqrtf(F);
	//#else
	//	// x86 ASM
	//	const float fThree = 3.0f;
	//	const float fOneHalf = 0.5f;
	//	float temp;
	//
	//	__asm
	//	{
	//		movss	xmm1, [F]
	//		rsqrtss	xmm0, xmm1			// 1/sqrt estimate (12 bits)
	//
	//									// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
	//									movss	xmm3, [fThree]
	//									movss	xmm2, xmm0
	//									mulss	xmm0, xmm1			// Y*X0
	//									mulss	xmm0, xmm2			// Y*X0*X0
	//									mulss	xmm2, [fOneHalf]		// 0.5*X0
	//									subss	xmm3, xmm0			// 3-Y*X0*X0
	//									mulss	xmm3, xmm2			// 0.5*X0*(3-Y*X0*X0)
	//									movss[temp], xmm3
	//	}
	//	return temp;
	//#endif
}

inline float appInvSqrtEst(float F)
{
	return appInvSqrt(F);
}

//
// MSM: Fast float square root using SSE.
// Accurate to within 1 LSB.
//
inline float appSqrt(float F)
{
	// DB: crt's sqrt is ~60% faster than the below code and is defined for near-zero values.
	return sqrt(F);
#if 0
	const float fZero = 0.0f;
	const float fThree = 3.0f;
	const float fOneHalf = 0.5f;
	float temp;

	__asm
	{
		movss	xmm1, [F]
		rsqrtss xmm0, xmm1			// 1/sqrt estimate (12 bits)

									// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
									movss	xmm3, [fThree]
									movss	xmm2, xmm0
									mulss	xmm0, xmm1			// Y*X0
									mulss	xmm0, xmm2			// Y*X0*X0
									mulss	xmm2, [fOneHalf]		// 0.5*X0
									subss	xmm3, xmm0			// 3-Y*X0*X0
									mulss	xmm3, xmm2			// 0.5*X0*(3-Y*X0*X0)

									movss	xmm4, [fZero]
									cmpss	xmm4, xmm1, 4			// not equal

									mulss	xmm3, xmm1			// sqrt(f) = f * 1/sqrt(f)

									andps	xmm3, xmm4			// seet result to zero if input is zero

									movss[temp], xmm3
	}

	return temp;
#endif
}

/**
* Computes the base 2 logarithm for an integer value that is greater than 0.
* The result is rounded down to the nearest integer.
*
* @param Value the value to compute the log of
*/
#pragma intrinsic( _BitScanReverse )
inline ULONG appFloorLog2(ULONG Value)
{
	// Use BSR to return the log2 of the integer
	ULONG Log2;
	_BitScanReverse(&Log2, Value);
	return Log2;
}

/**
* Counts the number of leading zeros in the bit representation of the value
*
* @param Value the value to determine the number of leading zeros for
*
* @return the number of zeros before the first "on" bit
*/
inline ULONG appCountLeadingZeros(ULONG Value)
{
	if (Value == 0) return 32;
	return 31 - appFloorLog2(Value);
}

FORCEINLINE float appFloatSelect(float Comparand, float ValueGEZero, float ValueLTZero)
{
	return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
}

template< class T, class U > FORCEINLINE T Lerp(const T& A, const T& B, const U& Alpha)
{
	return (T)(A + Alpha * (B - A));
}

template< class T, class U > FORCEINLINE T BiLerp(const T& P00, const T& P10, const T& P01, const T& P11, const U& FracX, const U& FracY)
{
	return Lerp(
		Lerp(P00, P10, FracX),
		Lerp(P01, P11, FracX),
		FracY
		);
}

// P - end points
// T - tangent directions at end points
// Alpha - distance along spline
template< class T, class U > FORCEINLINE T CubicInterp(const T& P0, const T& T0, const T& P1, const T& T1, const U& A)
{
	const float A2 = A * A;
	const float A3 = A2 * A;

	return (T)(((2 * A3) - (3 * A2) + 1) * P0) + ((A3 - (2 * A2) + A) * T0) + ((A3 - A2) * T1) + (((-2 * A3) + (3 * A2)) * P1);
}

template< class T, class U > FORCEINLINE T CubicInterpDerivative(const T& P0, const T& T0, const T& P1, const T& T1, const U& A)
{
	T a = 6.f * P0 + 3.f * T0 + 3.f * T1 - 6.f * P1;
	T b = -6.f * P0 - 4.f * T0 - 2.f * T1 + 6.f * P1;
	T c = T0;

	const float A2 = A * A;

	return (a * A2) + (b * A) + c;
}

template< class T, class U > FORCEINLINE T CubicInterpSecondDerivative(const T& P0, const T& T0, const T& P1, const T& T1, const U& A)
{
	T a = 12.f * P0 + 6.f * T0 + 6.f * T1 - 12.f * P1;
	T b = -6.f * P0 - 4.f * T0 - 2.f * T1 + 6.f * P1;

	return (a * A) + b;
}

/** Interpolate between A and B, applying an ease in/out function.  Exp controls the degree of the curve. */
template< class T > FORCEINLINE T InterpEaseInOut(const T& A, const T& B, float Alpha, float Exp)
{
	float const ModifiedAlpha = (Alpha < 0.5f) ?
		0.5f * appPow(2.f * Alpha, Exp) :
		1.f - 0.5f * appPow(2.f * (1.f - Alpha), Exp);

	return Lerp<T>(A, B, ModifiedAlpha);
}

template<> FORCEINLINE ULONG Min(const ULONG A, const ULONG B)
{
	// Negative if B is less than A (i.e. the high bit will be set)
	ULONG Delta = B - A;
	// Relies on sign bit rotating in
	ULONG Mask = static_cast<INT>(Delta) >> 31;
	ULONG Result = A + (Delta & Mask);

	return Result;
}

FORCEINLINE float LerpF(float a, float b, float t)
{
	return a + ((b - a) * t);
}

/**
* Add to a word angle, constraining it within a min (not to cross)
* and a max (not to cross).  Accounts for funkyness of word angles.
* Assumes that angle is initially in the desired range.
*/
inline INT16 SAddAngleConfined(INT Angle, INT Delta, INT MinThresh, INT MaxThresh);

struct SGlobalMath {
	// Constants.
	enum { ANGLE_SHIFT = 2 };		// Bits to right-shift to get lookup value.
	enum { ANGLE_BITS = 14 };		// Number of valid bits in angles.
	enum { NUM_ANGLES = 16384 }; 	// Number of angles that are in lookup table.
	enum { ANGLE_MASK = (((1 << ANGLE_BITS) - 1) << (16 - ANGLE_BITS)) };

	// Basic math functions.
	FORCEINLINE float SinTab(int i) const
	{
		return TrigFLOAT[((i >> ANGLE_SHIFT) & (NUM_ANGLES - 1))];
	}
	FORCEINLINE float CosTab(int i) const
	{
		return TrigFLOAT[(((i + 16384) >> ANGLE_SHIFT) & (NUM_ANGLES - 1))];
	}
	float SinFloat(float F) const
	{
		return SinTab(appTrunc((F * 65536.f) / (2.f * PI)));
	}
	float CosFloat(float F) const
	{
		return CosTab(appTrunc((F * 65536.f) / (2.f * PI)));
	}

	// Constructor.
	SGlobalMath();

private:
	// Tables.
	float  TrigFLOAT[NUM_ANGLES];
};
extern SGlobalMath g_SGlobalMath;

inline INT ReduceAngle(INT Angle);

/** Find the smallest angle between two headings (in radians) */
inline float FindDeltaAngle(float A1, float A2);

/**
* Fast 32-bit float evaluations.
* Warning: likely not portable, and useful on Pentium class processors only.
*/
inline BOOL IsNegativeFloat(const float& F1)
{
	return ((*(ULONG*)&F1) >= (ULONG)0x80000000); // Detects sign bit.
}

inline float RangeByteToFloat(UINT8 A)
{
	if (A < 128)
	{
		return (A - 128.f) / 128.f;
	}
	else
	{
		return (A - 128.f) / 127.f;
	}
}

inline UINT8 FloatToRangeByte(float A)
{
	return appRound(128.f + appFloatSelect(A, Min(127 * A, 127.f), Max(128 * A, -128.f)));
}

/**
*	Checks if two floating point numbers are nearly equal.
*	@param A				First number to compare
*	@param B				Second number to compare
*	@param ErrorTolerance	Maximum allowed difference for considering them as 'nearly equal'
*	@return					TRUE if A and B are nearly equal
**/
FORCEINLINE BOOL appIsNearlyEqual(float A, float B, float ErrorTolerance = SMALL_NUMBER)
{
	return Abs<float>(A - B) < ErrorTolerance;
}

/**
*	Checks if two floating point numbers are nearly equal.
*	@param A				First number to compare
*	@param B				Second number to compare
*	@param ErrorTolerance	Maximum allowed difference for considering them as 'nearly equal'
*	@return					TRUE if A and B are nearly equal
**/
FORCEINLINE BOOL appIsNearlyEqual(double A, double B, double ErrorTolerance = SMALL_NUMBER)
{
	return Abs<double>(A - B) < ErrorTolerance;
}

/**
*	Checks if a floating point number is nearly zero.
*	@param Value			Number to compare
*	@param ErrorTolerance	Maximum allowed difference for considering Value as 'nearly zero'
*	@return					TRUE if Value is nearly zero
**/
FORCEINLINE BOOL appIsNearlyZero(float Value, float ErrorTolerance = SMALL_NUMBER)
{
	return Abs<float>(Value) < ErrorTolerance;
}

/**
*	Checks if a floating point number is nearly zero.
*	@param Value			Number to compare
*	@param ErrorTolerance	Maximum allowed difference for considering Value as 'nearly zero'
*	@return					TRUE if Value is nearly zero
**/
FORCEINLINE BOOL appIsNearlyZero(double Value, double ErrorTolerance = SMALL_NUMBER)
{
	return Abs<double>(Value) < ErrorTolerance;
}

/**
*	Checks whether a number is a power of two.
*	@param Value	Number to check
*	@return			TRUE if Value is a power of two
*/
FORCEINLINE BOOL appIsPowerOfTwo(ULONG Value)
{
	return ((Value & (Value - 1)) == 0);
}

/**
* Snaps a value to the nearest grid multiple.
*/
inline float SSnap(float Location, float Grid)
{
	if (Grid == 0.f) return Location;
	else
	{
		return (float)(appFloor((float)((Location + 0.5 * Grid) / Grid)) * Grid);
	}
}

struct SAngle {
	//Zero angle.	
	static const SAngle Zero;

	INT16 angle;

	SAngle() { angle = 0; }
	SAngle(INT16 _angle) :angle(_angle) {}

	FORCEINLINE void SetValue(const UINT16 val) noexcept {
		angle = val;
	}

	FORCEINLINE INT16 Add(const SAngle& other) noexcept {
		return (angle += other.angle);
	}
	FORCEINLINE INT16 Sub(const SAngle& other) noexcept {
		return (angle -= other.angle);
	}

	FORCEINLINE INT16 FromRad(float rad) noexcept {
		return (angle = (INT16)(rad * RadToUnit));
	}
	FORCEINLINE INT16 FromDeg(float deg) noexcept {
		return (angle = (INT16)(deg * DegToUnit));
	}
	FORCEINLINE float ToRad() const noexcept {
		return (float)(angle * UnitToRad);
	}
	FORCEINLINE float ToDeg(float rad) const noexcept {
		return (float)(angle * UnitToDeg);
	}
};

struct SIntPoint {
	INT X, Y;
	SIntPoint() {}
	SIntPoint(INT InX, INT InY)
		: X(InX)
		, Y(InY) {}
	SIntPoint(const float Vector[3])
		: X((INT)Vector[0])
		, Y((INT)Vector[1]) {}

	static SIntPoint ZeroValue() {
		return SIntPoint(0, 0);
	}
	static SIntPoint NoneValue() {
		return SIntPoint(INDEX_NONE, INDEX_NONE);
	}
	const INT& operator()(INT i) const {
		return (&X)[i];
	}
	INT& operator()(INT i) {
		return (&X)[i];
	}
	static INT Num() {
		return 2;
	}
	BOOL operator==(const SIntPoint& Other) const {
		return X == Other.X && Y == Other.Y;
	}
	BOOL operator!=(const SIntPoint& Other) const {
		return X != Other.X || Y != Other.Y;
	}
	SIntPoint& operator*=(INT Scale) {
		X *= Scale;
		Y *= Scale;
		return *this;
	}
	SIntPoint& operator+=(const SIntPoint& Other) {
		X += Other.X;
		Y += Other.Y;
		return *this;
	}
	SIntPoint& operator-=(const SIntPoint& Other) {
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}
	SIntPoint& operator+=(INT Value) {
		X += Value;
		Y += Value;
		return *this;
	}
	SIntPoint& operator-=(INT Value) {
		X -= Value;
		Y -= Value;
		return *this;
	}
	SIntPoint operator*(INT Scale) const {
		return SIntPoint(*this) *= Scale;
	}
	SIntPoint operator+(const SIntPoint& Other) const {
		return SIntPoint(*this) += Other;
	}
	SIntPoint operator-(const SIntPoint& Other) const {
		return SIntPoint(*this) -= Other;
	}
	INT Size() const {
		return INT(appSqrt(float(X * X + Y * Y)));
	}
};

struct SIntRect
{
	SIntPoint Min, Max;
	SIntRect()
	{}
	SIntRect(INT X0, INT Y0, INT X1, INT Y1)
		: Min(X0, Y0)
		, Max(X1, Y1)
	{}
	SIntRect(SIntPoint InMin, SIntPoint InMax)
		: Min(InMin)
		, Max(InMax)
	{}

	const SIntPoint& operator()(INT i) const
	{
		return (&Min)[i];
	}
	SIntPoint& operator()(INT i)
	{
		return (&Min)[i];
	}
	static INT Num()
	{
		return 2;
	}
	BOOL operator==(const SIntRect& Other) const
	{
		return Min == Other.Min && Max == Other.Max;
	}
	BOOL operator!=(const SIntRect& Other) const
	{
		return Min != Other.Min || Max != Other.Max;
	}
	SIntRect Right(INT InWidth) const
	{
		return SIntRect(::Max(Min.X, Max.X - InWidth), Min.Y, Max.X, Max.Y);
	}
	SIntRect Bottom(INT InHeight) const
	{
		return SIntRect(Min.X, ::Max(Min.Y, Max.Y - InHeight), Max.X, Max.Y);
	}
	SIntPoint Size() const
	{
		return SIntPoint(Max.X - Min.X, Max.Y - Min.Y);
	}
	INT Width() const
	{
		return Max.X - Min.X;
	}
	INT Height() const
	{
		return Max.Y - Min.Y;
	}
	SIntRect& operator*=(INT Scale)
	{
		Min *= Scale;
		Max *= Scale;
		return *this;
	}
	SIntRect& operator+=(const SIntPoint& P)
	{
		Min += P;
		Max += P;
		return *this;
	}
	SIntRect& operator-=(const SIntPoint& P)
	{
		Min -= P;
		Max -= P;
		return *this;
	}
	SIntRect operator*(INT Scale) const
	{
		return SIntRect(Min * Scale, Max * Scale);
	}
	SIntRect operator+(const SIntPoint& P) const
	{
		return SIntRect(Min + P, Max + P);
	}
	SIntRect operator-(const SIntPoint& P) const
	{
		return SIntRect(Min - P, Max - P);
	}
	SIntRect operator+(const SIntRect& R) const
	{
		return SIntRect(Min + R.Min, Max + R.Max);
	}
	SIntRect operator-(const SIntRect& R) const
	{
		return SIntRect(Min - R.Min, Max - R.Max);
	}
	SIntRect Inner(SIntPoint P) const
	{
		return SIntRect(Min + P, Max - P);
	}
	BOOL Contains(SIntPoint P) const
	{
		return P.X >= Min.X && P.X < Max.X&& P.Y >= Min.Y && P.Y < Max.Y;
	}
	INT Area() const
	{
		return (Max.X - Min.X) * (Max.Y - Min.Y);
	}
	void GetCenterAndExtents(SIntPoint& Center, SIntPoint& Extent) const
	{
		Extent.X = (Max.X - Min.X) / 2;
		Extent.Y = (Max.Y - Min.Y) / 2;

		Center.X = Min.X + Extent.X;
		Center.Y = Min.Y + Extent.Y;
	}
	void Clip(const SIntRect& R)
	{
		Min.X = ::Max<INT>(Min.X, R.Min.X);
		Min.Y = ::Max<INT>(Min.Y, R.Min.Y);
		Max.X = ::Min<INT>(Max.X, R.Max.X);
		Max.Y = ::Min<INT>(Max.Y, R.Max.Y);

		// Adjust to zero area if the rects don't overlap.
		Max.X = ::Max<INT>(Min.X, Max.X);
		Max.Y = ::Max<INT>(Min.Y, Max.Y);
	}
	void SetWidth(INT width) noexcept {
		Max.X = Min.X + width;
	}
	void SetHeight(INT height) noexcept {
		Max.Y = Min.Y + height;
	}
	INT X() const noexcept {
		return Min.X;
	}
	INT Y() const noexcept {
		return Min.Y;
	}
	void ExpandFromCenter(INT Value) noexcept {
		Max += Value;
		Min -= Value;
	}
};

struct SVector2D {
	float
		X,
		Y;

	// Constructors.
	FORCEINLINE SVector2D()
	{}
	FORCEINLINE SVector2D(float InX, float InY)
		: X(InX), Y(InY)
	{}

	// Binary math operators.
	FORCEINLINE SVector2D operator+(const SVector2D& V) const
	{
		return SVector2D(X + V.X, Y + V.Y);
	}
	FORCEINLINE SVector2D operator-(const SVector2D& V) const
	{
		return SVector2D(X - V.X, Y - V.Y);
	}
	FORCEINLINE SVector2D operator*(float Scale) const
	{
		return SVector2D(X * Scale, Y * Scale);
	}
	SVector2D operator/(float Scale) const
	{
		const float RScale = 1.f / Scale;
		return SVector2D(X * RScale, Y * RScale);
	}
	FORCEINLINE SVector2D operator*(const SVector2D& V) const
	{
		return SVector2D(X * V.X, Y * V.Y);
	}
	FORCEINLINE float operator|(const SVector2D& V) const
	{
		return X * V.X + Y * V.Y;
	}
	FORCEINLINE float operator^(const SVector2D& V) const
	{
		return X * V.Y - Y * V.X;
	}

	// Binary comparison operators.
	BOOL operator==(const SVector2D& V) const
	{
		return X == V.X && Y == V.Y;
	}
	BOOL operator!=(const SVector2D& V) const
	{
		return X != V.X || Y != V.Y;
	}
	BOOL operator<(const SVector2D& Other) const
	{
		return X < Other.X&& Y < Other.Y;
	}
	BOOL operator>(const SVector2D& Other) const
	{
		return X > Other.X && Y > Other.Y;
	}
	BOOL operator<=(const SVector2D& Other) const
	{
		return X <= Other.X && Y <= Other.Y;
	}
	BOOL operator>=(const SVector2D& Other) const
	{
		return X >= Other.X && Y >= Other.Y;
	}
	// Error-tolerant comparison.
	BOOL Equals(const SVector2D& V, float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return Abs(X - V.X) < Tolerance && Abs(Y - V.Y) < Tolerance;
	}

	// Unary operators.
	FORCEINLINE SVector2D operator-() const
	{
		return SVector2D(-X, -Y);
	}
	// Assignment operators.
	FORCEINLINE SVector2D operator+=(const SVector2D& V)
	{
		X += V.X; Y += V.Y;
		return *this;
	}
	FORCEINLINE SVector2D operator-=(const SVector2D& V)
	{
		X -= V.X; Y -= V.Y;
		return *this;
	}
	FORCEINLINE SVector2D operator*=(float Scale)
	{
		X *= Scale; Y *= Scale;
		return *this;
	}
	SVector2D operator/=(float V)
	{
		const float RV = 1.f / V;
		X *= RV; Y *= RV;
		return *this;
	}
	SVector2D operator*=(const SVector2D& V)
	{
		X *= V.X; Y *= V.Y;
		return *this;
	}
	SVector2D operator/=(const SVector2D& V)
	{
		X /= V.X; Y /= V.Y;
		return *this;
	}
	float& operator[](INT i)
	{
		if (i == 0)	return X;
		else			return Y;
	}
	float operator[](INT i) const
	{
		return ((i == 0) ? X : Y);
	}
	// Simple functions.
	void Set(float InX, float InY)
	{
		X = InX;
		Y = InY;
	}
	float GetMax() const
	{
		return Max(X, Y);
	}
	float GetAbsMax() const
	{
		return Max(Abs(X), Abs(Y));
	}
	float GetMin() const
	{
		return Min(X, Y);
	}
	float Size() const
	{
		return appSqrt(X * X + Y * Y);
	}
	float SizeSquared() const
	{
		return X * X + Y * Y;
	}

	SVector2D SafeNormal(float Tolerance = SMALL_NUMBER) const
	{
		const float SquareSum = X * X + Y * Y;
		if (SquareSum > Tolerance)
		{
			const float Scale = appInvSqrt(SquareSum);
			return SVector2D(X * Scale, Y * Scale);
		}
		return SVector2D(0.f, 0.f);
	}
	void Normalize(float Tolerance = SMALL_NUMBER)
	{
		const float SquareSum = X * X + Y * Y;
		if (SquareSum > Tolerance)
		{
			const float Scale = appInvSqrt(SquareSum);
			X *= Scale;
			Y *= Scale;
			return;
		}
		X = 0.0f;
		Y = 0.0f;
	}
	int IsNearlyZero(float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return	Abs(X) < Tolerance
			&& Abs(Y) < Tolerance;
	}
	BOOL IsZero() const
	{
		return X == 0.f && Y == 0.f;
	}
	float& Component(INT Index)
	{
		return (&X)[Index];
	}

	/* Lerp Member Function
	Calculates the lerp for the vector and vector supplied using unit interval t */
	void Lerp(SVector2D& b, float t)
	{
		X = ((b.X - X) * t);
		Y = ((b.Y - Y) * t);
	}

	static SVector2D ZeroVector;
};

struct SVector {
	//Zero vector.
	static SVector	const Zero;

	//Unit vector.
	static SVector	const Unit;

	float
		X, Y, Z;

	// Constructors.
	SVector() : X(0.0f), Y(0.0f), Z(0.0f)
	{}
	explicit  SVector(float InF)
		: X(InF), Y(InF), Z(InF)
	{}
	SVector(float InX, float InY, float InZ)
		: X(InX), Y(InY), Z(InZ)
	{}
	explicit  SVector(const float* vec)
		: X(vec[0]), Y(vec[1]), Z(vec[2])
	{}
	// Binary math operators.
	FORCEINLINE SVector operator^(const SVector& V) const
	{
		return SVector
		(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
			);
	}

	FORCEINLINE float operator|(const SVector& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z;
	}
	FORCEINLINE SVector operator+(const SVector& V) const
	{
		return SVector(X + V.X, Y + V.Y, Z + V.Z);
	}
	FORCEINLINE SVector operator-(const SVector& V) const
	{
		return SVector(X - V.X, Y - V.Y, Z - V.Z);
	}
	FORCEINLINE SVector operator-(float Bias) const
	{
		return SVector(X - Bias, Y - Bias, Z - Bias);
	}
	FORCEINLINE SVector operator+(float Bias) const
	{
		return SVector(X + Bias, Y + Bias, Z + Bias);
	}
	FORCEINLINE SVector operator*(float Scale) const
	{
		return SVector(X * Scale, Y * Scale, Z * Scale);
	}
	SVector operator/(float Scale) const
	{
		const float RScale = 1.f / Scale;
		return SVector(X * RScale, Y * RScale, Z * RScale);
	}
	FORCEINLINE SVector operator*(const SVector& V) const
	{
		return SVector(X * V.X, Y * V.Y, Z * V.Z);
	}
	FORCEINLINE SVector operator/(const SVector& V) const
	{
		return SVector(X / V.X, Y / V.Y, Z / V.Z);
	}

	// Binary comparison operators.
	BOOL operator==(const SVector& V) const
	{
		return X == V.X && Y == V.Y && Z == V.Z;
	}
	BOOL operator!=(const SVector& V) const
	{
		return X != V.X || Y != V.Y || Z != V.Z;
	}

	// Error-tolerant comparison.
	BOOL Equals(const SVector& V, float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return Abs(X - V.X) < Tolerance && Abs(Y - V.Y) < Tolerance && Abs(Z - V.Z) < Tolerance;
	}

	/** Checks whether all components of the vector are the same, within a tolerance. */
	BOOL AllComponentsEqual(float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return Abs(X - Y) < Tolerance && Abs(X - Z) < Tolerance && Abs(Y - Z) < Tolerance;
	}

	// Unary operators.
	FORCEINLINE SVector operator-() const
	{
		return SVector(-X, -Y, -Z);
	}

	// Assignment operators.
	FORCEINLINE SVector operator+=(const SVector& V)
	{
		X += V.X; Y += V.Y; Z += V.Z;
		return *this;
	}
	FORCEINLINE SVector operator-=(const SVector& V)
	{
		X -= V.X; Y -= V.Y; Z -= V.Z;
		return *this;
	}
	FORCEINLINE SVector operator*=(float Scale)
	{
		X *= Scale; Y *= Scale; Z *= Scale;
		return *this;
	}
	SVector operator/=(float V)
	{
		const float RV = 1.f / V;
		X *= RV; Y *= RV; Z *= RV;
		return *this;
	}
	SVector operator*=(const SVector& V)
	{
		X *= V.X; Y *= V.Y; Z *= V.Z;
		return *this;
	}
	SVector operator/=(const SVector& V)
	{
		X /= V.X; Y /= V.Y; Z /= V.Z;
		return *this;
	}
	float& operator[](INT i)
	{
		if (i == 0)		return X;
		else if (i == 1)	return Y;
		else				return Z;
	}
	float operator[](INT i)const
	{
		if (i == 0)		return X;
		else if (i == 1)	return Y;
		else				return Z;
	}
	// Simple functions.
	void Set(float InX, float InY, float InZ)
	{
		X = InX;
		Y = InY;
		Z = InZ;
	}
	float GetMax() const
	{
		return Max(Max(X, Y), Z);
	}
	float GetAbsMax() const
	{
		return Max(Max(Abs(X), Abs(Y)), Abs(Z));
	}
	float GetMin() const
	{
		return Min(Min(X, Y), Z);
	}
	FORCEINLINE float Size() const
	{
		return appSqrt(X * X + Y * Y + Z * Z);
	}
	float SizeSquared() const
	{
		return X * X + Y * Y + Z * Z;
	}
	float Size2D() const
	{
		return appSqrt(X * X + Y * Y);
	}
	float SizeSquared2D() const
	{
		return X * X + Y * Y;
	}
	int IsNearlyZero(float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return
			Abs(X) < Tolerance
			&& Abs(Y) < Tolerance
			&& Abs(Z) < Tolerance;
	}
	BOOL IsZero() const
	{
		return X == 0.f && Y == 0.f && Z == 0.f;
	}

	BOOL Normalize(float Tolerance = SMALL_NUMBER)
	{
		const float SquareSum = X * X + Y * Y + Z * Z;
		if (SquareSum > Tolerance)
		{
			const float Scale = appInvSqrt(SquareSum);
			X *= Scale; Y *= Scale; Z *= Scale;
			return TRUE;
		}
		return FALSE;
	}
	/**
	* Returns TRUE if Normalized.
	*/
	BOOL IsNormalized() const
	{
		return (Abs(1.f - SizeSquared()) <= 0.01f);
	}

	//FORCEINLINE void WriteToStream(MemoryStream* stream) const noexcept {
	//	stream->WriteFloat(X);
	//	stream->WriteFloat(Y);
	//	stream->WriteFloat(Z);
	//}

	/** Util to convert this vector into a unit direction vector, and its original length */
	void ToDirectionAndLength(SVector& OutDir, float& OutLength)
	{
		OutLength = Size();
		if (OutLength > SMALL_NUMBER)
		{
			float OneOverLength = 1.0f / OutLength;
			OutDir = SVector(X * OneOverLength, Y * OneOverLength,
				Z * OneOverLength);
		}
		else
		{
			OutDir = SVector(0.0f, 0.0f, 0.0f);
		}
	}

	SVector Projection() const
	{
		const float RZ = 1.f / Z;
		return SVector(X * RZ, Y * RZ, 1);
	}
	FORCEINLINE SVector UnsafeNormal() const
	{
		const float Scale = appInvSqrt(X * X + Y * Y + Z * Z);
		return SVector(X * Scale, Y * Scale, Z * Scale);
	}
	SVector GridSnap(const float& GridSz) const
	{
		return SVector(SSnap(X, GridSz), SSnap(Y, GridSz), SSnap(Z, GridSz));
	}
	FORCEINLINE SVector Move2D(const float distance, float radAngle) const noexcept {
		return SVector(
		(X + (distance * appCos(radAngle))),
			(Y + (distance * appSin(radAngle))),
			Z);
	}
	SVector BoundToCube(float Radius) const
	{
		return SVector
		(
			Clamp(X, -Radius, Radius),
			Clamp(Y, -Radius, Radius),
			Clamp(Z, -Radius, Radius)
			);
	}
	void AddBounded(const SVector& V, float Radius = (float)INT16_MAX)
	{
		*this = (*this + V).BoundToCube(Radius);
	}

	/** Convert a direction vector into a 'heading' angle between +/-PI. 0 is pointing down +X. */
	inline float ToHeadingAngle()
	{
		// Project Dir into Z plane.
		SVector PlaneDir = *this;
		PlaneDir.Z = 0.f;
		PlaneDir = PlaneDir.SafeNormal();

		float Angle = appAcos(PlaneDir.X);

		if (PlaneDir.Y < 0.0f)
		{
			Angle *= -1.0f;
		}

		return Angle;
	}

	float& Component(INT Index)
	{
		return (&X)[Index];
	}

	/**
	* Returns TRUE if X == Y == Z within the specified tolerance.
	*/
	BOOL IsUniform(float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return (Abs(X - Y) < Tolerance) && (Abs(Y - Z) < Tolerance);
	}

	/**
	* Mirror a vector about a normal vector.
	*/
	SVector MirrorByVector(const SVector& MirrorNormal) const
	{
		return *this - MirrorNormal * (2.f * (*this | MirrorNormal));
	}

	/**
	* Mirrors a vector about a plane.
	*/
	//SVector MirrorByPlane(const SPlane& Plane) const;

	/**
	* Rotates around Axis (assumes Axis.Size() == 1).
	*/
	inline SVector RotateAngleAxis(const INT Angle, const SVector& Axis) const
	{
		const float S = g_SGlobalMath.SinTab(Angle);
		const float C = g_SGlobalMath.CosTab(Angle);

		const float XX = Axis.X * Axis.X;
		const float YY = Axis.Y * Axis.Y;
		const float ZZ = Axis.Z * Axis.Z;

		const float XY = Axis.X * Axis.Y;
		const float YZ = Axis.Y * Axis.Z;
		const float ZX = Axis.Z * Axis.X;

		const float XS = Axis.X * S;
		const float YS = Axis.Y * S;
		const float ZS = Axis.Z * S;

		const float OMC = 1.f - C;

		return SVector(
		(OMC * XX + C) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z,
			(OMC * XY + ZS) * X + (OMC * YY + C) * Y + (OMC * YZ - XS) * Z,
			(OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C) * Z
			);
	}

	/**
	* Find good arbitrary axis vectors to represent U and V axes of a plane,
	* given just the normal.
	*/
	void FindBestAxisVectors(SVector& Axis1, SVector& Axis2) const
	{
		const float NX = Abs(X);
		const float NY = Abs(Y);
		const float NZ = Abs(Z);

		// Find best basis vectors.
		if (NZ > NX && NZ > NY)	Axis1 = SVector(1, 0, 0);
		else					Axis1 = SVector(0, 0, 1);

		Axis1 = (Axis1 - *this * (Axis1 | *this)).SafeNormal();
		Axis2 = Axis1 ^ *this;
	}

	FORCEINLINE SVector SafeNormal(float Tolerance = SMALL_NUMBER) const
	{
		const float SquareSum = X * X + Y * Y + Z * Z;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if (SquareSum == 1.f)
		{
			return *this;
		}
		else if (SquareSum < Tolerance)
		{
			return SVector(0.f);
		}
		const float Scale = appInvSqrt(SquareSum);
		return SVector(X * Scale, Y * Scale, Z * Scale);
	}

	FORCEINLINE SVector SafeNormal2D(float Tolerance = SMALL_NUMBER) const
	{
		const float SquareSum = X * X + Y * Y;

		// Not sure if it's safe to add tolerance in there. Might introduce too many errors
		if (SquareSum == 1.f)
		{
			if (Z == 0.f)
			{
				return *this;
			}
			else
			{
				return SVector(X, Y, 0.f);
			}
		}
		else if (SquareSum < Tolerance)
		{
			return SVector(0.f);
		}

		const float Scale = appInvSqrt(SquareSum);
		return SVector(X * Scale, Y * Scale, 0.f);
	}

	/**
	* Performs a 2D dot product (no z)
	*
	* @param B the vector to perform the dot product with
	*/
	FORCEINLINE float Dot2d(SVector B)
	{
		SVector A(*this);
		A.Z = 0.0f;
		B.Z = 0.0f;
		A.Normalize();
		B.Normalize();
		return A | B;
	}

	/**
	* Projects this vector onto the input vector.  Does not assume A is unnormalized.
	*/
	FORCEINLINE SVector ProjectOnTo(const SVector& A) const
	{
		return (A * ((*this | A) / (A | A)));
	}

	/** When this vector contains Euler angles (degrees), ensure that angles are between +/-180 */
	void UnwindEuler();

	/** Utility to check if there are any NaNs in this vector. */
	BOOL ContainsNaN() const
	{
		return (appIsNaN(X) || !appIsFinite(X) ||
			appIsNaN(Y) || !appIsFinite(Y) ||
			appIsNaN(Z) || !appIsFinite(Z));
	}

	/**
	* Returns TRUE if the vector is a unit vector within the specified tolerance.
	*/
	inline BOOL IsUnit(float LengthSquaredTolerance = KINDA_SMALL_NUMBER) const
	{
		return Abs(1.0f - SizeSquared()) < LengthSquaredTolerance;
	}

	/* Lerp Member Function
	Calculates the lerp for the vector and vector supplied using unit interval t */
	void Lerp(SVector& b, float t)
	{
		X = ((b.X - X) * t);
		Y = ((b.Y - Y) * t);
		Z = ((b.Z - Z) * t);
	}

	SAngle FaceOther(const SVector& other) const noexcept {
		return SAngle(
		(INT16)(appAtan2(other.Y - Y, other.X - X) * RadToUnit)
			);
	}

	//Select min X,Y,Z individually, from both vectors
	void MinOnAllAxis(const SVector& other) noexcept {
		X = fminf(X, other.X);
		Y = fminf(Y, other.Y);
		Z = fminf(Z, other.Z);
	}

	//Select max X,Y,Z individually from both vectors
	void MaxOnAllAxis(const SVector& other) noexcept {
		X = fmaxf(X, other.X);
		Y = fmaxf(Y, other.Y);
		Z = fmaxf(Z, other.Z);
	}

	//inline SRotator Rotation() const noexcept;

	float DistanceTo(const SVector& Other) const noexcept {
		return sqrtf(pow(Other.X - X, 2) + pow(Other.Y - Y, 2) + pow(Other.Z - Z, 2));
	}

	static const  SVector ZeroVector;
};

__declspec(align(16))
struct SVector4 {
	float X, Y, Z, W;

	SVector4(const SVector4& InVector, float InW = 1.0f) :
		X(InVector.X), Y(InVector.Y), Z(InVector.Z), W(InW)
	{}

	explicit SVector4(float InX = 0.0f, float InY = 0.0f, float InZ = 0.0f, float InW = 1.0f) :
		X(InX), Y(InY), Z(InZ), W(InW)
	{}

	/** Compoment Accessors */
	FORCEINLINE float& operator[](INT ComponentIndex)
	{
		return (&X)[ComponentIndex];
	}
	FORCEINLINE float operator[](INT ComponentIndex) const
	{
		return (&X)[ComponentIndex];
	}
	FORCEINLINE void Set(float InX, float InY, float InZ, float InW)
	{
		X = InX;
		Y = InY;
		Z = InZ;
		W = InW;
	}

	// Unary operators.
	FORCEINLINE SVector4 operator-() const
	{
		return SVector4(-X, -Y, -Z, -W);
	}

	// Binary math operators.
	FORCEINLINE SVector4 operator+(const SVector4& V) const
	{
		return SVector4(X + V.X, Y + V.Y, Z + V.Z, W + V.W);
	}
	FORCEINLINE SVector4 operator-(const SVector4& V) const
	{
		return SVector4(X - V.X, Y - V.Y, Z - V.Z, W - V.W);
	}
	FORCEINLINE SVector4 operator*(float Scale) const
	{
		return SVector4(X * Scale, Y * Scale, Z * Scale, W * Scale);
	}
	SVector4 operator/(float Scale) const
	{
		const float RScale = 1.f / Scale;
		return SVector4(X * RScale, Y * RScale, Z * RScale, W * RScale);
	}
	FORCEINLINE SVector4 operator*(const SVector4& V) const
	{
		return SVector4(X * V.X, Y * V.Y, Z * V.Z, W * V.W);
	}

	// Simple functions.
	float& Component(INT Index)
	{
		return (&X)[Index];
	}
	friend FORCEINLINE float Dot3(const SVector4& V1, const SVector4& V2)
	{
		return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
	}
	friend FORCEINLINE float Dot4(const SVector4& V1, const SVector4& V2)
	{
		return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z + V1.W * V2.W;
	}
	friend FORCEINLINE SVector4 operator*(float Scale, const SVector4& V)
	{
		return V.operator*(Scale);
	}

	/**
	* Basic == or != operators for SQuat
	*/
	BOOL operator==(const SVector4& V) const
	{
		return ((X == V.X) && (Y == V.Y) && (Z == V.Z) && (W == V.W));
	}
	BOOL operator!=(const SVector4& V) const
	{
		return ((X != V.X) || (Y != V.Y) || (Z != V.Z) || (W != V.W));
	}

	/**
	* Error tolerant comparison
	*/
	BOOL Equals(const SVector4& V, float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return Abs(X - V.X) < Tolerance && Abs(Y - V.Y) < Tolerance && Abs(Z - V.Z) < Tolerance && Abs(W - V.W) < Tolerance;
	}

	/** Returns a normalized 3D SVector */
	FORCEINLINE SVector4 SafeNormal(float Tolerance = SMALL_NUMBER) const
	{
		const float SquareSum = X * X + Y * Y + Z * Z;
		if (SquareSum > Tolerance)
		{
			const float Scale = appInvSqrt(SquareSum);
			return SVector4(X * Scale, Y * Scale, Z * Scale, 0.0f);
		}
		return SVector4(0.f);
	}

	FORCEINLINE SVector ToVector() const noexcept {
		return SVector(
			X,
			Y,
			Z
			);
	}

	static SVector4 ZeroVector;

};

__declspec(align(16))
struct SPlane : public SVector
{
public:
	// Variables.
	float W;

	// Constructors.
	FORCEINLINE SPlane()
	{}
	FORCEINLINE SPlane(const SPlane& P)
		: SVector(P)
		, W(P.W)
	{}
	FORCEINLINE SPlane(const SVector4& V)
		: SVector(V.X, V.Y, V.Z)
		, W(V.W)
	{}
	FORCEINLINE SPlane(float InX, float InY, float InZ, float InW)
		: SVector(InX, InY, InZ)
		, W(InW)
	{}
	FORCEINLINE SPlane(SVector InNormal, float InW)
		: SVector(InNormal), W(InW)
	{}
	FORCEINLINE SPlane(SVector InBase, const SVector& InNormal)
		: SVector(InNormal)
		, W(InBase | InNormal)
	{}
	SPlane(SVector A, SVector B, SVector C)
		: SVector(((B - A) ^ (C - A)).SafeNormal())
		, W(A | ((B - A) ^ (C - A)).SafeNormal())
	{}


	// Functions.
	FORCEINLINE float PlaneDot(const SVector& P) const
	{
		return X * P.X + Y * P.Y + Z * P.Z - W;
	}
	SPlane Flip() const
	{
		return SPlane(-X, -Y, -Z, -W);
	}

	BOOL operator==(const SPlane& V) const
	{
		return X == V.X && Y == V.Y && Z == V.Z && W == V.W;
	}
	BOOL operator!=(const SPlane& V) const
	{
		return X != V.X || Y != V.Y || Z != V.Z || W != V.W;
	}

	// Error-tolerant comparison.
	BOOL Equals(const SPlane& V, float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return Abs(X - V.X) < Tolerance && Abs(Y - V.Y) < Tolerance && Abs(Z - V.Z) < Tolerance && Abs(W - V.W) < Tolerance;
	}

	FORCEINLINE float operator|(const SPlane& V) const
	{
		return X * V.X + Y * V.Y + Z * V.Z + W * V.W;
	}
	SPlane operator+(const SPlane& V) const
	{
		return SPlane(X + V.X, Y + V.Y, Z + V.Z, W + V.W);
	}
	SPlane operator-(const SPlane& V) const
	{
		return SPlane(X - V.X, Y - V.Y, Z - V.Z, W - V.W);
	}
	SPlane operator/(float Scale) const
	{
		const float RScale = 1.f / Scale;
		return SPlane(X * RScale, Y * RScale, Z * RScale, W * RScale);
	}
	SPlane operator*(float Scale) const
	{
		return SPlane(X * Scale, Y * Scale, Z * Scale, W * Scale);
	}
	SPlane operator*(const SPlane& V)
	{
		return SPlane(X * V.X, Y * V.Y, Z * V.Z, W * V.W);
	}
	SPlane operator+=(const SPlane& V)
	{
		X += V.X; Y += V.Y; Z += V.Z; W += V.W;
		return *this;
	}
	SPlane operator-=(const SPlane& V)
	{
		X -= V.X; Y -= V.Y; Z -= V.Z; W -= V.W;
		return *this;
	}
	SPlane operator*=(float Scale)
	{
		X *= Scale; Y *= Scale; Z *= Scale; W *= Scale;
		return *this;
	}
	SPlane operator*=(const SPlane& V)
	{
		X *= V.X; Y *= V.Y; Z *= V.Z; W *= V.W;
		return *this;
	}
	SPlane operator/=(float V)
	{
		const float RV = 1.f / V;
		X *= RV; Y *= RV; Z *= RV; W *= RV;
		return *this;
	}
};

struct SSphere
{
public:

	// Variables.
	SVector Center;
	float W;

	// Constructors.
	SSphere()
	{}
	SSphere(INT) : Center(0, 0, 0), W(0)
	{}
	SSphere(SVector InV, float InW) : Center(InV), W(InW)
	{}
	SSphere(const SVector* Pts, INT Count);

	//inline SSphere TransformBy(const SMatrix& M) const;

	// Error-tolerant comparison.
	BOOL Equals(const SSphere& Sphere, float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return Center.Equals(Sphere.Center, Tolerance) && Abs(W - Sphere.W) < Tolerance;
	}

	BOOL IsInside(const SSphere& Other, float Tolerance = KINDA_SMALL_NUMBER) const
	{
		if (W > Other.W - Tolerance)
		{
			return FALSE;
		}
		return (Center - Other.Center).SizeSquared() <= Square(Other.W - Tolerance - W);
	}
};

struct SBox
{
public:
	// Variables.
	SVector Min;
	SVector Max;
	UINT8 IsValid;

	// Constructors.
	SBox() {}
	SBox(INT) { Init(); }
	SBox(const SVector& InMin, const SVector& InMax) : Min(InMin), Max(InMax), IsValid(1) {}
	SBox(const SVector* Points, INT Count);

	/** Utility function to build an AABB from Origin and Extent */
	static SBox BuildAABB(const SVector& Origin, const SVector& Extent)
	{
		SBox NewBox(Origin - Extent, Origin + Extent);
		return NewBox;
	}

	// Accessors.
	SVector& GetExtrema(int i)
	{
		return (&Min)[i];
	}
	const SVector& GetExtrema(int i) const
	{
		return (&Min)[i];
	}

	// Functions.
	void Init()
	{
		Min = Max = SVector(0, 0, 0);
		IsValid = 0;
	}
	FORCEINLINE SBox& operator+=(const SVector& Other)
	{
		if (IsValid)
		{
			//#if ASM_X86
			//__asm
			//{
			//	mov		eax, [Other]
			//	mov		ecx, [this]

			//	movss	xmm3, [eax]SVector.X
			//	movss	xmm4, [eax]SVector.Y
			//	movss	xmm5, [eax]SVector.Z

			//	movss	xmm0, [ecx]SBox.Min.X
			//	movss	xmm1, [ecx]SBox.Min.Y
			//	movss	xmm2, [ecx]SBox.Min.Z
			//	minss	xmm0, xmm3
			//	minss	xmm1, xmm4
			//	minss	xmm2, xmm5
			//	movss[ecx]SBox.Min.X, xmm0
			//	movss[ecx]SBox.Min.Y, xmm1
			//	movss[ecx]SBox.Min.Z, xmm2

			//	movss	xmm0, [ecx]SBox.Max.X
			//	movss	xmm1, [ecx]SBox.Max.Y
			//	movss	xmm2, [ecx]SBox.Max.Z
			//	maxss	xmm0, xmm3
			//	maxss	xmm1, xmm4
			//	maxss	xmm2, xmm5
			//	movss[ecx]SBox.Max.X, xmm0
			//	movss[ecx]SBox.Max.Y, xmm1
			//	movss[ecx]SBox.Max.Z, xmm2
			//}
			////#else
			Min.X = ::Min(Min.X, Other.X);
			Min.Y = ::Min(Min.Y, Other.Y);
			Min.Z = ::Min(Min.Z, Other.Z);

			Max.X = ::Max(Max.X, Other.X);
			Max.Y = ::Max(Max.Y, Other.Y);
			Max.Z = ::Max(Max.Z, Other.Z);
			//#endif
		}
		else
		{
			Min = Max = Other;
			IsValid = 1;
		}
		return *this;
	}
	SBox operator+(const SVector& Other) const
	{
		return SBox(*this) += Other;
	}
	SBox& operator+=(const SBox& Other)
	{
		if (IsValid && Other.IsValid)
		{
			//#if ASM_X86
			/*__asm
			{
				mov		eax, [Other]
				mov		ecx, [this]

				movss	xmm0, [ecx]SBox.Min.X
				movss	xmm1, [ecx]SBox.Min.Y
				movss	xmm2, [ecx]SBox.Min.Z
				minss	xmm0, [eax]SBox.Min.X
				minss	xmm1, [eax]SBox.Min.Y
				minss	xmm2, [eax]SBox.Min.Z
				movss[ecx]SBox.Min.X, xmm0
				movss[ecx]SBox.Min.Y, xmm1
				movss[ecx]SBox.Min.Z, xmm2

				movss	xmm0, [ecx]SBox.Max.X
				movss	xmm1, [ecx]SBox.Max.Y
				movss	xmm2, [ecx]SBox.Max.Z
				maxss	xmm0, [eax]SBox.Max.X
				maxss	xmm1, [eax]SBox.Max.Y
				maxss	xmm2, [eax]SBox.Max.Z
				movss[ecx]SBox.Max.X, xmm0
				movss[ecx]SBox.Max.Y, xmm1
				movss[ecx]SBox.Max.Z, xmm2
			}*/
			//#else
			Min.X = ::Min(Min.X, Other.Min.X);
			Min.Y = ::Min(Min.Y, Other.Min.Y);
			Min.Z = ::Min(Min.Z, Other.Min.Z);

			Max.X = ::Max(Max.X, Other.Max.X);
			Max.Y = ::Max(Max.Y, Other.Max.Y);
			Max.Z = ::Max(Max.Z, Other.Max.Z);
			//#endif
		}
		else if (Other.IsValid)
		{
			(*this) = Other;
		}
		return *this;
	}
	SBox operator+(const SBox& Other) const
	{
		return SBox(*this) += Other;
	}
	SVector& operator[](INT i)
	{
		if (i == 0)		return Min;
		else				return Max;
	}
	//SBox TransformBy(const SMatrix& M) const;
	SBox ExpandBy(float W) const
	{
		return SBox(Min - SVector(W, W, W), Max + SVector(W, W, W));
	}

	// Returns the midpoint between the min and max points.
	SVector GetCenter() const
	{
		return SVector((Min + Max) * 0.5f);
	}
	// Returns the extent around the center
	SVector GetExtent() const
	{
		return (Max - Min) * 0.5f;
	}

	void GetCenterAndExtents(SVector& center, SVector& Extents) const
	{
		Extents = GetExtent();
		center = Min + Extents;
	}

	BOOL Intersect(const SBox& other) const
	{
		if (Min.X > other.Max.X || other.Min.X > Max.X)
			return FALSE;
		if (Min.Y > other.Max.Y || other.Min.Y > Max.Y)
			return FALSE;
		if (Min.Z > other.Max.Z || other.Min.Z > Max.Z)
			return FALSE;
		return TRUE;
	}

	BOOL IntersectXY(const SBox& other) const
	{
		if (Min.X > other.Max.X || other.Min.X > Max.X)
			return FALSE;
		if (Min.Y > other.Max.Y || other.Min.Y > Max.Y)
			return FALSE;
		return TRUE;
	}

	// Checks to see if the location is inside this box
	BOOL IsInside(const SVector& In) const
	{
		return (In.X > Min.X && In.X < Max.X
			&& In.Y > Min.Y && In.Y < Max.Y
			&& In.Z > Min.Z && In.Z < Max.Z);
	}

	/** Calculate volume of this box. */
	float GetVolume() const
	{
		return ((Max.X - Min.X) * (Max.Y - Min.Y) * (Max.Z - Min.Z));
	}

	/** Util to calculate distance from a point to a bounding box */
	inline float ComputeSquaredDistanceToPoint(const SVector& Point) const
	{
		// Accumulates the distance as we iterate axis
		float DistSquared = 0.f;

		// Check each axis for min/max and add the distance accordingly
		// NOTE: Loop manually unrolled for > 2x speed up
		if (Point.X < Min.X)
		{
			DistSquared += Square(Point.X - Min.X);
		}
		else if (Point.X > Max.X)
		{
			DistSquared += Square(Point.X - Max.X);
		}

		if (Point.Y < Min.Y)
		{
			DistSquared += Square(Point.Y - Min.Y);
		}
		else if (Point.Y > Max.Y)
		{
			DistSquared += Square(Point.Y - Max.Y);
		}

		if (Point.Z < Min.Z)
		{
			DistSquared += Square(Point.Z - Min.Z);
		}
		else if (Point.Z > Max.Z)
		{
			DistSquared += Square(Point.Z - Max.Z);
		}

		return DistSquared;
	}

	/** Return closest point on or inside the box to the given point in space. */
	SVector GetClosestPointTo(const SVector& Point) const
	{
		// Start by considering the Point inside the Box.
		SVector ClosestPoint = Point;

		// Now clamp to inside box if it's outside.
		if (Point.X < Min.X)
		{
			ClosestPoint.X = Min.X;
		}
		else if (Point.X > Max.X)
		{
			ClosestPoint.X = Max.X;
		}

		// Now clamp to inside box if it's outside.
		if (Point.Y < Min.Y)
		{
			ClosestPoint.Y = Min.Y;
		}
		else if (Point.Y > Max.Y)
		{
			ClosestPoint.Y = Max.Y;
		}

		// Now clamp to inside box if it's outside.
		if (Point.Z < Min.Z)
		{
			ClosestPoint.Z = Min.Z;
		}
		else if (Point.Z > Max.Z)
		{
			ClosestPoint.Z = Max.Z;
		}

		return ClosestPoint;
	}
};

struct SRotator {
	// Variables.
	INT Pitch; // Looking up and down (0=Straight Ahead, +Up, -Down).
	INT Yaw;   // Rotating around (running in circles), 0=East, +North, -South.
	INT Roll;  // Rotation about axis of screen, 0=Straight, +Clockwise, -CCW.

			   // Constructors.
	SRotator() {}
	SRotator(INT InPitch, INT InYaw, INT InRoll)
		: Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}

	// Binary arithmetic operators.
	SRotator operator+(const SRotator& R) const
	{
		return SRotator(Pitch + R.Pitch, Yaw + R.Yaw, Roll + R.Roll);
	}
	SRotator operator-(const SRotator& R) const
	{
		return SRotator(Pitch - R.Pitch, Yaw - R.Yaw, Roll - R.Roll);
	}
	SRotator operator*(float Scale) const
	{
		return SRotator(appTrunc(Pitch * Scale), appTrunc(Yaw * Scale), appTrunc(Roll * Scale));
	}
	SRotator operator*= (float Scale)
	{
		Pitch = appTrunc(Pitch * Scale); Yaw = appTrunc(Yaw * Scale); Roll = appTrunc(Roll * Scale);
		return *this;
	}
	// Binary comparison operators.
	BOOL operator==(const SRotator& R) const
	{
		return Pitch == R.Pitch && Yaw == R.Yaw && Roll == R.Roll;
	}
	BOOL operator!=(const SRotator& V) const
	{
		return Pitch != V.Pitch || Yaw != V.Yaw || Roll != V.Roll;
	}
	// Assignment operators.
	SRotator operator+=(const SRotator& R)
	{
		Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
		return *this;
	}
	SRotator operator-=(const SRotator& R)
	{
		Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
		return *this;
	}

	// Functions.
	SRotator Reduce() const
	{
		return SRotator(ReduceAngle(Pitch), ReduceAngle(Yaw), ReduceAngle(Roll));
	}
	int IsZero() const
	{
		return ((Pitch & 65535) == 0) && ((Yaw & 65535) == 0) && ((Roll & 65535) == 0);
	}
	SRotator Add(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll)
	{
		Yaw += DeltaYaw;
		Pitch += DeltaPitch;
		Roll += DeltaRoll;
		return *this;
	}
	SRotator AddBounded(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll)
	{
		Yaw += DeltaYaw;
		Pitch = SAddAngleConfined(Pitch, DeltaPitch, 192 * 0x100, 64 * 0x100);
		Roll = SAddAngleConfined(Roll, DeltaRoll, 192 * 0x100, 64 * 0x100);
		return *this;
	}
	SRotator GridSnap(const SRotator& RotGrid) const
	{
		return SRotator
		(
			appTrunc(SSnap((float)Pitch, (float)RotGrid.Pitch)),
			appTrunc(SSnap((float)Yaw, (float)RotGrid.Yaw)),
			appTrunc(SSnap((float)Roll, (float)RotGrid.Roll))
			);
	}
	SVector Vector() const;
	//SQuat Quaternion() const;
	SVector Euler() const;

	static SRotator MakeFromEuler(const SVector& Euler);

	// Resets the rotation values so they fall within the range [0,65535]
	SRotator Clamp() const
	{
		return SRotator(Pitch & 65535, Yaw & 65535, Roll & 65535);
	}

	static INT NormalizeAxis(INT Angle)
	{
		Angle &= 0xFFFF;
		if (Angle > 32767)
		{
			Angle -= 0x10000;
		}
		return Angle;
	}

	SRotator GetNormalized() const
	{
		SRotator Rot = *this;
		Rot.Pitch = NormalizeAxis(Rot.Pitch);
		Rot.Roll = NormalizeAxis(Rot.Roll);
		Rot.Yaw = NormalizeAxis(Rot.Yaw);
		return Rot;
	}

	SRotator GetDenormalized() const
	{
		SRotator Rot = *this;
		Rot.Pitch = Rot.Pitch & 0xFFFF;
		Rot.Yaw = Rot.Yaw & 0xFFFF;
		Rot.Roll = Rot.Roll & 0xFFFF;
		return Rot;
	}

	/**
	*	Decompose this Rotator into a Winding part (multiples of 65536) and a Remainder part.
	*	Remainder will always be in [-32768, 32767] range.
	*/
	void MakeShortestRoute();
	/**
	*	Decompose this Rotator into a Winding part (multiples of 65536) and a Remainder part.
	*	Remainder will always be in [-32768, 32767] range.
	*/
	void GetWindingAndRemainder(SRotator& Winding, SRotator& Remainder) const;

	static SRotator ZeroRotator;
};

__declspec(align(16))
struct SQuat
{
public:
	static const SQuat Identity;

	// Variables.
	float X, Y, Z, W;
	// X,Y,Z, W also doubles as the Axis/Angle format.

	// Constructors.
	FORCEINLINE SQuat()
	{}

	FORCEINLINE SQuat(float InX, float InY, float InZ, float InA)
		: X(InX), Y(InY), Z(InZ), W(InA)
	{}

	//explicit FORCEINLINE SQuat(const SMatrix& M);

	FORCEINLINE SQuat(const SQuat& Q) :
		X(Q.X),
		Y(Q.Y),
		Z(Q.Z),
		W(Q.W)
	{
	}

	//explicit FORCEINLINE SQuat(const SRotator& R);

	/**
	* Copy another SQuat into this one
	*/
	FORCEINLINE SQuat& operator=(const SQuat& Other)
	{
		this->X = Other.X;
		this->Y = Other.Y;
		this->Z = Other.Z;
		this->W = Other.W;

		return *this;
	}

	/**
	* Assumes Axis is normalized.
	*/
	SQuat(SVector Axis, float Angle)
	{
		const float half_a = 0.5f * Angle;
		const float s = appSin(half_a);
		const float c = appCos(half_a);

		X = s * Axis.X;
		Y = s * Axis.Y;
		Z = s * Axis.Z;
		W = c;
	}

	//static SQuat MakeFromEuler(const SVector& Euler);

	SVector Euler() const;

	// Binary operators.
	FORCEINLINE SQuat operator+(const SQuat& Q) const
	{
		return SQuat(X + Q.X, Y + Q.Y, Z + Q.Z, W + Q.W);
	}

	FORCEINLINE SQuat operator+=(const SQuat& Q)
	{
		this->X += Q.X;
		this->Y += Q.Y;
		this->Z += Q.Z;
		this->W += Q.W;
		return *this;
	}

	FORCEINLINE SQuat operator-(const SQuat& Q) const
	{
		return SQuat(X - Q.X, Y - Q.Y, Z - Q.Z, W - Q.W);
	}

	inline BOOL Equals(const SQuat& Q, float Tolerance = KINDA_SMALL_NUMBER) const
	{
		return Abs(X - Q.X) < Tolerance && Abs(Y - Q.Y) < Tolerance && Abs(Z - Q.Z) < Tolerance && Abs(W - Q.W) < Tolerance;
	}

	FORCEINLINE SQuat operator-=(const SQuat& Q)
	{
		this->X -= Q.X;
		this->Y -= Q.Y;
		this->Z -= Q.Z;
		this->W -= Q.W;
		return *this;
	}

	FORCEINLINE SQuat operator*(const SQuat& Q) const
	{
		SQuat Result;
		VectorQuaternionMultiply(&Result, this, &Q);
		return Result;
	}

	FORCEINLINE SQuat operator*=(const SQuat& Q)
	{
		/**
		* Now this uses VectorQuaternionMultiply that is optimized per platform.
		*/
		VectorRegister A = VectorLoadAligned(this);
		VectorRegister B = VectorLoadAligned(&Q);
		VectorRegister Result;
		VectorQuaternionMultiply(&Result, &A, &B);
		VectorStoreAligned(Result, this);

		return *this;
	}

	FORCEINLINE SQuat operator*=(const float Scale)
	{
		X *= Scale;
		Y *= Scale;
		Z *= Scale;
		W *= Scale;

		return *this;
	}

	FORCEINLINE SQuat operator*(const float Scale) const
	{
		return SQuat(Scale * X, Scale * Y, Scale * Z, Scale * W);
	}

	FORCEINLINE SQuat operator/(const float Scale) const
	{
		return SQuat(X / Scale, Y / Scale, Z / Scale, W / Scale);
	}
	// Unary operators.
	FORCEINLINE SQuat operator-() const
	{
		// For historical reasons, this returns the Multiplicative Inverse, and assumes *this is normalized.
		return SQuat(-X, -Y, -Z, W);
	}

	// Misc operators
	BOOL operator==(const SQuat& Q) const
	{
		return X == Q.X && Y == Q.Y && Z == Q.Z && W == Q.W;
	}

	BOOL operator!=(const SQuat& Q) const
	{
		return X != Q.X || Y != Q.Y || Z != Q.Z || W != Q.W;
	}

	float operator|(const SQuat& Q) const
	{
		return X * Q.X + Y * Q.Y + Z * Q.Z + W * Q.W;
	}

	FORCEINLINE void Normalize(float Tolerance = SMALL_NUMBER)
	{
		const float SquareSum = X * X + Y * Y + Z * Z + W * W;
		if (SquareSum > Tolerance)
		{
			const float Scale = appInvSqrt(SquareSum);
			X *= Scale;
			Y *= Scale;
			Z *= Scale;
			W *= Scale;
		}
		else
		{
			*this = SQuat::Identity;
		}
	}

	// Return TRUE if this quaternion is normalized
	BOOL IsNormalized() const
	{
		return (Abs(1.f - SizeSquared()) <= 0.01f);
	}

	FORCEINLINE float Size() const
	{
		return appSqrt(X * X + Y * Y + Z * Z + W * W);
	}

	FORCEINLINE float SizeSquared() const
	{
		return (X * X + Y * Y + Z * Z + W * W);
	}

	// Warning : assumes normalized quaternions.
	void ToAxisAndAngle(SVector& Axis, float& Angle) const
	{
		Angle = 2.f * appAcos(W);

		// Ensure we never try to sqrt a neg number
		const float S = appSqrt(::Max(1.f - (W * W), 0.f));
		if (S >= 0.0001f)
		{
			Axis.X = X / S;
			Axis.Y = Y / S;
			Axis.Z = Z / S;
		}
		else
		{
			Axis.X = 1.f;
			Axis.Y = 0.f;
			Axis.Z = 0.f;
		}
	};

	SVector RotateVector(SVector v) const
	{
		// (q.W*q.W-qv.qv)v + 2(qv.v)qv + 2 q.W (qv x v)

		const SVector qv(X, Y, Z);
		SVector vOut = (qv ^ v) * W * 2.f;
		vOut += v * ((W * W) - (qv | qv));
		vOut += qv * (2.f * (qv | v));

		return vOut;
	}

	// Exp should really only be used after Log.
	SQuat Log() const;
	SQuat Exp() const;
	FORCEINLINE SQuat Inverse() const
	{
		return SQuat(-X, -Y, -Z, W);
	}

	/**
	* Enforce that the delta between this Quaternion and another one represents
	* the shortest possible rotation angle
	*/
	void EnforceShortestArcWith(const SQuat& OtherQuat)
	{
		const float DotResult = (OtherQuat | *this);
		const float Bias = appFloatSelect(DotResult, 1.0f, -1.0f);
		X *= Bias;
		Y *= Bias;
		Z *= Bias;
		W *= Bias;
	}

	/** Get X Rotation Axis. */
	FORCEINLINE SVector GetAxisX() const
	{
		return RotateVector(SVector(1.f, 0.f, 0.f));
	}
	/** Get Y Rotation Axis. */
	FORCEINLINE SVector GetAxisY() const
	{
		return RotateVector(SVector(0.f, 1.f, 0.f));
	}
	/** Get Z Rotation Axis. */
	FORCEINLINE SVector GetAxisZ() const
	{
		return RotateVector(SVector(0.f, 0.f, 1.f));
	}

	//FORCEINLINE SQuat MakeFromRotator(const SRotator & rotator) const;
	FORCEINLINE SRotator Rotator() const;
};

struct SMatrix
{
public:
	union
	{
		__declspec(align(16)) float M[4][4];
	};
	__declspec(align(16)) static const SMatrix Identity;

	// Constructors.

	FORCEINLINE SMatrix();
	FORCEINLINE SMatrix(const SPlane& InX, const SPlane& InY, const SPlane& InZ, const SPlane& InW);
	FORCEINLINE SMatrix(const SVector& InX, const SVector& InY, const SVector& InZ, const SVector& InW);

	// Destructor.
	inline void SetIdentity();

	// Concatenation operator.

	FORCEINLINE SMatrix		operator* (const SMatrix& Other) const;
	FORCEINLINE void		operator*=(const SMatrix& Other);

	FORCEINLINE SMatrix		operator+ (const SMatrix& Other) const;
	FORCEINLINE void		operator+=(const SMatrix& Other);

	/**
	* This isn't applying SCALE, just multiplying float to all members - i.e. weighting
	*/
	FORCEINLINE SMatrix		operator* (float Other) const;
	FORCEINLINE void		operator*=(float Other);
	// Comparison operators.

	inline BOOL operator==(const SMatrix& Other) const;

	// Error-tolerant comparison.
	inline BOOL Equals(const SMatrix& Other, float Tolerance = KINDA_SMALL_NUMBER) const;

	inline BOOL operator!=(const SMatrix& Other) const;

	// Homogeneous transform.

	FORCEINLINE SVector4 TransformFVector4(const SVector4& V) const;

	// Regular transform.

	/** Transform a location - will take into account translation part of the SMatrix. */
	FORCEINLINE SVector4 TransformFVector(const SVector& V) const;

	/** Inverts the matrix and then transforms V - correctly handles scaling in this matrix. */
	FORCEINLINE SVector InverseTransformFVector(const SVector& V) const;

	/** Faster version of InverseTransformFVector that assumes no scaling. WARNING: Will NOT work correctly if there is scaling in the matrix. */
	FORCEINLINE SVector InverseTransformFVectorNoScale(const SVector& V) const;

	// Normal transform.

	/**
	*	Transform a direction vector - will not take into account translation part of the SMatrix.
	*	If you want to transform a surface normal (or plane) and correctly account for non-uniform scaling you should use TransformByUsingAdjointT.
	*/
	FORCEINLINE SVector4 TransformNormal(const SVector& V) const;

	/**
	*	Transform a direction vector by the inverse of this matrix - will not take into account translation part.
	*	If you want to transform a surface normal (or plane) and correctly account for non-uniform scaling you should use TransformByUsingAdjointT with adjoint of matrix inverse.
	*/
	FORCEINLINE SVector InverseTransformNormal(const SVector& V) const;

	/** Faster version of InverseTransformNormal that assumes no scaling. WARNING: Will NOT work correctly if there is scaling in the matrix. */
	FORCEINLINE SVector InverseTransformNormalNoScale(const SVector& V) const;

	// Transpose.

	FORCEINLINE SMatrix Transpose() const;

	// Determinant.

	inline float Determinant() const;

	/** Calculate determinant of rotation 3x3 matrix */
	inline float RotDeterminant() const;

	// Inverse.
	/** Fast path, doesn't check for nil matrices in final release builds */
	inline SMatrix Inverse() const;
	/** Fast path, and handles nil matrices. */
	inline SMatrix InverseSafe() const;
	/** Slow and safe path */
	inline SMatrix InverseSlow() const;

	inline SMatrix TransposeAdjoint() const;

	// Remove any scaling from this matrix (ie magnitude of each row is 1)
	inline void RemoveScaling(float Tolerance = SMALL_NUMBER);

	// Returns matrix after RemoveScaling
	inline SMatrix GetMatrixWithoutScale(float Tolerance = SMALL_NUMBER) const;

	/** Remove any scaling from this matrix (ie magnitude of each row is 1) and return the 3D scale vector that was initially present. */
	inline SVector ExtractScaling(float Tolerance = SMALL_NUMBER);

	/** return a 3D scale vector calculated from this matrix (where each component is the magnitude of a row vector). */
	inline SVector GetScaleVector(float Tolerance = SMALL_NUMBER) const;

	// Remove any translation from this matrix
	inline SMatrix RemoveTranslation() const;

	/** Returns a matrix with an additional translation concatenated. */
	inline SMatrix ConcatTranslation(const SVector& Translation) const;

	/** Returns TRUE if any element of this matrix is NaN */
	inline BOOL ContainsNaN() const;

	inline void ScaleTranslation(const SVector& Scale3D);

	/** @return the maximum magnitude of any row of the matrix. */
	inline float GetMaximumAxisScale() const;

	/** Apply Scale to this matrix **/
	inline SMatrix ApplyScale(float Scale);
	// GetOrigin

	inline SVector GetOrigin() const;

	inline SVector GetAxis(INT i) const;

	inline void GetAxes(SVector& X, SVector& Y, SVector& Z) const;

	inline void SetAxis(INT i, const SVector& Axis);

	inline void SetOrigin(const SVector& NewOrigin);

	inline void SetAxes(SVector* Axis0 = NULL, SVector* Axis1 = NULL, SVector* Axis2 = NULL, SVector* Origin = NULL);

	inline SVector GetColumn(INT i) const;

	SRotator Rotator() const;

	//SQuat ToQuat() const;

	// Frustum plane extraction.
	FORCEINLINE BOOL GetFrustumNearPlane(SPlane& OutPlane) const;

	FORCEINLINE BOOL GetFrustumFarPlane(SPlane& OutPlane) const;

	FORCEINLINE BOOL GetFrustumLeftPlane(SPlane& OutPlane) const;

	FORCEINLINE BOOL GetFrustumRightPlane(SPlane& OutPlane) const;

	FORCEINLINE BOOL GetFrustumTopPlane(SPlane& OutPlane) const;

	FORCEINLINE BOOL GetFrustumBottomPlane(SPlane& OutPlane) const;

	/**
	* Utility for mirroring this transform across a certain plane,
	* and flipping one of the axis as well.
	*/
	inline void Mirror(UINT8 MirrorAxis, UINT8 FlipAxis);
};

struct SPerspectiveMatrix : public SMatrix
{
public:

	// Note: the value of this must match the mirror in Common.usf!
#define Z_PRECISION	0.001f

	SPerspectiveMatrix(float HalfFOVX, float HalfFOVY, float MultFOVX, float MultFOVY, float MinZ, float MaxZ) :
		SMatrix(
			SPlane(MultFOVX / appTan(HalfFOVX), 0.0f, 0.0f, 0.0f),
			SPlane(0.0f, MultFOVY / appTan(HalfFOVY), 0.0f, 0.0f),
			SPlane(0.0f, 0.0f, ((MinZ == MaxZ) ? (1.0f - Z_PRECISION) : MaxZ / (MaxZ - MinZ)), 1.0f),
			SPlane(0.0f, 0.0f, -MinZ * ((MinZ == MaxZ) ? (1.0f - Z_PRECISION) : MaxZ / (MaxZ - MinZ)), 0.0f))
	{
	}

	/** Note that the FOV you pass in is actually half the FOV, unlike most perspective matrix functions (D3DXMatrixPerspectiveFovLH). */
	SPerspectiveMatrix(float HalfFOV, float Width, float Height, float MinZ, float MaxZ) :
		SMatrix(
			SPlane(1.0f / appTan(HalfFOV), 0.0f, 0.0f, 0.0f),
			SPlane(0.0f, Width / appTan(HalfFOV) / Height, 0.0f, 0.0f),
			SPlane(0.0f, 0.0f, MaxZ / (MaxZ - MinZ), 1.0f),
			SPlane(0.0f, 0.0f, -MinZ * (MaxZ / (MaxZ - MinZ)), 0.0f))
	{
	}

	SPerspectiveMatrix(float HalfFOV, float Width, float Height, float MinZ) :
		SMatrix(
			SPlane(1.0f / appTan(HalfFOV), 0.0f, 0.0f, 0.0f),
			SPlane(0.0f, Width / appTan(HalfFOV) / Height, 0.0f, 0.0f),
			SPlane(0.0f, 0.0f, (1.0f - Z_PRECISION), 1.0f),
			SPlane(0.0f, 0.0f, -MinZ * (1.0f - Z_PRECISION), 0.0f))
	{
	}

};

struct SOrthoMatrix : public SMatrix
{
public:

	SOrthoMatrix(float Width, float Height, float ZScale, float ZOffset) :
		SMatrix(
			SPlane(1.0f / Width, 0.0f, 0.0f, 0.0f),
			SPlane(0.0f, 1.0f / Height, 0.0f, 0.0f),
			SPlane(0.0f, 0.0f, ZScale, 0.0f),
			SPlane(0.0f, 0.0f, ZOffset* ZScale, 1.0f))
	{
	}
};

struct STranslationMatrix : public SMatrix
{
public:

	STranslationMatrix(const SVector& Delta) :
		SMatrix(
			SPlane(1.0f, 0.0f, 0.0f, 0.0f),
			SPlane(0.0f, 1.0f, 0.0f, 0.0f),
			SPlane(0.0f, 0.0f, 1.0f, 0.0f),
			SPlane(Delta.X, Delta.Y, Delta.Z, 1.0f))
	{
	}
};

struct SRotationTranslationMatrix : public SMatrix
{
public:
	SRotationTranslationMatrix(const SRotator& Rot, const SVector& Origin)
	{
		const float	SR = g_SGlobalMath.SinTab(Rot.Roll);
		const float	SP = g_SGlobalMath.SinTab(Rot.Pitch);
		const float	SY = g_SGlobalMath.SinTab(Rot.Yaw);
		const float	CR = g_SGlobalMath.CosTab(Rot.Roll);
		const float	CP = g_SGlobalMath.CosTab(Rot.Pitch);
		const float	CY = g_SGlobalMath.CosTab(Rot.Yaw);

		M[0][0] = CP * CY;
		M[0][1] = CP * SY;
		M[0][2] = SP;
		M[0][3] = 0.f;

		M[1][0] = SR * SP * CY - CR * SY;
		M[1][1] = SR * SP * SY + CR * CY;
		M[1][2] = -SR * CP;
		M[1][3] = 0.f;

		M[2][0] = -(CR * SP * CY + SR * SY);
		M[2][1] = CY * SR - CR * SP * SY;
		M[2][2] = CR * CP;
		M[2][3] = 0.f;

		M[3][0] = Origin.X;
		M[3][1] = Origin.Y;
		M[3][2] = Origin.Z;
		M[3][3] = 1.f;
	}
};

struct SRotationMatrix : public SRotationTranslationMatrix
{
public:
	SRotationMatrix(const SRotator& Rot) : SRotationTranslationMatrix(Rot, SVector::ZeroVector)
	{}
};

struct SScaleRotationTranslationMatrix : public SMatrix
{
public:
	SScaleRotationTranslationMatrix(const SVector& Scale, const SRotator& Rot, const SVector& Origin)
	{
		const float	SR = g_SGlobalMath.SinTab(Rot.Roll);
		const float	SP = g_SGlobalMath.SinTab(Rot.Pitch);
		const float	SY = g_SGlobalMath.SinTab(Rot.Yaw);
		const float	CR = g_SGlobalMath.CosTab(Rot.Roll);
		const float	CP = g_SGlobalMath.CosTab(Rot.Pitch);
		const float	CY = g_SGlobalMath.CosTab(Rot.Yaw);

		M[0][0] = (CP * CY) * Scale.X;
		M[0][1] = (CP * SY) * Scale.X;
		M[0][2] = (SP)*Scale.X;
		M[0][3] = 0.f;

		M[1][0] = (SR * SP * CY - CR * SY) * Scale.Y;
		M[1][1] = (SR * SP * SY + CR * CY) * Scale.Y;
		M[1][2] = (-SR * CP) * Scale.Y;
		M[1][3] = 0.f;

		M[2][0] = (-(CR * SP * CY + SR * SY)) * Scale.Z;
		M[2][1] = (CY * SR - CR * SP * SY) * Scale.Z;
		M[2][2] = (CR * CP) * Scale.Z;
		M[2][3] = 0.f;

		M[3][0] = Origin.X;
		M[3][1] = Origin.Y;
		M[3][2] = Origin.Z;
		M[3][3] = 1.f;
	}
};

struct SQuatRotationTranslationMatrix : public SMatrix
{
public:
	SQuatRotationTranslationMatrix(const SQuat& Q, const SVector& Origin)
	{
		const float x2 = Q.X + Q.X;  const float y2 = Q.Y + Q.Y;  const float z2 = Q.Z + Q.Z;
		const float xx = Q.X * x2;   const float xy = Q.X * y2;   const float xz = Q.X * z2;
		const float yy = Q.Y * y2;   const float yz = Q.Y * z2;   const float zz = Q.Z * z2;
		const float wx = Q.W * x2;   const float wy = Q.W * y2;   const float wz = Q.W * z2;

		M[0][0] = 1.0f - (yy + zz);	M[1][0] = xy - wz;				M[2][0] = xz + wy;			M[3][0] = Origin.X;
		M[0][1] = xy + wz;			M[1][1] = 1.0f - (xx + zz);		M[2][1] = yz - wx;			M[3][1] = Origin.Y;
		M[0][2] = xz - wy;			M[1][2] = yz + wx;				M[2][2] = 1.0f - (xx + yy);	M[3][2] = Origin.Z;
		M[0][3] = 0.0f;				M[1][3] = 0.0f;					M[2][3] = 0.0f;				M[3][3] = 1.0f;
	}
};

struct SInverseRotationMatrix : public SMatrix
{
public:
	//	6/25/09 - Was changed because CosTab and SinTab provide wrong results for negative numbers.
	// see https://udn.epicgames.com/lists/showpost.php?list=ue3bugs&id=12074
	// workaround implementation below.
	//
	// 	SInverseRotationMatrix(const SRotator& Rot) :
	// 		SMatrix(
	// 			SMatrix(	// Yaw
	// 				SPlane(+g_SGlobalMath.CosTab(-Rot.Yaw),	+g_SGlobalMath.SinTab(-Rot.Yaw), 0.0f,	0.0f),
	// 				SPlane(-g_SGlobalMath.SinTab(-Rot.Yaw),	+g_SGlobalMath.CosTab(-Rot.Yaw), 0.0f,	0.0f),
	// 				SPlane(0.0f,					0.0f,					1.0f,	0.0f),
	// 				SPlane(0.0f,					0.0f,					0.0f,	1.0f)) *
	// 			SMatrix(	// Pitch
	// 				SPlane(+g_SGlobalMath.CosTab(-Rot.Pitch),0.0f,					+g_SGlobalMath.SinTab(-Rot.Pitch),	0.0f),
	// 				SPlane(0.0f,					1.0f,					0.0f,						0.0f),
	// 				SPlane(-g_SGlobalMath.SinTab(-Rot.Pitch),0.0f,					+g_SGlobalMath.CosTab(-Rot.Pitch),	0.0f),
	// 				SPlane(0.0f,					0.0f,					0.0f,						1.0f)) *
	// 			SMatrix(	// Roll
	// 				SPlane(1.0f,					0.0f,					0.0f,						0.0f),
	// 				SPlane(0.0f,					+g_SGlobalMath.CosTab(-Rot.Roll),-g_SGlobalMath.SinTab(-Rot.Roll),	0.0f),
	// 				SPlane(0.0f,					+g_SGlobalMath.SinTab(-Rot.Roll),+g_SGlobalMath.CosTab(-Rot.Roll),	0.0f),
	// 				SPlane(0.0f,					0.0f,					0.0f,						1.0f))
	// 			)
	// 	{
	// 	}

	SInverseRotationMatrix(const SRotator& Rot) :
		SMatrix(
			SMatrix( // Yaw
				SPlane(+g_SGlobalMath.CosTab(Rot.Yaw), -g_SGlobalMath.SinTab(Rot.Yaw), 0.0f, 0.0f),
				SPlane(+g_SGlobalMath.SinTab(Rot.Yaw), +g_SGlobalMath.CosTab(Rot.Yaw), 0.0f, 0.0f),
				SPlane(0.0f, 0.0f, 1.0f, 0.0f),
				SPlane(0.0f, 0.0f, 0.0f, 1.0f))*
			SMatrix( // Pitch
				SPlane(+g_SGlobalMath.CosTab(Rot.Pitch), 0.0f, -g_SGlobalMath.SinTab(Rot.Pitch), 0.0f),
				SPlane(0.0f, 1.0f, 0.0f, 0.0f),
				SPlane(+g_SGlobalMath.SinTab(Rot.Pitch), 0.0f, +g_SGlobalMath.CosTab(Rot.Pitch), 0.0f),
				SPlane(0.0f, 0.0f, 0.0f, 1.0f))*
			SMatrix( // Roll
				SPlane(1.0f, 0.0f, 0.0f, 0.0f),
				SPlane(0.0f, +g_SGlobalMath.CosTab(Rot.Roll), +g_SGlobalMath.SinTab(Rot.Roll), 0.0f),
				SPlane(0.0f, -g_SGlobalMath.SinTab(Rot.Roll), +g_SGlobalMath.CosTab(Rot.Roll), 0.0f),
				SPlane(0.0f, 0.0f, 0.0f, 1.0f))
			)
	{
	}

};

struct SScaleMatrix : public SMatrix
{
public:

	/**
	* Uniform scale.
	*/
	SScaleMatrix(float Scale) :
		SMatrix(
			SPlane(Scale, 0.0f, 0.0f, 0.0f),
			SPlane(0.0f, Scale, 0.0f, 0.0f),
			SPlane(0.0f, 0.0f, Scale, 0.0f),
			SPlane(0.0f, 0.0f, 0.0f, 1.0f))
	{
	}

	/**
	* Non-uniform scale.
	*/
	SScaleMatrix(const SVector& Scale) :
		SMatrix(
			SPlane(Scale.X, 0.0f, 0.0f, 0.0f),
			SPlane(0.0f, Scale.Y, 0.0f, 0.0f),
			SPlane(0.0f, 0.0f, Scale.Z, 0.0f),
			SPlane(0.0f, 0.0f, 0.0f, 1.0f))
	{
	}
};

struct SBasisVectorMatrix : SMatrix
{
	SBasisVectorMatrix(const SVector& XAxis, const SVector& YAxis, const SVector& ZAxis, const SVector& Origin)
	{
		for (UINT RowIndex = 0; RowIndex < 3; RowIndex++)
		{
			M[RowIndex][0] = (&XAxis.X)[RowIndex];
			M[RowIndex][1] = (&YAxis.X)[RowIndex];
			M[RowIndex][2] = (&ZAxis.X)[RowIndex];
			M[RowIndex][3] = 0.0f;
		}
		M[3][0] = Origin | XAxis;
		M[3][1] = Origin | YAxis;
		M[3][2] = Origin | ZAxis;
		M[3][3] = 1.0f;
	}
};

struct SLookAtMatrix : SMatrix
{
	/**
	* Creates a view matrix given an eye position, a position to look at, and an up vector.
	* This does the same thing as D3DXMatrixLookAtLH.
	*/
	SLookAtMatrix(const SVector& EyePosition, const SVector& LookAtPosition, const SVector& UpVector)
	{
		const SVector ZAxis = (LookAtPosition - EyePosition).SafeNormal();
		const SVector XAxis = (UpVector ^ ZAxis).SafeNormal();
		const SVector YAxis = ZAxis ^ XAxis;

		for (UINT RowIndex = 0; RowIndex < 3; RowIndex++)
		{
			M[RowIndex][0] = (&XAxis.X)[RowIndex];
			M[RowIndex][1] = (&YAxis.X)[RowIndex];
			M[RowIndex][2] = (&ZAxis.X)[RowIndex];
			M[RowIndex][3] = 0.0f;
		}
		M[3][0] = -EyePosition | XAxis;
		M[3][1] = -EyePosition | YAxis;
		M[3][2] = -EyePosition | ZAxis;
		M[3][3] = 1.0f;
	}
};

struct SMirrorMatrix : public SMatrix
{
public:
	/**
	* Constructor.
	* Updated for the fact that our SPlane uses Ax+By+Cz=D.
	*
	* @param	Plane - source plane for mirroring (assumed normalized)
	*/
	SMirrorMatrix(const SPlane& Plane) :
		SMatrix(
			SPlane(-2.f * Plane.X * Plane.X + 1.f, -2.f * Plane.Y * Plane.X, -2.f * Plane.Z * Plane.X, 0.f),
			SPlane(-2.f * Plane.X * Plane.Y, -2.f * Plane.Y * Plane.Y + 1.f, -2.f * Plane.Z * Plane.Y, 0.f),
			SPlane(-2.f * Plane.X * Plane.Z, -2.f * Plane.Y * Plane.Z, -2.f * Plane.Z * Plane.Z + 1.f, 0.f),
			SPlane(2.f * Plane.X * Plane.W, 2.f * Plane.Y * Plane.W, 2.f * Plane.Z * Plane.W, 1.f))
	{
		//check( Abs(1.f - Plane.SizeSquared()) < KINDA_SMALL_NUMBER && TEXT("not normalized"));
	}
};

struct SClipProjectionMatrix : public SMatrix
{
public:
	/**
	* Constructor
	*
	* @param	SrcProjMat - source projection matrix to premultiply with the clip matrix
	* @param	Plane - clipping plane used to build the clip matrix (assumed to be in camera space)
	*/
	SClipProjectionMatrix(const SMatrix& SrcProjMat, const SPlane& Plane) :
		SMatrix(SrcProjMat)
	{
		// Calculate the clip-space corner point opposite the clipping plane
		// as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
		// transform it into camera space by multiplying it
		// by the inverse of the projection matrix
		SPlane CornerPlane(
			sgn(Plane.X) / SrcProjMat.M[0][0],
			sgn(Plane.Y) / SrcProjMat.M[1][1],
			1.0f,
			-(1.0f - SrcProjMat.M[2][2]) / SrcProjMat.M[3][2]
			);

		// Calculate the scaled plane vector
		SPlane ProjPlane(Plane * (1.0f / (Plane | CornerPlane)));

		// use the projected space clip plane in z column 
		// Note: (account for our negated W coefficient)
		M[0][2] = ProjPlane.X;
		M[1][2] = ProjPlane.Y;
		M[2][2] = ProjPlane.Z;
		M[3][2] = -ProjPlane.W;
	}

private:
	/** return sign of a number */
	FORCEINLINE float sgn(float a)
	{
		if (a > 0.0f) return (1.0f);
		if (a < 0.0f) return (-1.0f);
		return (0.0f);
	}
};

/*-----------------------------------------------------------------------------
	More global functions
-----------------------------------------------------------------------------*/

FORCEINLINE BOOL MakeFrustumPlane(float A, float B, float C, float D, SPlane& OutPlane)
{
	const float	LengthSquared = A * A + B * B + C * C;
	if (LengthSquared > DELTA * DELTA)
	{
		const float	InvLength = appInvSqrt(LengthSquared);
		OutPlane = SPlane(-A * InvLength, -B * InvLength, -C * InvLength, D * InvLength);
		return 1;
	}
	else
		return 0;
}

/** Same as above, but taking a 2d vector as the range. */
inline float GetRangePct(SVector2D const& Range, float Value)
{
	return (Range.X != Range.Y) ? (Value - Range.X) / (Range.Y - Range.X) : Range.X;
}

/** Basically a Vector2d version of Lerp. */
inline float GetRangeValue(SVector2D const& Range, float Pct)
{
	return LerpF(Range.X, Range.Y, Pct);
}

/** Find the point on line segment from LineStart to LineEnd which is closest to Point */
SVector ClosestPointOnLine(const SVector& LineStart, const SVector& LineEnd, const SVector& Point);

/**
* Add to a word angle, constraining it within a min (not to cross)
* and a max (not to cross).  Accounts for funkyness of word angles.
* Assumes that angle is initially in the desired range.
*/
inline INT16 SAddAngleConfined(INT Angle, INT Delta, INT MinThresh, INT MaxThresh)
{
	if (Delta < 0)
	{
		if (Delta <= -0x10000L || Delta <= -(INT)((INT16)(Angle - MinThresh)))
			return MinThresh;
	}
	else if (Delta > 0)
	{
		if (Delta >= 0x10000L || Delta >= (INT)((INT16)(MaxThresh - Angle)))
			return MaxThresh;
	}
	return (INT16)(Angle + Delta);
}

/** Converts given Cartesian coordinate pair to Polar coordinate system. */
inline void CartesianToPolar(float X, float Y, float& OutRad, float& OutAng)
{
	OutRad = appSqrt(Square(X) + Square(Y));
	OutAng = appAtan2(Y, X);
}

/** Converts given Polar coordinate pair to Cartesian coordinate system. */
inline void PolarToCartesian(float Rad, float Ang, float& OutX, float& OutY)
{
	OutX = Rad * appCos(Ang);
	OutY = Rad * appSin(Ang);
}

/** Interpolate a normal vector Current to Target, by interpolating the angle between those vectors with constant step. */
SVector VInterpNormalRotationTo(const SVector& Current, const SVector& Target, float DeltaTime, float RotationSpeedDegrees);

/** Interpolate a normal vector from Current to Target with constant step */
SVector VInterpNormalConstantTo(const SVector Current, const SVector& Target, float DeltaTime, float InterpSpeed);

/** Interpolate vector from Current to Target with constant step */
SVector VInterpConstantTo(const SVector Current, const SVector& Target, float DeltaTime, float InterpSpeed);

/** Interpolate vector from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
SVector VInterpTo(const SVector& Current, const SVector& Target, float& DeltaTime, float InterpSpeed);
SRotator RInterpTo(const SRotator& Current, const SRotator& Target, float& DeltaTime, float InterpSpeed, BOOL bConstantInterpSpeed = FALSE);

float SInterpTo(float Current, float Target, float DeltaTime, float InterpSpeed);
float SInterpConstantTo(float Current, float Target, float DeltaTime, float InterpSpeed);
float SInterpEaseInOut(float A, float B, float Alpha, float Exp);

/** Clamp of Vector A From Min to Max of XYZ **/
SVector VClamp(SVector A, SVector Min, SVector Max);

inline INT ReduceAngle(INT Angle)
{
	return Angle & SGlobalMath::ANGLE_MASK;
};

/** Convert a direction vector into a 'heading' angle between +/-PI. 0 is pointing down +X. */
inline float HeadingAngle(SVector Dir)
{
	// Project Dir into Z plane.
	SVector PlaneDir = Dir;
	PlaneDir.Z = 0.f;
	PlaneDir = PlaneDir.SafeNormal();

	float Angle = appAcos(PlaneDir.X);

	if (PlaneDir.Y < 0.0f)
	{
		Angle *= -1.0f;
	}

	return Angle;
}

/** Find the smallest angle between two headings (in radians) */
inline float FindDeltaAngle(float A1, float A2)
{
	// Find the difference
	float Delta = A2 - A1;

	// If change is larger than PI
	if (Delta > PI)
	{
		// Flip to negative equivalent
		Delta = Delta - (PI * 2.0f);
	}
	else if (Delta < -PI)
	{
		// Otherwise, if change is smaller than -PI
		// Flip to positive equivalent
		Delta = Delta + (PI * 2.0f);
	}

	// Return delta in [-PI,PI] range
	return Delta;
}

/** Given a heading which may be outside the +/- PI range, 'unwind' it back into that range. */
inline float UnwindHeading(float A)
{
	while (A > PI)
	{
		A -= ((float)PI * 2.0f);
	}

	while (A < -PI)
	{
		A += ((float)PI * 2.0f);
	}

	return A;
}

/*-----------------------------------------------------------------------------
Floating point constants.
-----------------------------------------------------------------------------*/

/**
* Lengths of normalized vectors (These are half their maximum values
* to assure that dot products with normalized vectors don't overflow).
*/
#define FLOAT_NORMAL_THRESH				(0.0001f)

//
// Magic numbers for numerical precision.
//
#define THRESH_POINT_ON_PLANE			(0.10f)		/* Thickness of plane for front/back/inside test */
#define THRESH_POINT_ON_SIDE			(0.20f)		/* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define THRESH_POINTS_ARE_SAME			(0.002f)	/* Two points are same if within this distance */
#define THRESH_POINTS_ARE_NEAR			(0.015f)	/* Two points are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_NORMALS_ARE_SAME			(0.00002f)	/* Two normal points are same if within this distance */
/* Making this too large results in incorrect CSG classification and disaster */
#define THRESH_VECTORS_ARE_NEAR			(0.0004f)	/* Two vectors are near if within this distance and can be combined if imprecise math is ok */
/* Making this too large results in lighting problems due to inaccurate texture coordinates */
#define THRESH_SPLIT_POLY_WITH_PLANE	(0.25f)		/* A plane splits a polygon in half */
#define THRESH_SPLIT_POLY_PRECISELY		(0.01f)		/* A plane exactly splits a polygon */
#define THRESH_ZERO_NORM_SQUARED		(0.0001f)	/* Size of a unit normal that is considered "zero", squared */
#define THRESH_VECTORS_ARE_PARALLEL		(0.02f)		/* Vectors are parallel if dot product varies less than this */


/**
* Compare two points and see if they're the same, using a threshold.
* Returns 1=yes, 0=no.  Uses fast distance approximation.
*/
inline BOOL SPointsAreSame(const SVector& P, const SVector& Q)
{
	float Temp;
	Temp = P.X - Q.X;
	if ((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
	{
		Temp = P.Y - Q.Y;
		if ((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
		{
			Temp = P.Z - Q.Z;
			if ((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
			{
				return 1;
			}
		}
	}
	return 0;
}

/**
* Compare two points and see if they're the same, using a threshold.
* Returns 1=yes, 0=no.  Uses fast distance approximation.
*/
inline BOOL SPointsAreNear(const SVector& Point1, const SVector& Point2, float Dist)
{
	float Temp;
	Temp = (Point1.X - Point2.X); if (Abs(Temp) >= Dist) return 0;
	Temp = (Point1.Y - Point2.Y); if (Abs(Temp) >= Dist) return 0;
	Temp = (Point1.Z - Point2.Z); if (Abs(Temp) >= Dist) return 0;
	return 1;
}

/**
* Calculate the signed distance (in the direction of the normal) between
* a point and a plane.
*/
inline float SPointPlaneDist
(
	const SVector& Point,
	const SVector& PlaneBase,
	const SVector& PlaneNormal
	)
{
	return (Point - PlaneBase) | PlaneNormal;
}

/**
* Calculate a the projection of a point on the plane defined by CCW points A,B,C
* @param Point - the point to project onto the plane
* @param A,B,C - three points in CCW order defining the plane
*
* @return Projection of Point onto plane ABC
*/
inline SVector SPointPlaneProject(const SVector& Point, const SVector& A, const SVector& B, const SVector& C)
{
	//Compute the plane normal from ABC
	SPlane Plane(A, B, C);

	//Find the distance of X from the plane
	//Add the distance back along the normal from the point
	return Point - Plane * Plane.PlaneDot(Point);
}

/**
* Calculate a the projection of a point on the plane defined by PlaneBase, and PlaneNormal
* @param Point - the point to project onto the plane
* @param PlaneBase - point on the plane
* @param PlaneNorm - normal of the plane
*
* @return Projection of Point onto plane ABC
*/
inline SVector SPointPlaneProject(const SVector& Point, const SVector& PlaneBase, const SVector& PlaneNorm)
{
	//Find the distance of X from the plane
	//Add the distance back along the normal from the point
	return Point - PlaneNorm * SPointPlaneDist(Point, PlaneBase, PlaneNorm);
}

/**
* Euclidean distance between two points.
*/
inline float SDist(const SVector& V1, const SVector& V2)
{
	return appSqrt(Square(V2.X - V1.X) + Square(V2.Y - V1.Y) + Square(V2.Z - V1.Z));
}

/**
* Squared distance between two points.
*/
inline float SDistSquared(const SVector& V1, const SVector& V2)
{
	return Square(V2.X - V1.X) + Square(V2.Y - V1.Y) + Square(V2.Z - V1.Z);
}

/**
* See if two normal vectors (or plane normals) are nearly parallel.
*/
inline BOOL SParallel(const SVector& Normal1, const SVector& Normal2)
{
	const float NormalDot = Normal1 | Normal2;
	return (Abs(NormalDot - 1.f) <= THRESH_VECTORS_ARE_PARALLEL);
}

/**
* See if two planes are coplanar.
*/
inline BOOL SCoplanar(const SVector& Base1, const SVector& Normal1, const SVector& Base2, const SVector& Normal2)
{
	if (!SParallel(Normal1, Normal2)) return 0;
	else if (SPointPlaneDist(Base2, Base1, Normal1) > THRESH_POINT_ON_PLANE) return 0;
	else    return 1;
}

/**
* Triple product of three vectors.
*/
inline float STriple(const SVector& X, const SVector& Y, const SVector& Z)
{
	return
		((X.X * (Y.Y * Z.Z - Y.Z * Z.Y))
			+ (X.Y * (Y.Z * Z.X - Y.X * Z.Z))
			+ (X.Z * (Y.X * Z.Y - Y.Y * Z.X)));
}

/**
* Compute pushout of a box from a plane.
*/
inline float SBoxPushOut(const SVector& Normal, const SVector& Size)
{
	return Abs(Normal.X * Size.X) + Abs(Normal.Y * Size.Y) + Abs(Normal.Z * Size.Z);
}

/*-----------------------------------------------------------------------------
Random numbers.
-----------------------------------------------------------------------------*/

/**
* Return a uniformly distributed random unit vector.
*/
inline SVector VRand()
{
	SVector Result;
	do
	{
		// Check random vectors in the unit sphere so result is statistically uniform.
		Result.X = appFrand() * 2 - 1;
		Result.Y = appFrand() * 2 - 1;
		Result.Z = appFrand() * 2 - 1;
	} while (Result.SizeSquared() > 1.f);
	return Result.UnsafeNormal();
}

/**
* Returns a random unit vector, uniformly distributed, within the specified cone.
*/
SVector VRandCone(SVector const& Dir, float ConeHalfAngleRad);

/**
* A version of VRandCone that supports squashed cones.
*/
SVector VRandCone(SVector const& Dir, float HorizontalConeHalfAngleRad, float VerticalConeHalfAngleRad);

/*-----------------------------------------------------------------------------
Advanced geometry.
-----------------------------------------------------------------------------*/

/**
* Find the intersection of an infinite line (defined by two points) and
* a plane.  Assumes that the line and plane do indeed intersect; you must
* make sure they're not parallel before calling.
*/
inline SVector SLinePlaneIntersection
(
	const SVector& Point1,
	const SVector& Point2,
	const SVector& PlaneOrigin,
	const SVector& PlaneNormal
	)
{
	return
		Point1
		+ (Point2 - Point1)
		* (((PlaneOrigin - Point1) | PlaneNormal) / ((Point2 - Point1) | PlaneNormal));
}
inline SVector SLinePlaneIntersection
(
	const SVector& Point1,
	const SVector& Point2,
	const SPlane& Plane
	)
{
	return
		Point1
		+ (Point2 - Point1)
		* ((Plane.W - (Point1 | Plane)) / ((Point2 - Point1) | Plane));
}

/**
* Determine if a plane and an AABB intersect
* @param P - the plane to test
* @param AABB - the axis aligned bounding box to test
* @return if collision occurs
*/
BOOL SPlaneAABBIsect(const SPlane& P, const SBox& AABB);

/**
* Determines whether a point is inside a box.
*/
inline BOOL SPointBoxIntersection
(
	const SVector& Point,
	const SBox& Box
	)
{
	if (Point.X >= Box.Min.X && Point.X <= Box.Max.X &&
		Point.Y >= Box.Min.Y && Point.Y <= Box.Max.Y &&
		Point.Z >= Box.Min.Z && Point.Z <= Box.Max.Z)
		return 1;
	else
		return 0;
}

/**
* Determines whether a line intersects a box.
*/
inline BOOL SLineBoxIntersection
(
	const SBox& Box,
	const SVector& Start,
	const SVector& End,
	const SVector& Direction,
	const SVector& OneOverDirection
	)
{
	SVector	Time;
	BOOL	bStartIsOutside = FALSE;

	if (Start.X < Box.Min.X)
	{
		bStartIsOutside = TRUE;
		if (End.X >= Box.Min.X)
		{
			Time.X = (Box.Min.X - Start.X) * OneOverDirection.X;
		}
		else
		{
			return FALSE;
		}
	}
	else if (Start.X > Box.Max.X)
	{
		bStartIsOutside = TRUE;
		if (End.X <= Box.Max.X)
		{
			Time.X = (Box.Max.X - Start.X) * OneOverDirection.X;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.X = 0.0f;
	}

	if (Start.Y < Box.Min.Y)
	{
		bStartIsOutside = TRUE;
		if (End.Y >= Box.Min.Y)
		{
			Time.Y = (Box.Min.Y - Start.Y) * OneOverDirection.Y;
		}
		else
		{
			return FALSE;
		}
	}
	else if (Start.Y > Box.Max.Y)
	{
		bStartIsOutside = TRUE;
		if (End.Y <= Box.Max.Y)
		{
			Time.Y = (Box.Max.Y - Start.Y) * OneOverDirection.Y;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.Y = 0.0f;
	}

	if (Start.Z < Box.Min.Z)
	{
		bStartIsOutside = TRUE;
		if (End.Z >= Box.Min.Z)
		{
			Time.Z = (Box.Min.Z - Start.Z) * OneOverDirection.Z;
		}
		else
		{
			return FALSE;
		}
	}
	else if (Start.Z > Box.Max.Z)
	{
		bStartIsOutside = TRUE;
		if (End.Z <= Box.Max.Z)
		{
			Time.Z = (Box.Max.Z - Start.Z) * OneOverDirection.Z;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		Time.Z = 0.0f;
	}

	if (bStartIsOutside)
	{
		const float	MaxTime = Max(Time.X, Max(Time.Y, Time.Z));

		if (MaxTime >= 0.0f && MaxTime <= 1.0f)
		{
			const SVector Hit = Start + Direction * MaxTime;
			const float BOX_SIDE_THRESHOLD = 0.1f;
			if (Hit.X > Box.Min.X - BOX_SIDE_THRESHOLD && Hit.X < Box.Max.X + BOX_SIDE_THRESHOLD &&
				Hit.Y > Box.Min.Y - BOX_SIDE_THRESHOLD && Hit.Y < Box.Max.Y + BOX_SIDE_THRESHOLD &&
				Hit.Z > Box.Min.Z - BOX_SIDE_THRESHOLD && Hit.Z < Box.Max.Z + BOX_SIDE_THRESHOLD)
			{
				return TRUE;
			}
		}

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL SLineExtentBoxIntersection(const SBox& inBox,
	const SVector& Start,
	const SVector& End,
	const SVector& Extent,
	SVector& HitLocation,
	SVector& HitNormal,
	float& HitTime);

/**
* Determines whether a line intersects a sphere.
*/
inline BOOL SLineSphereIntersection(const SVector& Start, const SVector& Dir, float Length, const SVector& Origin, float Radius)
{
	const SVector	EO = Start - Origin;
	const float		v = (Dir | (Origin - Start));
	const float		disc = Radius * Radius - ((EO | EO) - v * v);

	if (disc >= 0.0f)
	{
		const float	Time = (v - appSqrt(disc)) / Length;

		if (Time >= 0.0f && Time <= 1.0f)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

/*-----------------------------------------------------------------------------
SPlane functions.
-----------------------------------------------------------------------------*/

/**
* Compute intersection point of three planes. Return 1 if valid, 0 if infinite.
*/
inline BOOL SIntersectPlanes3(SVector& I, const SPlane& P1, const SPlane& P2, const SPlane& P3)
{
	// Compute determinant, the triple product P1|(P2^P3)==(P1^P2)|P3.
	const float Det = (P1 ^ P2) | P3;
	if (Square(Det) < Square(0.001f))
	{
		// Degenerate.
		I = SVector(0, 0, 0);
		return 0;
	}
	else
	{
		// Compute the intersection point, guaranteed valid if determinant is nonzero.
		I = ((P2 ^ P3) * P1.W + (P3 ^ P1) * P2.W + (P1 ^ P2) * P3.W) / Det;
	}
	return 1;
}

/**
* Compute intersection point and direction of line joining two planes.
* Return 1 if valid, 0 if infinite.
*/
inline BOOL SIntersectPlanes2(SVector& I, SVector& D, const SPlane& P1, const SPlane& P2)
{
	// Compute line direction, perpendicular to both plane normals.
	D = P1 ^ P2;
	const float DD = D.SizeSquared();
	if (DD < Square(0.001f))
	{
		// Parallel or nearly parallel planes.
		D = I = SVector(0, 0, 0);
		return 0;
	}
	else
	{
		// Compute intersection.
		I = ((P2 ^ D) * P1.W + (D ^ P1) * P2.W) / DD;
		D.Normalize();
		return 1;
	}
}


/**
* SMatrix inline functions.
*/

// Constructors.

FORCEINLINE SMatrix::SMatrix()
{
}

FORCEINLINE SMatrix::SMatrix(const SPlane& InX, const SPlane& InY, const SPlane& InZ, const SPlane& InW)
{
	M[0][0] = InX.X; M[0][1] = InX.Y;  M[0][2] = InX.Z;  M[0][3] = InX.W;
	M[1][0] = InY.X; M[1][1] = InY.Y;  M[1][2] = InY.Z;  M[1][3] = InY.W;
	M[2][0] = InZ.X; M[2][1] = InZ.Y;  M[2][2] = InZ.Z;  M[2][3] = InZ.W;
	M[3][0] = InW.X; M[3][1] = InW.Y;  M[3][2] = InW.Z;  M[3][3] = InW.W;
}

FORCEINLINE SMatrix::SMatrix(const SVector& InX, const SVector& InY, const SVector& InZ, const SVector& InW)
{
	M[0][0] = InX.X; M[0][1] = InX.Y;  M[0][2] = InX.Z;  M[0][3] = 0.0f;
	M[1][0] = InY.X; M[1][1] = InY.Y;  M[1][2] = InY.Z;  M[1][3] = 0.0f;
	M[2][0] = InZ.X; M[2][1] = InZ.Y;  M[2][2] = InZ.Z;  M[2][3] = 0.0f;
	M[3][0] = InW.X; M[3][1] = InW.Y;  M[3][2] = InW.Z;  M[3][3] = 1.0f;
}

inline void SMatrix::SetIdentity()
{
	M[0][0] = 1; M[0][1] = 0;  M[0][2] = 0;  M[0][3] = 0;
	M[1][0] = 0; M[1][1] = 1;  M[1][2] = 0;  M[1][3] = 0;
	M[2][0] = 0; M[2][1] = 0;  M[2][2] = 1;  M[2][3] = 0;
	M[3][0] = 0; M[3][1] = 0;  M[3][2] = 0;  M[3][3] = 1;
}

FORCEINLINE void SMatrix::operator*=(const SMatrix& Other)
{
	VectorMatrixMultiply(this, this, &Other);
}

FORCEINLINE SMatrix SMatrix::operator*(const SMatrix& Other) const
{
	SMatrix Result;
	VectorMatrixMultiply(&Result, this, &Other);
	return Result;
}

FORCEINLINE SMatrix	SMatrix::operator+(const SMatrix& Other) const
{
	SMatrix ResultMat;

	for (INT X = 0; X < 4; X++)
	{
		for (INT Y = 0; Y < 4; Y++)
		{
			ResultMat.M[X][Y] = M[X][Y] + Other.M[X][Y];
		}
	}

	return ResultMat;
}

FORCEINLINE void SMatrix::operator+=(const SMatrix& Other)
{
	*this = *this + Other;
}

FORCEINLINE SMatrix	SMatrix::operator*(float Other) const
{
	SMatrix ResultMat;

	for (INT X = 0; X < 4; X++)
	{
		for (INT Y = 0; Y < 4; Y++)
		{
			ResultMat.M[X][Y] = M[X][Y] * Other;
		}
	}

	return ResultMat;
}

FORCEINLINE void SMatrix::operator*=(float Other)
{
	*this = *this * Other;
}

// Comparison operators.
inline BOOL SMatrix::operator==(const SMatrix& Other) const
{
	for (INT X = 0; X < 4; X++)
	{
		for (INT Y = 0; Y < 4; Y++)
		{
			if (M[X][Y] != Other.M[X][Y])
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

// Error-tolerant comparison.
inline BOOL SMatrix::Equals(const SMatrix& Other, float Tolerance/*=KINDA_SMALL_NUMBER*/) const
{
	for (INT X = 0; X < 4; X++)
	{
		for (INT Y = 0; Y < 4; Y++)
		{
			if (Abs(M[X][Y] - Other.M[X][Y]) > Tolerance)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

inline BOOL SMatrix::operator!=(const SMatrix& Other) const
{
	return !(*this == Other);
}

// Homogeneous transform.
FORCEINLINE SVector4 SMatrix::TransformFVector4(const SVector4& P) const
{
	SVector4 Result;

#if ASM_X86
#ifdef _MSC_VER
	__asm
	{
		mov		eax, [P]
		mov		ecx, [this]

		movups	xmm4, [ecx]			// M[0][0]
		movups	xmm5, [ecx + 16]		// M[1][0]
		movups	xmm6, [ecx + 32]		// M[2][0]
		movups	xmm7, [ecx + 48]		// M[3][0]

		movss	xmm0, [eax]SVector.X
		shufps	xmm0, xmm0, 0
		mulps	xmm0, xmm4

		movss	xmm1, [eax]SVector.Y
		shufps	xmm1, xmm1, 0
		mulps	xmm1, xmm5

		movss	xmm2, [eax]SVector.Z
		shufps	xmm2, xmm2, 0
		mulps	xmm2, xmm6

		addps	xmm0, xmm1

		movss	xmm3, [eax]SVector4.W
		shufps	xmm3, xmm3, 0
		mulps	xmm3, xmm7

		// stall
		lea		eax, [Result]

		addps	xmm2, xmm3

		// stall

		addps	xmm0, xmm2

		movups[eax], xmm0
	}
#else
#error Please implement for your compiler.
#endif

#else
	Result.X = P.X * M[0][0] + P.Y * M[1][0] + P.Z * M[2][0] + P.W * M[3][0];
	Result.Y = P.X * M[0][1] + P.Y * M[1][1] + P.Z * M[2][1] + P.W * M[3][1];
	Result.Z = P.X * M[0][2] + P.Y * M[1][2] + P.Z * M[2][2] + P.W * M[3][2];
	Result.W = P.X * M[0][3] + P.Y * M[1][3] + P.Z * M[2][3] + P.W * M[3][3];
#endif

	return Result;
}

// Regular transform.
/** Transform a location - will take into account translation part of the SMatrix. */
FORCEINLINE SVector4 SMatrix::TransformFVector(const SVector& V) const
{
	return TransformFVector4(SVector4(V.X, V.Y, V.Z, 1.0f));
}

/** Inverts the matrix and then transforms V - correctly handles scaling in this matrix. */
FORCEINLINE SVector SMatrix::InverseTransformFVector(const SVector& V) const
{
	SMatrix InvSelf = this->Inverse();
	return InvSelf.TransformFVector(V).ToVector();
}

/** Faster version of InverseTransformFVector that assumes no scaling. WARNING: Will NOT work correctly if there is scaling in the matrix. */
FORCEINLINE SVector SMatrix::InverseTransformFVectorNoScale(const SVector& V) const
{
	SVector t, Result;

	t.X = V.X - M[3][0];
	t.Y = V.Y - M[3][1];
	t.Z = V.Z - M[3][2];

	Result.X = t.X * M[0][0] + t.Y * M[0][1] + t.Z * M[0][2];
	Result.Y = t.X * M[1][0] + t.Y * M[1][1] + t.Z * M[1][2];
	Result.Z = t.X * M[2][0] + t.Y * M[2][1] + t.Z * M[2][2];

	return Result;
}

// Normal transform.

/**
*	Transform a direction vector - will not take into account translation part of the SMatrix.
*	If you want to transform a surface normal (or plane) and correctly account for non-uniform scaling you should use TransformByUsingAdjointT.
*/
FORCEINLINE SVector4 SMatrix::TransformNormal(const SVector& V) const
{
	return TransformFVector4(SVector4(V.X, V.Y, V.Z, 0.0f));
}

/** Faster version of InverseTransformNormal that assumes no scaling. WARNING: Will NOT work correctly if there is scaling in the matrix. */
FORCEINLINE SVector SMatrix::InverseTransformNormal(const SVector& V) const
{
	SMatrix InvSelf = this->Inverse();
	return InvSelf.TransformNormal(V).ToVector();
}

/**
*	Transform a direction vector by the inverse of this matrix - will not take into account translation part.
*	If you want to transform a surface normal (or plane) and correctly account for non-uniform scaling you should use TransformByUsingAdjointT with adjoint of matrix inverse.
*/
FORCEINLINE SVector SMatrix::InverseTransformNormalNoScale(const SVector& V) const
{

	return SVector(V.X * M[0][0] + V.Y * M[0][1] + V.Z * M[0][2],
		V.X * M[1][0] + V.Y * M[1][1] + V.Z * M[1][2],
		V.X * M[2][0] + V.Y * M[2][1] + V.Z * M[2][2]);
}

// Transpose.
FORCEINLINE SMatrix SMatrix::Transpose() const
{
	SMatrix	Result;

	Result.M[0][0] = M[0][0];
	Result.M[0][1] = M[1][0];
	Result.M[0][2] = M[2][0];
	Result.M[0][3] = M[3][0];

	Result.M[1][0] = M[0][1];
	Result.M[1][1] = M[1][1];
	Result.M[1][2] = M[2][1];
	Result.M[1][3] = M[3][1];

	Result.M[2][0] = M[0][2];
	Result.M[2][1] = M[1][2];
	Result.M[2][2] = M[2][2];
	Result.M[2][3] = M[3][2];

	Result.M[3][0] = M[0][3];
	Result.M[3][1] = M[1][3];
	Result.M[3][2] = M[2][3];
	Result.M[3][3] = M[3][3];

	return Result;
}

// Determinant.
inline float SMatrix::Determinant() const
{
	return	M[0][0] * (
		M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
		M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
		M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
		) -
		M[1][0] * (
			M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
			M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
			M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
			) +
		M[2][0] * (
			M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
			M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
			M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
			) -
		M[3][0] * (
			M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
			M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
			M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
			);
}

/** Calculate determinant of rotation 3x3 matrix */
inline float SMatrix::RotDeterminant() const
{
	return
		M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
		M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
		M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1]);
}

// Inverse.
/** Fast path, doesn't check for nil matrices in final release builds */
inline SMatrix SMatrix::Inverse() const
{
	// If we're in non final release, then make sure we're not creating NaNs
#if 0 && !FINAL_RELEASE
	// Check for zero scale matrix to invert
	if (GetAxis(0).IsNearlyZero(SMALL_NUMBER) &&
		GetAxis(1).IsNearlyZero(SMALL_NUMBER) &&
		GetAxis(2).IsNearlyZero(SMALL_NUMBER))
	{
		appErrorf(TEXT("SMatrix::Inverse(), trying to invert a NIL matrix, this results in NaNs! Use InverseSafe() instead."));
	}
#endif
	SMatrix Result;
	VectorMatrixInverse(&Result, this);
	return Result;
}

// Inverse.
inline SMatrix SMatrix::InverseSafe() const
{
	SMatrix Result;

	// Check for zero scale matrix to invert
	if (GetAxis(0).IsNearlyZero(SMALL_NUMBER) &&
		GetAxis(1).IsNearlyZero(SMALL_NUMBER) &&
		GetAxis(2).IsNearlyZero(SMALL_NUMBER))
	{
		// just set to zero - avoids unsafe inverse of zero and duplicates what QNANs were resulting in before (scaling away all children)
		Result = SMatrix(SVector(0.0f), SVector(0.0f), SVector(0.0f), SVector(0.0f));
	}
	else
	{
		VectorMatrixInverse(&Result, this);
	}
	return Result;
}

inline SMatrix SMatrix::InverseSlow() const
{
	SMatrix Result;
	const float	Det = Determinant();

	if (Det == 0.0f)
		return SMatrix::Identity;

	const float	RDet = 1.0f / Det;

	Result.M[0][0] = RDet * (
		M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
		M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
		M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
		);
	Result.M[0][1] = -RDet * (
		M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
		M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
		M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
		);
	Result.M[0][2] = RDet * (
		M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
		M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
		M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
		);
	Result.M[0][3] = -RDet * (
		M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
		M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
		M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
		);

	Result.M[1][0] = -RDet * (
		M[1][0] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
		M[2][0] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
		M[3][0] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
		);
	Result.M[1][1] = RDet * (
		M[0][0] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
		M[2][0] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
		M[3][0] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
		);
	Result.M[1][2] = -RDet * (
		M[0][0] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
		M[1][0] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
		M[3][0] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
		);
	Result.M[1][3] = RDet * (
		M[0][0] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
		M[1][0] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
		M[2][0] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
		);

	Result.M[2][0] = RDet * (
		M[1][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
		M[2][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) +
		M[3][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1])
		);
	Result.M[2][1] = -RDet * (
		M[0][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
		M[2][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
		M[3][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1])
		);
	Result.M[2][2] = RDet * (
		M[0][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) -
		M[1][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
		M[3][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
		);
	Result.M[2][3] = -RDet * (
		M[0][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1]) -
		M[1][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1]) +
		M[2][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
		);

	Result.M[3][0] = -RDet * (
		M[1][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
		M[2][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) +
		M[3][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
		);
	Result.M[3][1] = RDet * (
		M[0][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
		M[2][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
		M[3][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1])
		);
	Result.M[3][2] = -RDet * (
		M[0][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) -
		M[1][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
		M[3][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
		);
	Result.M[3][3] = RDet * (
		M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
		M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
		M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
		);

	return Result;
}

inline SMatrix SMatrix::TransposeAdjoint() const
{
	SMatrix TA;

	TA.M[0][0] = this->M[1][1] * this->M[2][2] - this->M[1][2] * this->M[2][1];
	TA.M[0][1] = this->M[1][2] * this->M[2][0] - this->M[1][0] * this->M[2][2];
	TA.M[0][2] = this->M[1][0] * this->M[2][1] - this->M[1][1] * this->M[2][0];
	TA.M[0][3] = 0.f;

	TA.M[1][0] = this->M[2][1] * this->M[0][2] - this->M[2][2] * this->M[0][1];
	TA.M[1][1] = this->M[2][2] * this->M[0][0] - this->M[2][0] * this->M[0][2];
	TA.M[1][2] = this->M[2][0] * this->M[0][1] - this->M[2][1] * this->M[0][0];
	TA.M[1][3] = 0.f;

	TA.M[2][0] = this->M[0][1] * this->M[1][2] - this->M[0][2] * this->M[1][1];
	TA.M[2][1] = this->M[0][2] * this->M[1][0] - this->M[0][0] * this->M[1][2];
	TA.M[2][2] = this->M[0][0] * this->M[1][1] - this->M[0][1] * this->M[1][0];
	TA.M[2][3] = 0.f;

	TA.M[3][0] = 0.f;
	TA.M[3][1] = 0.f;
	TA.M[3][2] = 0.f;
	TA.M[3][3] = 1.f;

	return TA;
}

// Remove any scaling from this matrix (ie magnitude of each row is 1)
inline void SMatrix::RemoveScaling(float Tolerance/*=SMALL_NUMBER*/)
{
	// For each row, find magnitude, and if its non-zero re-scale so its unit length.
	for (INT i = 0; i < 3; i++)
	{
		const float SquareSum = (M[i][0] * M[i][0]) + (M[i][1] * M[i][1]) + (M[i][2] * M[i][2]);
		if (SquareSum > Tolerance)
		{
			const float Scale = appInvSqrt(SquareSum);
			M[i][0] *= Scale;
			M[i][1] *= Scale;
			M[i][2] *= Scale;
		}
	}
}

// Returns matrix without scale information
inline SMatrix SMatrix::GetMatrixWithoutScale(float Tolerance/*=SMALL_NUMBER*/) const
{
	SMatrix Result = *this;
	Result.RemoveScaling(Tolerance);
	return Result;
}

/** Remove any scaling from this matrix (ie magnitude of each row is 1) and return the 3D scale vector that was initially present. */
inline SVector SMatrix::ExtractScaling(float Tolerance/*=SMALL_NUMBER*/)
{
	SVector Scale3D(0, 0, 0);

	// For each row, find magnitude, and if its non-zero re-scale so its unit length.
	for (INT i = 0; i < 3; i++)
	{
		const float SquareSum = (M[i][0] * M[i][0]) + (M[i][1] * M[i][1]) + (M[i][2] * M[i][2]);
		if (SquareSum > Tolerance)
		{
			const float Scale = appSqrt(SquareSum);
			const float InvScale = 1.f / Scale;
			M[i][0] *= InvScale;
			M[i][1] *= InvScale;
			M[i][2] *= InvScale;

			Scale3D[i] = Scale;
		}
	}

	return Scale3D;
}

/** return a 3D scale vector calculated from this matrix (where each component is the magnitude of a row vector). */
inline SVector SMatrix::GetScaleVector(float Tolerance/*=SMALL_NUMBER*/) const
{
	SVector Scale3D(1, 1, 1);

	// For each row, find magnitude, and if its non-zero re-scale so its unit length.
	for (INT i = 0; i < 3; i++)
	{
		const float SquareSum = (M[i][0] * M[i][0]) + (M[i][1] * M[i][1]) + (M[i][2] * M[i][2]);
		if (SquareSum > Tolerance)
		{
			Scale3D[i] = appSqrt(SquareSum);
		}
	}

	return Scale3D;
}
// Remove any translation from this matrix
inline SMatrix SMatrix::RemoveTranslation() const
{
	SMatrix Result = *this;
	Result.M[3][0] = 0.0f;
	Result.M[3][1] = 0.0f;
	Result.M[3][2] = 0.0f;
	return Result;
}

inline SMatrix SMatrix::ConcatTranslation(const SVector& Translation) const
{
	SMatrix Result = *this;
	Result.M[3][0] += Translation.X;
	Result.M[3][1] += Translation.Y;
	Result.M[3][2] += Translation.Z;
	return Result;
}

/** Returns TRUE if any element of this matrix is NaN */
inline BOOL SMatrix::ContainsNaN() const
{
	for (INT i = 0; i < 4; i++)
	{
		for (INT j = 0; j < 4; j++)
		{
			if (appIsNaN(M[i][j]) || !appIsFinite(M[i][j]))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

/** @return the maximum magnitude of any row of the matrix. */
inline float SMatrix::GetMaximumAxisScale() const
{
	const float MaxRowScaleSquared = Max(
		GetAxis(0).SizeSquared(),
		Max(
			GetAxis(1).SizeSquared(),
			GetAxis(2).SizeSquared()
			)
		);
	return appSqrt(MaxRowScaleSquared);
}

inline SMatrix SMatrix::ApplyScale(float Scale)
{
	return SScaleMatrix(Scale) * (*this);
}

inline void SMatrix::ScaleTranslation(const SVector& Scale3D)
{
	M[3][0] *= Scale3D.X;
	M[3][1] *= Scale3D.Y;
	M[3][2] *= Scale3D.Z;
}

// GetOrigin

inline SVector SMatrix::GetOrigin() const
{
	return SVector(M[3][0], M[3][1], M[3][2]);
}

inline SVector SMatrix::GetAxis(INT i) const
{
	return SVector(M[i][0], M[i][1], M[i][2]);
}

inline void SMatrix::GetAxes(SVector& X, SVector& Y, SVector& Z) const
{
	X.X = M[0][0]; X.Y = M[0][1]; X.Z = M[0][2];
	Y.X = M[1][0]; Y.Y = M[1][1]; Y.Z = M[1][2];
	Z.X = M[2][0]; Z.Y = M[2][1]; Z.Z = M[2][2];
}

inline void SMatrix::SetAxis(INT i, const SVector& Axis)
{
	M[i][0] = Axis.X;
	M[i][1] = Axis.Y;
	M[i][2] = Axis.Z;
}

inline void SMatrix::SetOrigin(const SVector& NewOrigin)
{
	M[3][0] = NewOrigin.X;
	M[3][1] = NewOrigin.Y;
	M[3][2] = NewOrigin.Z;
}

inline void SMatrix::SetAxes(SVector* Axis0 /*= NULL*/, SVector* Axis1 /*= NULL*/, SVector* Axis2 /*= NULL*/, SVector* Origin /*= NULL*/)
{
	if (Axis0 != NULL)
	{
		M[0][0] = Axis0->X;
		M[0][1] = Axis0->Y;
		M[0][2] = Axis0->Z;
	}
	if (Axis1 != NULL)
	{
		M[1][0] = Axis1->X;
		M[1][1] = Axis1->Y;
		M[1][2] = Axis1->Z;
	}
	if (Axis2 != NULL)
	{
		M[2][0] = Axis2->X;
		M[2][1] = Axis2->Y;
		M[2][2] = Axis2->Z;
	}
	if (Origin != NULL)
	{
		M[3][0] = Origin->X;
		M[3][1] = Origin->Y;
		M[3][2] = Origin->Z;
	}
}

inline SVector SMatrix::GetColumn(INT i) const
{
	return SVector(M[0][i], M[1][i], M[2][i]);
}


// Frustum plane extraction.
FORCEINLINE BOOL SMatrix::GetFrustumNearPlane(SPlane& OutPlane) const
{
	return MakeFrustumPlane(
		M[0][2],
		M[1][2],
		M[2][2],
		M[3][2],
		OutPlane
		);
}

FORCEINLINE BOOL SMatrix::GetFrustumFarPlane(SPlane& OutPlane) const
{
	return MakeFrustumPlane(
		M[0][3] - M[0][2],
		M[1][3] - M[1][2],
		M[2][3] - M[2][2],
		M[3][3] - M[3][2],
		OutPlane
		);
}

FORCEINLINE BOOL SMatrix::GetFrustumLeftPlane(SPlane& OutPlane) const
{
	return MakeFrustumPlane(
		M[0][3] + M[0][0],
		M[1][3] + M[1][0],
		M[2][3] + M[2][0],
		M[3][3] + M[3][0],
		OutPlane
		);
}

FORCEINLINE BOOL SMatrix::GetFrustumRightPlane(SPlane& OutPlane) const
{
	return MakeFrustumPlane(
		M[0][3] - M[0][0],
		M[1][3] - M[1][0],
		M[2][3] - M[2][0],
		M[3][3] - M[3][0],
		OutPlane
		);
}

FORCEINLINE BOOL SMatrix::GetFrustumTopPlane(SPlane& OutPlane) const
{
	return MakeFrustumPlane(
		M[0][3] - M[0][1],
		M[1][3] - M[1][1],
		M[2][3] - M[2][1],
		M[3][3] - M[3][1],
		OutPlane
		);
}

FORCEINLINE BOOL SMatrix::GetFrustumBottomPlane(SPlane& OutPlane) const
{
	return MakeFrustumPlane(
		M[0][3] + M[0][1],
		M[1][3] + M[1][1],
		M[2][3] + M[2][1],
		M[3][3] + M[3][1],
		OutPlane
		);
}

/**
* Utility for mirroring this transform across a certain plane,
* and flipping one of the axis as well.
*/
inline void SMatrix::Mirror(UINT8 MirrorAxis, UINT8 FlipAxis)
{
	if (MirrorAxis == AXIS_X)
	{
		M[0][0] *= -1.f;
		M[1][0] *= -1.f;
		M[2][0] *= -1.f;

		M[3][0] *= -1.f;
	}
	else if (MirrorAxis == AXIS_Y)
	{
		M[0][1] *= -1.f;
		M[1][1] *= -1.f;
		M[2][1] *= -1.f;

		M[3][1] *= -1.f;
	}
	else if (MirrorAxis == AXIS_Z)
	{
		M[0][2] *= -1.f;
		M[1][2] *= -1.f;
		M[2][2] *= -1.f;

		M[3][2] *= -1.f;
	}

	if (FlipAxis == AXIS_X)
	{
		M[0][0] *= -1.f;
		M[0][1] *= -1.f;
		M[0][2] *= -1.f;
	}
	else if (FlipAxis == AXIS_Y)
	{
		M[1][0] *= -1.f;
		M[1][1] *= -1.f;
		M[1][2] *= -1.f;
	}
	else if (FlipAxis == AXIS_Z)
	{
		M[2][0] *= -1.f;
		M[2][1] *= -1.f;
		M[2][2] *= -1.f;
	}
}

SRotator SVectorToRotation(const SVector& vec);

SQuat SRotatorToSQuat(const SRotator& rot);

///**
//* Mirrors a vector about a plane.
//*/
//inline SVector SVector::MirrorByPlane(const SPlane& Plane) const
//{
//	return *this - Plane * (2.f * Plane.PlaneDot(*this));
//}

//SRotator SVector::Rotation() const noexcept {
//	SRotator R;
//
//	// Find yaw.
//	R.Yaw = appRound(appAtan2(Y, X) * (float)INT16_MAX / (2.f*PI));
//
//	// Find pitch.
//	R.Pitch = appRound(appAtan2(Z, appSqrt(X*X + Y*Y)) * (float)INT16_MAX / (2.f*PI));
//
//	// Find roll.
//	R.Roll = 0;
//
//	return R;
//}

//SSphere SSphere::TransformBy(const SMatrix & M) const
//{
//	SSphere	Result;
//
//	Result.Center = M.TransformFVector(this->Center).ToVector();
//
//	const SVector XAxis(M.M[0][0], M.M[0][1], M.M[0][2]);
//	const SVector YAxis(M.M[1][0], M.M[1][1], M.M[1][2]);
//	const SVector ZAxis(M.M[2][0], M.M[2][1], M.M[2][2]);
//
//	Result.W = appSqrt(Max(XAxis | XAxis, Max(YAxis | YAxis, ZAxis | ZAxis))) * W;
//
//	return Result;
//}
