#include "FMipMap.h"

#include "FIStream.h"

FMipMap::FMipMap()
{
}

FMipMap::FMipMap(FIStream * stream)
	:FReadable(stream)
{
	data = FBulkData(stream);
	width = stream->ReadInt32();
	height= stream->ReadInt32();
}
