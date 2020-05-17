#include "MeshComponent.h"
#include "FPropertyTag.h"
#include "StaticMesh.h"

void MeshComponent::ReadData(FIStream & data)
{
	PrimitiveComponent::ReadData(data);
}

void StaticMeshComponent::ReadData(FIStream & data)
{
	MeshComponent::ReadData(data);

	FPropertyTag* prop = GetPropByName("StaticMesh");
	if (prop) {
		mesh = (StaticMesh*)prop->value;

		mesh->ReadObject(false);
	}
	else {
		mesh = nullptr;
	}

	//lodInfo = new FPArray<FStaticMeshComponentLODInfo>(&data);
}

StaticMeshComponent::~StaticMeshComponent()
{
	if (lodInfo) {
		delete lodInfo;
		lodInfo = nullptr;
	}
}

void SkeletalMeshComponent::ReadData(FIStream & data)
{
	MeshComponent::ReadData(data);

	FPropertyTag* prop = GetPropByName("SkeletalMesh");
	if (prop) {
		mesh = package->uObjectForIndex(*(int32*)prop->value);
	}
	else {
		mesh = nullptr;
	}
}
