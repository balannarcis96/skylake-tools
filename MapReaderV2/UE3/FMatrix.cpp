#include "FMatrix.h"

#include "FIStream.h"

FMatrix::FMatrix(FIStream * stream)
	:FReadable(stream)
{
	stream->Read((uint8*)m, 16 * sizeof(float));
}

float FMatrix::operator[](int index)
{
	if (index < 0 || index >= 16) {
		return FLT_MAX;
	}

	return m[index];
}
