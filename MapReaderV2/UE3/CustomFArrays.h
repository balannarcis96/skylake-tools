#ifndef FCUSTOMARRAY_H
#define FCUSTOMARRAY_H

#include "FReadable.h"

struct FStream;

struct FIntArray : FReadable {
	int32 count;
	int32 *data{ nullptr };

	FIntArray();
	FIntArray(FIStream*);

	int32 operator[](int32 index);

	~FIntArray();
};


#endif
