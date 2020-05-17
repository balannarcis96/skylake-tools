#include "FBulkData.h"
#include "FIStream.h"
#include "Enums.h"

FBulkData::FBulkData()
{
}

FBulkData::FBulkData(FIStream * stream)
	:FReadable(stream)
{
	bulkFlags = stream->ReadInt32();
	decompressedSize = stream->ReadUInt32();
	compressedSize = stream->ReadUInt32();
	compressedOffset = stream->ReadUInt32();

	if ((bulkFlags & BULKDATA_Unused) && !(bulkFlags & BULKDATA_StoreInSeparateFile) && compressedSize) {
		u_int64 pos = 0;

		if (pos != compressedOffset && compressedOffset) {
			pos = stream->_pos;
			stream->_pos = compressedOffset;
		}

		data = new uint8[compressedSize];
		stream->Read(data, compressedSize);

		if (pos){
			stream->_pos = pos;
		}
	}
}

FBulkData::~FBulkData()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}
}

FByteBulkData::FByteBulkData()
{
}

FByteBulkData::FByteBulkData(FIStream * stream)
{

}

FByteBulkData::~FByteBulkData()
{
}
