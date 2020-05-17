#ifndef FLIGHTMAP_H
#define FLIGHTMAP_H

#include "FReadable.h"
#include "FArray.h"
#include "FGUID.h"
#include "FVector.h"
#include "FBulkData.h"

#include <vector>

struct UObject;

struct FLightMap :FReadable {
	LightMapType type;
	FArray<FGUID> lightGuids;
	std::vector<FVector3> scaleVectors;

	FLightMap();
	FLightMap(FIStream * stream);


	static FLightMap *  FromStream(FIStream * stream);

protected:
	void serialize(FIStream* stream);
};

struct FLightMap1D :FLightMap {
	UObject * owner;
	FBulkData directionalSamples;
	FBulkData simpleSamples;

	FLightMap1D();
	FLightMap1D(FIStream * stream);
};

struct FLightMap2D :FLightMap {
	std::vector<UObject *> textures;
	FVector2 coordinateScale;
	FVector2 coordinateBias;

	FLightMap2D();
	FLightMap2D(FIStream * stream);
};

#endif
