#include "FGUID.h"
#include "FIStream.h"

FGUID::FGUID()
{
}

FGUID::FGUID(FIStream * stream)
{
	stream->Read(data, 16);
}
