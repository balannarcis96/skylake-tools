#include "FTerrain.h"
#include "FIStream.h"
#include "UObject.h"

FTerrainHeight::FTerrainHeight(FIStream * stream)
	:FReadable(stream)
{
	value = stream->ReadInt16();
}

FTerrainHeight::FTerrainHeight()
{
}


FTerrainInfoData::FTerrainInfoData(FIStream * stream)
	: FReadable(stream)
{
	Data = stream->ReadUInt8();
}

FAlphaMap::FAlphaMap()
{
}

FAlphaMap::FAlphaMap(FIStream * stream)
	: FReadable(stream)
{
	data = FArray<uint8>(stream);
}

FTerrainMaterialMask::FTerrainMaterialMask()
{
}

FTerrainMaterialMask::FTerrainMaterialMask(FIStream * stream)
	: FReadable(stream)
{
	numBits = stream->ReadInt32();
	bitMask= stream->ReadUInt64();
}

FTerrainMaterialResource::FTerrainMaterialResource()
{
}

FTerrainMaterialResource::FTerrainMaterialResource(FIStream * stream)
	: FReadable(stream)
{
	terrain = UObject::ReadFromStream(stream);
	materialMask = FTerrainMaterialMask(stream);
	materialIds = FArray<FGUID>(stream);
	bEnableSpecular = stream->ReadInt32();
}

FCachedTerrainMaterialArray::FCachedTerrainMaterialArray()
{
}

FCachedTerrainMaterialArray::FCachedTerrainMaterialArray(FIStream * stream)
	: FReadable(stream)
{
	cachedMaterials = FArray<FTerrainMaterialResource>(stream);
}

FTerrainLayer::FTerrainLayer(FIStream * stream)
	: FReadable(stream)
{
	//??
}

FTerrainBVNode::FTerrainBVNode()
{
}

FTerrainBVNode::FTerrainBVNode(FIStream * stream)
	: FReadable(stream)
{
	boundingVolume = FTerrainBV(stream);
	bIsLeaf = stream->ReadInt32();
	stream->Read((uint8*)nodeIndexes, 4 * sizeof(int16));
}

FTerrainBVTree::FTerrainBVTree(FIStream * stream)
	: FReadable(stream)
{
	nodesCount = stream->ReadInt32();
	if (nodesCount) {
		nodes = new FTerrainBVNode[nodesCount];

		for (size_t i = 0; i < nodesCount; i++)
		{
			nodes[i] = FTerrainBVNode(stream);
		}
	}
	else {
		nodes = nullptr;
	}
}
