#ifndef FSTREAMABLETEXTUREINSTANCE_H
#define FSTREAMABLETEXTUREINSTANCE_H

#include "FReadable.h"
#include "FBoxSphereBounds.h"

struct FStreamableTextureInstance : FReadable {
	FSphereBounds boundingSphere;
	float texelFactor;

	FStreamableTextureInstance();
	FStreamableTextureInstance(FIStream * stream);
};

#endif