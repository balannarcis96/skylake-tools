#include "FLightMap.h"
#include "UObject.h"

FLightMap::FLightMap()
{
}

FLightMap::FLightMap(FIStream * stream)
	:FReadable(stream)
{
}

FLightMap * FLightMap::FromStream(FIStream * stream)
{
	LightMapType type = (LightMapType)stream->ReadInt32();

	FLightMap * m = nullptr;
	switch (type)
	{
	case LMT_None:
		break;
	case LMT_1D:
		m = new FLightMap1D(stream);
		break;
	case LMT_2D:
		m = new FLightMap2D(stream);
		break;
	default:
		m = new FLightMap(stream);
		break;
	}

	m->type = type;
	return m;
}

void FLightMap::serialize(FIStream * stream)
{
	lightGuids = FArray<FGUID>(stream);
}

FLightMap1D::FLightMap1D()
{ }

FLightMap1D::FLightMap1D(FIStream * stream)
	:FLightMap(stream)
{
	serialize(stream);
	owner = UObject::ReadFromStream(stream);
	directionalSamples = FBulkData(stream);

	for (size_t i = 0; i < 4; i++)
	{
		scaleVectors.push_back(FVector3(stream));
	}

	simpleSamples = FBulkData(stream);
}

FLightMap2D::FLightMap2D()
{
}

FLightMap2D::FLightMap2D(FIStream * stream)
	:FLightMap(stream)
{
	serialize(stream);

	for (size_t i = 0; i < 4; i++)
	{
		UObject * o = UObject::ReadFromStream(stream);
		if (!o) {
			throw "Failed to read lightmap";
		}

		textures.push_back(o);
		scaleVectors.push_back(FVector3(stream));
	}

	coordinateScale = FVector2(stream);
	coordinateBias = FVector2(stream);
}
