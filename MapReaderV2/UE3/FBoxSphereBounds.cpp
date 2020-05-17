#include "FBoxSphereBounds.h"

#include "FIStream.h"

FBox::FBox()
{
}

FBox::FBox(FIStream * stream)
	:FReadable(stream)
{
	min = FVector3(stream);
	max = FVector3(stream);
}

FSphereBounds::FSphereBounds()
{
}

FSphereBounds::FSphereBounds(FIStream * stream)
	:FReadable(stream)
{
	w = stream->ReadFloat();
	min = FVector3(stream);
}

FBoxSphereBounds::FBoxSphereBounds()
{
}

FBoxSphereBounds::FBoxSphereBounds(FIStream *stream)
	:FReadable(stream)
{
	origin = FVector3(stream);
	extent = FVector3(stream);
	radius = stream->ReadFloat();
}

FTerrainBV::FTerrainBV()
{
}

FTerrainBV::FTerrainBV(FIStream * stream)
	:FReadable(stream)
{
	bounds = FBox(stream);
}

FConvexVolume::FConvexVolume()
{
}

FConvexVolume::FConvexVolume(FIStream * stream)
	:FReadable(stream)
{
	planes = FArray<FPlane>(stream);
	permutedPlanes = FArray<FPlane>(stream);
}
