#ifndef STATICMESH_H
#define STATICMESH_H

#include "UObject.h"
#include "FBoxSphereBounds.h"
#include "FStaticMesh.h"
#include "FString.h"
#include "FArray.h"
#include "FRotator.h"

#include <d3d11.h>
#include <fstream>

struct kDOPNode
{
	Vector3  boundingVolumeMin;
	Vector3  boundingVolumeMax;
	int         bIsLeaf;
	union
	{
		// This structure contains the left and right child kDOP indices
		// These index values correspond to the array in the FkDOPTree
		struct
		{
			short LeftNode;
			short RightNode;
		} n;
		// This structure contains the list of enclosed triangles
		// These index values correspond to the triangle information in the
		// FkDOPTree using the start and count as the means of delineating
		// which triangles are involved
		struct
		{
			short NumTriangles;
			short StartIndex;
		} t;
	};
};

struct StaticMesh : UObject {
	FBoxSphereBounds bounds;
	FString sourceFile;
	UObject * bodySetup;
	BOOL isInit = FALSE;
	BOOL hasRead = FALSE;

	int32 kDOPNodeCount;
	kDOPNode * kDOPNodes;
	int32 kDOPTriangleCount;
	_int64 *kDOPTriangles;

	int32 legacykDOPTreeFlags;
	int32 legacykDOPTreeCount;
	int32 legacykDOPTreeSize;
	int32 legacykDOPTreeOffset;
	int32 legacykDOPTree;

	int32  stringsCount;
	FString* strings;
	int32 version;

	FStaticLodInfo* lodInfo;
	int32 lodInfoCount;
	FRotator thumbnailAngle;
	int32 thumbNailDistance;
	FArray<FVector3> physMeshScale3D;
	int32 unk;

	~StaticMesh();
	void Render(bool defaultVBuffer = true);
	void ReadData(FIStream& data);
	void Init();
	void ExportObj(std::ofstream&  file);
};

#endif
