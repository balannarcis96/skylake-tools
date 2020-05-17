#ifndef TERRAINCOMPONENT_H
#define TERRAINCOMPONENT_H

#include <D3D11.h>

#include "UComponent.h"
#include "FArray.h"
#include "FTerrain.h"
#include "typeDefs.h"
#include  "Actor.h"
#include "FGlobalMath.h"

struct Terrain;
struct FTerrainPatch :FReadable {
	float heights[4 * 4];
};

struct FTerrainPatchBounds :FReadable {
	float
		minHeight,
		maxHeight,
		maxDisplacement;

	FTerrainPatchBounds(FIStream* stream);
};

struct TerrainComponent : PrimitiveComponent {
	uint32	collisionVerticesCount;
	FVector3* collisionVertices;
	FTerrainBVTree* BVTree;
	int32 	SectionBaseX;
	int32 	SectionBaseY;
	int32 	SectionSizeX;
	int32 	SectionSizeY;
	int32 	TrueSectionSizeX;
	int32 	TrueSectionSizeY;
	
	Terrain * terrain;

	D3DXVECTOR3 maxVertex{ FLT_MIN,FLT_MIN,FLT_MIN };
	D3DXVECTOR3 minVertex{ FLT_MAX,FLT_MAX,FLT_MAX };
	D3DXVECTOR3 minY{ FLT_MAX,FLT_MAX,FLT_MAX };
	D3DXVECTOR3 minZ{ FLT_MAX,FLT_MAX,FLT_MAX };

	TerrainComponent()
		:collisionVertices(nullptr),
		collisionVerticesCount(0),
		BVTree(nullptr) {}

	~TerrainComponent();

	void Init(Terrain * terrain);
	void Render(ID3D11DeviceContext * context);
	const Vector3 GetPos()const {
		if (!vertices) {
			return{ 0.0f,0.0f ,0.0f };
		}

		return{ vertices[0].position.x,vertices[0].position.y,vertices[0].position.z };
	}

	const D3DXVECTOR3 GetLocalVertex(INT X, INT Y, INT  numVerticesX, INT numVerticesY, INT16 * heights) const
	{
		return D3DXVECTOR3(
			X,
			Y,
			(-32768.0f + (FLOAT)Height_T(SectionBaseX + X, SectionBaseY + Y, numVerticesX, numVerticesY, heights)) * TERRAIN_ZSCALE
		);
	}
	const D3DXVECTOR3 GetWorldVertex(INT X, INT Y, INT  numVerticesX, INT numVerticesY, INT16 * heights, D3DXVECTOR3 terrainLocation) const
	{
		D3DXMATRIX LToW = LocalToWorld(terrainLocation);
		D3DXVECTOR4 outVert = D3DXVECTOR4(
			X,
			Y,
			(-32768.0f + (FLOAT)Height_T(SectionBaseX + X, SectionBaseY + Y, numVerticesX, numVerticesY, heights)) * TERRAIN_ZSCALE,
			0
		);

		D3DXVec3Transform(&outVert, (D3DXVECTOR3*)&outVert, &LToW);

		return (D3DXVECTOR3)outVert;
	}
	const D3DXMATRIX LocalToWorld(D3DXVECTOR3 terrainLocation) const;

	void ReadData(FIStream &data);
	void CalculateNormals();

	ID3D11Buffer * vertexBuffer = nullptr;
	ID3D11Buffer * indexBuffer = nullptr;
	D3DVertex * vertices = nullptr;
	uint16 *indices = nullptr;
	uint32 vertexCount = 0;
	uint32 indexCount = 0;
	uint32  NumVisibleTriangles = 0;
};

struct FWeightmapLayerAllocationInfo : FReadable
{
	FName LayerName;
	BYTE WeightmapTextureIndex;
	BYTE WeightmapTextureChannel;

	FWeightmapLayerAllocationInfo()
		: LayerName()
		, WeightmapTextureIndex(255)	// Indicates an invalid allocation
		, WeightmapTextureChannel(255)
	{}

	FWeightmapLayerAllocationInfo(FIStream * stream);
};

struct LandscapeComponent : PrimitiveComponent {
	INT SectionBaseX;
	INT SectionBaseY;
	INT ComponentSizeQuads;
	INT SubsectionSizeQuads;
	INT NumSubsections;

	LandscapeComponent()
		:PrimitiveComponent()
	{}

	LandscapeComponent(FIStream * stream);
};

#endif