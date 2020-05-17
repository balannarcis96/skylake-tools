#include "CustomFArrays.h"
#include "FIStream.h"


FIntArray::FIntArray()
{
}

FIntArray::FIntArray(FIStream * stream)
	:FReadable(stream)
{
	count = stream->ReadInt32();
	data = new int32[count];
	stream->Read((uint8*)data, count * sizeof(int32));
}

int32 FIntArray::operator[](int32 index)
{
	if (index < 0 || index >= count) {
		return 0xFFFFFFFF;
	}

	return data[index];
}

FIntArray::~FIntArray()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}
}
