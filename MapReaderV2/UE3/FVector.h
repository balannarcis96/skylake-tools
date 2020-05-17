#ifndef FVECTOR_H
#define FVECTOR_H

#include "FReadable.h"

struct FIStream;

struct FVector2 : FReadable {
	float x, y;

	FVector2();
	FVector2(FIStream * stream);
};

struct FVector3 : FVector2 {
	float z;

	FVector3();
	FVector3(FIStream * stream);
};

struct FVector4 : FVector3 {
	float w;
	FVector4();
	FVector4(FIStream * stream);
};

struct FPlane : FVector4 {
	FPlane();
	FPlane(FIStream * stream);
};

#endif
