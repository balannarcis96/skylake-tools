#ifndef MIP_MAP_H
#define MIP_MAP_H

#include "FReadable.h"
#include "FBulkData.h"

struct FMipMap : FReadable {
	FBulkData data;
	int32 
		width,
		height;

	FMipMap();
	FMipMap(FIStream *stream);
};

#endif
