#include "FColor.h"
#include "FIStream.h"

FColor::FColor(FIStream * stream)
	:FReadable(stream)
{
	stream->Read(color, 4);
}

FColor::FColor()
{
}

FLinerColor::FLinerColor(FIStream * stream)
	: FReadable(stream)
{
	stream->Read((uint8*)color, 4 * sizeof(float));
}

FLinerColor::FLinerColor()
{
}
