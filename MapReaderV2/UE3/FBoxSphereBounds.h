#ifndef FBOXSPHEREBOUNDS_H
#define FBOXSPHEREBOUNDS_H

#include <memory>

#include "FReadable.h"
#include "FVector.h"
#include "FArray.h"

struct FBox : FReadable {
	FVector3 min, max;

	FBox();
	FBox(FIStream*);
};

struct FSphereBounds : FReadable {
	FVector3 min;
	float w;

	FSphereBounds();
	FSphereBounds(FIStream*);
};

struct FBoxSphereBounds : FReadable {
	FVector3 origin, extent;
	float radius;

	FBoxSphereBounds();
	FBoxSphereBounds(FIStream*);
};

struct FTerrainBV : FReadable {
	FBox bounds;
	
	FTerrainBV();
	FTerrainBV(FIStream*);
};

struct FConvexVolume : FReadable {

	FArray<FPlane> 
		planes,
		permutedPlanes;

	FConvexVolume();
	FConvexVolume(FIStream*);
};

#endif
