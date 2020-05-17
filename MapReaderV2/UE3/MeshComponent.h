#ifndef MESHCOMPONENT_H
#define MESHCOMPONENT_H

#include "UComponent.h"
#include "FStaticMesh.h"

struct MeshComponent : PrimitiveComponent {
	UObject* mesh;

	void ReadData(FIStream& data);
};

struct StaticMeshComponent : MeshComponent {
	FPArray<FStaticMeshComponentLODInfo>* lodInfo;
	void ReadData(FIStream& data);

	~StaticMeshComponent();
};

struct SkeletalMeshComponent : MeshComponent {
	void ReadData(FIStream& data);
};

#endif
