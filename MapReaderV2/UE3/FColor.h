#ifndef FCOLOR_H
#define FCOLOR_H

#include "FReadable.h"

struct FIStream;

struct FLinerColor :FReadable {
	float color[4];

	FLinerColor(FIStream *  stream);

	FLinerColor();

	//todo ToColor()
};

struct FColor :FReadable {
	uint8 color[4];

	FColor(FIStream *  stream);

	FColor();

	//todo ToColor()
};

#endif