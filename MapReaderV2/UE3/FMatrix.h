#ifndef FMATRIX_H
#define FMATRIX_H

#include "FReadable.h"

struct FIStream;

struct FMatrix : FReadable {
	float m[16];

	FMatrix(FIStream* stream);

	float operator[] (int index);
};

#endif
