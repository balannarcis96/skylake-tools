#ifndef FGLOBALMATH_H
#define FGLOBALMATH_H

#include "typeDefs.h"
#include "UMath.h"


#undef  PI
#define PI 					(3.1415926535897932)

class FGlobalMath{
public:
	// Constants.
	enum{ ANGLE_SHIFT = 2 };		// Bits to right-shift to get lookup value.
	enum{ ANGLE_BITS = 14 };		// Number of valid bits in angles.
	enum{ NUM_ANGLES = 16384 }; 	// Number of angles that are in lookup table.
	enum{ ANGLE_MASK = (((1 << ANGLE_BITS) - 1) << (16 - ANGLE_BITS)) };

	// Basic math functions.
	FORCEINLINE FLOAT SinTab(int i) const{
		return TrigFLOAT[((i >> ANGLE_SHIFT) & (NUM_ANGLES - 1))];
	}
	FORCEINLINE FLOAT CosTab(int i) const{
		return TrigFLOAT[(((i + 16384) >> ANGLE_SHIFT)& (NUM_ANGLES - 1))];
	}
	FLOAT SinFloat(FLOAT F) const{
		return SinTab(appTruncL((F * 65536.f) / (2.f * PI)));
	}
	FLOAT CosFloat(FLOAT F) const{
		return CosTab(appTruncL((F * 65536.f) / (2.f * PI)));
	}

	// Constructor.
	FGlobalMath(){
		// Init base angle table.
		{
			for (INT i = 0; i < NUM_ANGLES; i++)
				TrigFLOAT[i] = appSin((FLOAT)i * 2.f * PI / (FLOAT)NUM_ANGLES);
		}
	}

private:
	// Tables.
	FLOAT  TrigFLOAT[NUM_ANGLES];
};
extern FGlobalMath GMath;

extern INT ReduceAngle(INT Angle);


#endif