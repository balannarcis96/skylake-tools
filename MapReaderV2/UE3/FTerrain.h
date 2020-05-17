#ifndef FTERRAIN_H
#define FTERRAIN_H

#include "FReadable.h"
#include "FArray.h"
#include "FGUID.h"
#include "FString.h"
#include "FColor.h"
#include "FBoxSphereBounds.h"

enum ETerrainLayerType
{
	TLT_Empty = -1,
	TLT_HeightMap = 0,
	TLT_Layer,
	TLT_ProceduralLayer,
	TLT_DecoLayer,
	TLT_TerrainMaterial,
	TLT_FilteredMaterial,
	TLT_Decoration,
	TLT_DisplacementMap,
	TLT_MAX_COUNT
};

inline const char* GetTerrainLayerTypeString(ETerrainLayerType Type)
{
	switch (Type)
	{
	case TLT_Empty:             return "EMPTY";
	case TLT_HeightMap:         return "HeightMap";
	case TLT_Layer:             return "Layer";
	case TLT_DecoLayer:         return "DecoLayer";
	case TLT_TerrainMaterial:   return "TerrainMaterial";
	case TLT_FilteredMaterial:  return "FilteredMaterial";
	case TLT_Decoration:        return "Decoration";
	}

	return"INVALID";
}

struct UObject;

struct FTerrainHeight : FReadable {
	uint16 value;

	FTerrainHeight(FIStream * stream);
	FTerrainHeight();
};


struct FTerrainInfoData : FReadable
{
	BYTE    Data;

	enum InfoFlags
	{
		TID_Visibility_Off = 0x0001,
		TID_OrientationFlip = 0x0002,
		TID_Unreachable = 0x0004,
		TID_Locked = 0x0008,
		//      TID_    = 0x0010,
		//      TID_    = 0x0020,
		//      TID_    = 0x0040,
		//      TID_    = 0x0080,
		//      TID_    = 0x0100,
		//      TID_    = 0x0200,
		//      TID_    = 0x0400,
		//      TID_    = 0x0800,
		//      TID_    = 0x1000,
		//      TID_    = 0x2000,
		//      TID_    = 0x4000,
		//      TID_    = 0x8000,
	};


	FTerrainInfoData()
		:FReadable()
	{ }

	FTerrainInfoData(FIStream* stream);

	// Getters
	inline uint32 IsVisible() const
	{
		return ((Data & TID_Visibility_Off) == 0);
	}

	inline uint32 IsOrientationFlipped() const
	{
		return ((Data & TID_OrientationFlip) != 0);
	}

	inline uint32 IsUnreachable() const
	{
		return ((Data & TID_Unreachable) != 0);
	}

	inline int32 IsLocked() const
	{
		return ((Data & TID_Locked) != 0);
	}

	// Setters
	inline void SetIsVisible(uint32 bVisibile)
	{
		if (bVisibile)
		{
			Data &= ~TID_Visibility_Off;
		}
		else
		{
			Data |= TID_Visibility_Off;
		}
	}

	inline void SetIsOrientationFlipped(uint32 bOrientationFlipped)
	{
		if (bOrientationFlipped)
		{
			Data |= TID_OrientationFlip;
		}
		else
		{
			Data &= ~TID_OrientationFlip;
		}
	}

	inline void SetIsUnreachable(int32 bUnreachable)
	{
		if (bUnreachable)
		{
			Data |= TID_Unreachable;
		}
		else
		{
			Data &= ~TID_Unreachable;
		}
	}

	inline void SetIsLocked(int32 bLocked)
	{
		if (bLocked)
		{
			Data |= TID_Locked;
		}
		else
		{
			Data &= ~TID_Locked;
		}
	}
};

struct FAlphaMap : FReadable {
	FArray<uint8> data;

	FAlphaMap();
	FAlphaMap(FIStream * stream);
};

struct FTerrainMaterialMask : FReadable {
	u_int64 bitMask;
	int32 numBits;

	FTerrainMaterialMask();
	FTerrainMaterialMask(FIStream * stream);
};

struct FTerrainMaterialResource : FReadable
{
	UObject* terrain;
	FTerrainMaterialMask materialMask;
	FArray<FGUID> materialIds;
	int32 bEnableSpecular;

	FTerrainMaterialResource();
	FTerrainMaterialResource(FIStream* stream);
};

struct FCachedTerrainMaterialArray : FReadable {
	FArray<FTerrainMaterialResource> cachedMaterials;

	FCachedTerrainMaterialArray();
	FCachedTerrainMaterialArray(FIStream * stream);
};

struct FTerrainLayer : FReadable {
	FString name;
	FColor highlightedColor;
	int32
		minVertex,
		minY,
		maxX,
		maxY;

	FTerrainLayer(FIStream * stream);
};

struct FTerrainBVNode : FReadable {
	FTerrainBV boundingVolume;
	int32 bIsLeaf;

	int16 nodeIndexes[4];

	FTerrainBVNode();
	FTerrainBVNode(FIStream * stream);
};

struct FTerrainBVTree : FReadable {
	uint32 nodesCount;
	FTerrainBVNode* nodes;

	FTerrainBVTree()
		:nodes(nullptr), nodesCount(0) {}
	FTerrainBVTree(FIStream * stream);
};
#endif
