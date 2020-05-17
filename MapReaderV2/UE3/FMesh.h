#ifndef FMESH_H
#define FMESH_H

#include "FReadable.h"
#include "FVector.h"
#include "FArray.h"
#include "FBulkData.h"

#include <D3D10_1.h>
#include <D3DX10math.h>

void _UnpackNormal(uint8 *normal, D3DXVECTOR3* out_normal);

struct FPackedNormal {
	uint8 normal[4]; // x  y  z  w

	FPackedNormal();
	FPackedNormal(float x, float y, float z);

	Vector3 UnpackNormal();
	void operator = (FPackedNormal& norm);
};

struct FEdge {
	int32 vertices[2];
	int32 faces[2];
};

struct GPUVertex {
	Vector3 position;
	Vector3 normal;
	float uv[2];
};

struct FMeshHalfUV {
	int16 uv[2];
};

struct FMeshFloatUV {
	float uv[2];
};

struct FGPUVer3Half {
	Vector3 position;
	FPackedNormal normals[2];
	int32 boneIndex;
	int32 boneWeight;
	FMeshHalfUV uv;
};

struct FSoftVertex {
	Vector3 position;
	FPackedNormal normals[3];
	FMeshFloatUV uv;
	int32 boneIndex;
	int32 boneWeight;
};

struct FRigidVertex {
	Vector3 position;
	FPackedNormal normals[3];
	FMeshFloatUV uv;
	uint8 boneIndex;
};

struct FMeshBone : FReadable {
	FVector4 orientation;
	FVector3 position;
	_int64 nameIndex;
	int32 childrenCount;
	int32 parentIndex;
	int32 flags;
	int32 unk;

	FMeshBone();
	FMeshBone(FIStream * stream);
};

struct FMeshSection : FReadable {
	int16 material;
	int16 chunkIndex;
	int32 firstIndex;
	uint16 faceCount;

	FMeshSection();
	FMeshSection(FIStream * stream);
};

struct FMultiSizeIndexContainer : FReadable {
	int32 elementSize;
	int32 elementCount;

	uint8 *rawData;
	FMultiSizeIndexContainer();
	FMultiSizeIndexContainer(FIStream * stream);

	~FMultiSizeIndexContainer();
};

struct FSkeletalMeshChunk : FReadable {
	int32 firstIndex;

	int rigidVerticsCount;
	FRigidVertex* rigidVertices;


	int softVerticesCount;
	FSoftVertex* softVertices;

	uint32 boneMapCount;
	int16 * boneMap;
	int32 maxBoneInfluences;

	FSkeletalMeshChunk();
	FSkeletalMeshChunk(FIStream * stream);

	~FSkeletalMeshChunk();
};

struct FGPUSkinBuffer : FReadable {
	int32 bUseFullUV;
	int32 bUsePackedPosition;
	int32 elementSize;
	int32 elementCount;
	FGPUVer3Half* vertices;

	FGPUSkinBuffer();
	FGPUSkinBuffer(FIStream * stream);
	~FGPUSkinBuffer();
};

struct FLodInfo : FReadable {
	FArray<FMeshSection> sections;
	FMultiSizeIndexContainer indexContainer;
	FArray<FSkeletalMeshChunk> chunks;
	FBulkData rawPoints;
	int32 size;
	int32 vertexCount;
	FGPUSkinBuffer vertexBufferGPUSkin;
	int32 legacyShadowIndicesCount;
	int16 * legacyShadowIndices{ nullptr };
	int32 legacyEdgeCount;
	FEdge * legacyEdges{ nullptr };
	int32 activeBoneIndicesCount;
	uint16 *activeBoneIndices{ nullptr };
	int32 legacyShadowTriangleDoubleSidedCount;
	uint8* legacyShadowTriangleDoubleSided{ nullptr };
	int32 extraVertexInfluenceCount;
	int32 requiredBonesCount;
	uint8* requiredBones{ nullptr };

	FLodInfo();
	FLodInfo(FIStream * stream);
	~FLodInfo();

	FSoftVertex * vertices();
};

struct D3DVertex {
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 uv;

	D3DVertex() {}
	D3DVertex(const D3DVertex& other) {
		memcpy_s(this, sizeof(D3DVertex), &other, sizeof(D3DVertex));
	}
	void operator=(const D3DVertex& other) {
		memcpy_s(this, sizeof(D3DVertex), &other, sizeof(D3DVertex));
	}
};

#endif
