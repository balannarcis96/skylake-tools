#ifndef FSTATIC_MESH_H
#define FSTATIC_MESH_H

#include "FReadable.h"
#include "FMesh.h"
#include "FVector.h"

#include <D3D11.h>

struct UObject;
struct FLightMap;
struct _D3DDriver;

struct FFragmentRange
{
	FFragmentRange(FIStream * stream)
	{
		baseIndex = stream->ReadInt32();
		numPrimitives = stream->ReadInt32();
	}

	int32 baseIndex;
	int32 numPrimitives;
};

struct FUVFloat1
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshFloatUV    UVs;
};

struct FUVFloat2
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshFloatUV    UVs[2];
};

struct FUVFloat3
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshFloatUV    UVs[3];
};

struct FUVFloat4
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshFloatUV    UVs[4];
};

struct FUVHalf1
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshHalfUV     UVs;
};

struct FUVHalf2
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshHalfUV     UVs[2];
};

struct FUVHalf3
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshHalfUV     UVs[3];
};

struct FUVHalf4
{
	FPackedNormal   normal[2];
	int32             color;
	FMeshHalfUV     UVs[4];
};

struct FVert : FReadable {
	int32 pVertex;
	int32 iSide;
	FVector2 shadowTexCoord;
	FVector2 backfaceShadowTexCoord;

	FVert();
	FVert(FIStream * stream);
};

struct FPositionVertexData : FReadable {
	D3DXVECTOR3 *vertices;
	UINT stride;
	int32 count;

	FPositionVertexData();
	FPositionVertexData(FIStream* stream);
	~FPositionVertexData();

	const D3DXVECTOR3* GetDate()const { return vertices; }
};

struct FUVVertexData : FReadable {
	uint8 * data{ nullptr };
	int32 numUVStets;
	int32 stride;
	int32 numUVs;
	int32 bUseFullUVs;
	int32 size;

	FUVVertexData();
	FUVVertexData(FIStream  * stream);
	~FUVVertexData();
};

struct FStaticMeshSection : FReadable {
	int32
		material,
		enableCollision,
		oldEnableCollision,
		bEnableShadowCasting,
		firstIndex,
		faceCount,
		minVertexIndex,
		maxVertexIndex,
		materialIndex,
		fragmentCount;

	ID3D11Buffer * indexBuffer = nullptr;
	uint8 * indices;
	uint32 indexSize;
	uint32 indexCount;
	DXGI_FORMAT dxgiFormat;

	std::vector<FFragmentRange> fragments;

	FStaticMeshSection();
	FStaticMeshSection(FIStream * stream);
	~FStaticMeshSection();

	void Init(FMultiSizeIndexContainer *indices);
	void Render(ID3D11DeviceContext * driver);
};

struct FStaticColorData :FReadable {
	int32 stride;
	int32 count;
	uint8 * data{ nullptr };

	FStaticColorData();
	FStaticColorData(FIStream *stream);
	~FStaticColorData();
};

struct FStaticLodInfo : FReadable {
	int32 sectionsCount;
	FStaticMeshSection* sections;
	FPositionVertexData *possitionBuffer;
	FUVVertexData* textureBuffer;
	FStaticColorData colorBuffer;
	FMultiSizeIndexContainer* indexContainer;
	FMultiSizeIndexContainer wireframeIndexBuffer;
	int32 legacyEdgeCount;
	FEdge* legacyEdges;
	int32 numVertices;

	FStaticLodInfo();
	FStaticLodInfo(FIStream* stream);
	~FStaticLodInfo();

	void Render(bool defaultVBuffer = true);
	void Init();

	ID3D11Buffer *vertexBuffer;
	struct D3DVertex * vertices;
};

struct FStaticMeshComponentLODInfo :FReadable {
	FArray<UObject*> shadowMaps;
	FArray<UObject*> shadowVertexBuffers;
	FLightMap *lightMap;
	UObject * unkShadowMap;

	FStaticMeshComponentLODInfo();
	FStaticMeshComponentLODInfo(FIStream * stream);
	~FStaticMeshComponentLODInfo();
};

void ReadFStaticLodInfo(FIStream * stream, FStaticLodInfo* lod);

#endif