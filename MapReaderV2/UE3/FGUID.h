#ifndef FGUID_H
#define FGUID_H

#include "FReadable.h"
#include "typeDefs.h"

struct FIStream;

struct FGUID :FReadable {
	uint8 data[16];

	FGUID();
	FGUID(FIStream * stream);
};


#endif
