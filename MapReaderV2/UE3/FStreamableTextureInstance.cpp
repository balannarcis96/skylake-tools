#include "FStreamableTextureInstance.h"
#include "FIStream.h"

FStreamableTextureInstance::FStreamableTextureInstance()
{
}

FStreamableTextureInstance::FStreamableTextureInstance(FIStream * stream)
	:FReadable(stream)
{
	boundingSphere = FSphereBounds(stream);
	texelFactor = stream->ReadFloat();
}
