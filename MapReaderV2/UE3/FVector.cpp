#include "FVector.h"
#include "FIStream.h"

FVector2::FVector2()
{
}

FVector2::FVector2(FIStream * stream)
	:FReadable(stream)
{
	x = stream->ReadFloat();
	y = stream->ReadFloat();
}

FVector3::FVector3()
{
}

FVector3::FVector3(FIStream * stream)
	:FVector2(stream)
{
	z = stream->ReadFloat();
}

FVector4::FVector4()
{
}

FVector4::FVector4(FIStream * stream)
	:FVector3(stream)
{
	w = stream->ReadFloat();
}

FPlane::FPlane()
{
}

FPlane::FPlane(FIStream * stream)
	: FVector4(stream)
{
}
