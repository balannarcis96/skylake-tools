#ifndef FBULKDATA_H
#define FBULKDATA_H

#include "FReadable.h"

struct FIStream;

struct FBulkData : FReadable
{
	int32 bulkFlags;
	uint compressedSize;
	uint decompressedSize;
	uint compressedOffset;
	uint8*  data{ nullptr };
	bool bIsUsed;

	FBulkData();
	FBulkData(FIStream* stream);
	~FBulkData();
};


struct FByteBulkData : FReadable
{
	int32 bulkFlags;
	uint compressedSize;
	uint decompressedSize;
	uint compressedOffset;
	uint8*  data{ nullptr };
	uint elementCount;

	FByteBulkData();
	FByteBulkData(FIStream* stream);
	~FByteBulkData();
};


#endif
