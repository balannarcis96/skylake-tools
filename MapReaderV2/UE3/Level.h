#ifndef LEVEL_H
#define LEVEL_H

#include "UObject.h"
#include "FArray.h"
#include "UPackage.h"
#include "FString.h"
#include "FStreamableTextureInstance.h"
#include "FMap.h"
#include "Texture2D.h"
#include "FVector.h"

struct FKCachedConvexDataElement : FReadable {
	//Cooked physics information for a single convex mesh.


	//Cooked data stream for physics engine for one convex hull.
	FByteArray* convexElementData;

	FKCachedConvexDataElement();
	FKCachedConvexDataElement(FIStream*stream);
	~FKCachedConvexDataElement();
};

struct FKCachedPerTriData : FReadable {
	FByteArray cachedPerTriData;

	FKCachedPerTriData();
	FKCachedPerTriData(FIStream*stream);
};

struct FKCachedConvexData : FReadable {
	FPArray<FKCachedConvexDataElement>* cachedConvexElements;

	FKCachedConvexData();
	FKCachedConvexData(FIStream*stream);
	~FKCachedConvexData();
};

struct FCachedPhysSMData : FReadable {
	//Used for storing cached data for simplified static mesh collision at a particular scaling. One entry, relating to a particular static mesh cached at a particular scaling, giving an index into the CachedPhysSMDataStore.
	FVector3 scale3D;
	int32 cachedDataIndex;

	FCachedPhysSMData();
	FCachedPhysSMData(FIStream*stream);
};

struct FCachedPerTriPhysSMData : FReadable {
	FCachedPerTriPhysSMData();
	FCachedPerTriPhysSMData(FIStream*stream);
};

struct Level :UObject {
	TransUObjectArray* actors;
	FUrl* url;

	UObject* model;
	UObjectArray modelComponents;
	UObjectArray gameSequences;
	PMapUObjectKeyArray<FStreamableTextureInstance> textureToInstancesMap;

	FByteArray cachedPhysBSPData;

	FMultiMap<FCachedPhysSMData> cachedPhysSMDataMap;
	FPArray<FKCachedConvexData> *cachedPhysSMDataStore;

	FMultiMap<FCachedPerTriPhysSMData> cachedPhysPerTriSMDataMap;
	FPArray<FKCachedPerTriData> cachedPhysPerTriSMDataStore;

	int32 cachedPhysBSPDataVersion;
	int32 cachedPhysSMDataVersion;

	FMap<Texture2D*, int32> forceStreamTextures;

	UObject* navListStart;
	UObject* navListEnd;
	UObject* coverListStart;
	UObject* coverListEnd;
	UObject* pylonListStart;
	UObject* pylonListEnd;

	FArray<UObject* > crossLevelActors;
	int32 unk;

	std::vector<Terrain*> terrains;

	//gui data
	bool _drawing = false;
	//---------

	void ReadData(FIStream & data);

	void Init();
	void Render();

	void Export(FIStream &);

	Level();
	~Level();
};

#endif
