#ifndef TERRAIN_H
#define TERRAIN_H

#include "UObject.h"
#include "FArray.h"
#include "FString.h"
#include "FTerrain.h"
#include "FGlobalMath.h"

#include "D3D.h"
#include "FMesh.h"
#include <D3D11.h>
#include <Windows.h>

struct TerrainComponent;
#define UE3_UNIT_PIXELS 75.591f

int32 clampI32(int32 min, int32 max, int32 val);


// .BMP file header.
#pragma pack(push,1)
struct FBitmapFileHeader
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
};
#pragma pack(pop)

// .BMP subheader.
#pragma pack(push,1)
struct FBitmapInfoHeader
{
	DWORD biSize;
	DWORD biWidth;
	DWORD biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	DWORD biXPelsPerMeter;
	DWORD biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
};
#pragma pack(pop)

struct FTerrainVertex
{
	union
	{
		DWORD PackedCoordinates;
		struct
		{
			BYTE
				X,
				Y,
				Z_LOBYTE,
				Z_HIBYTE;
		};
	};

	int16
		GradientX,
		GradientY;
};

struct TerrainExport {
	uint32 version;
	char name[256];
};

struct Terrain : UObject {
	uint32 heightsCount;
	int16* heights;
	uint32 infoDataCount;
	FTerrainInfoData* infoData;
	FArray<FTerrainLayer> *layers = nullptr;
	FArray<UObject> weightedTextureMaps;
	int32 alphaMaps;
	FArray<UObject> cachedTerrainMaterials;
	std::vector<TerrainComponent*> components;

	INT terrainSizeX;
	INT terrainSizeY;
	INT width;
	INT height;

	inline const INT GetWidth() const {
		return (numPatchesX * drawScale * DrawScale3D.x);
	}

	inline const INT GetHeight() const {
		return (numPatchesY * drawScale * DrawScale3D.y);
	}

	int32 numVerticesX;
	int32 numVerticesY;
	int32 numSectionsX;
	int32 numSectionsY;
	int32 numPatchesX;
	int32 numPatchesY;
	int32 maxTesselationLevel;
	float drawScale;
	D3DXVECTOR3 DrawScale3D;
	D3DXVECTOR3 location;
	TerrainExport exportData;

	ID3D11Buffer * vertexBuffer = nullptr;
	ID3D11Buffer * indexBuffer = nullptr;
	D3DVertex * vertices = nullptr;
	uint16 *indices = nullptr;
	uint32 vertexCount = 0;
	uint32 indexCount = 0;
	uint32 NumVisibleTriangles = 0;
	D3DXVECTOR3 minVertex{ FLT_MAX,FLT_MAX,FLT_MAX };
	D3DXVECTOR3 maxVertex{ FLT_MIN,FLT_MIN,FLT_MIN };

	Terrain()
		:heights(nullptr),
		layers(nullptr),
		infoData(nullptr) {}

	~Terrain();

	void CalculateNormals();

	inline const FTerrainInfoData* GetInfoData(INT X, INT Y) const
	{
		X = clampI32(X, 0, numVerticesX - 1);
		Y = clampI32(Y, 0, numVerticesY - 1);

		return  infoData + (Y * numVerticesX + X);
	}
	inline BOOL IsTerrainQuadVisible(INT X, INT Y) const
	{
		const FTerrainInfoData* TheInfoData = GetInfoData(X, Y);
		return TheInfoData->IsVisible();
	}
	inline BOOL IsTerrainQuadFlipped(INT X, INT Y) const
	{
		const FTerrainInfoData* TheInfoData = GetInfoData(X, Y);
		return TheInfoData->IsOrientationFlipped();
	}
	__forceinline int16 Height(int32 X, int32 Y)
	{
		X = clampI32(0, numVerticesX - 1, X);
		Y = clampI32(0, numVerticesY - 1, Y);

		return heights[Y * numVerticesX + X];
	}

	inline D3DXMATRIX LocalToWorld()
	{
		D3DXMATRIX Result;

		const float DrawScale = drawScale;
		const D3DXVECTOR3 Location = location;

		const FLOAT	SR = GMath.SinTab(0),
			SP = GMath.SinTab(0),
			SY = GMath.SinTab(0),
			CR = GMath.CosTab(0),
			CP = GMath.CosTab(0),
			CY = GMath.CosTab(0);

		const FLOAT	LX = Location.x,
			LY = Location.y,
			LZ = Location.z,
			PX = 0,	   // no prepivot 
			PY = 0,	   // no prepivot 
			PZ = 0;	   // no prepivot 

		const FLOAT	DX = 1.0f,
			DY = 1.0f,
			DZ = 1.0f;

		Result.m[0][0] = CP * CY * DX;
		Result.m[0][1] = CP * DX * SY;
		Result.m[0][2] = DX * SP;
		Result.m[0][3] = 0.f;

		Result.m[1][0] = DY * (CY * SP * SR - CR * SY);
		Result.m[1][1] = DY * (CR * CY + SP * SR * SY);
		Result.m[1][2] = -CP * DY * SR;
		Result.m[1][3] = 0.f;

		Result.m[2][0] = -DZ * (CR * CY * SP + SR * SY);
		Result.m[2][1] = DZ * (CY * SR - CR * SP * SY);
		Result.m[2][2] = CP * CR * DZ;
		Result.m[2][3] = 0.f;

		Result.m[3][0] = LX - CP * CY * DX * PX + CR * CY * DZ * PZ * SP - CY * DY * PY * SP * SR + CR * DY * PY * SY + DZ * PZ * SR * SY;
		Result.m[3][1] = LY - (CR * CY * DY * PY + CY * DZ * PZ * SR + CP * DX * PX * SY - CR * DZ * PZ * SP * SY + DY * PY * SP * SR * SY);
		Result.m[3][2] = LZ - (CP * CR * DZ * PZ + DX * PX * SP - CP * DY * PY * SR);
		Result.m[3][3] = 1.f;

		return Result;
	}

	//inline WORD BilinearHeight(FLOAT fX,FLOAT fY)
	//{
	//	INT X = floorf(fX);
	//	INT Y = floorf(fY);
	//	FLOAT tx = fX - (FLOAT)X;
	//	FLOAT ty = fY - (FLOAT)Y;
	//
	//	D3DXQUATERNION q1 = D3DXQUATERNION((FLOAT)Height(X, Y), (FLOAT)Height(X + 1, Y), tx, 1.0f);
	//	D3DXQUATERNION q2 = D3DXQUATERNION((FLOAT)Height(X, Y + 1), (FLOAT)Height(X + 1, Y + 1), tx, 1.0f);
	//	D3DXQuaternionSlerp(&q1, &q1, &q2, ty);
	//
	//	//return appRound( );
	//}

	void Init();
	void Render();
	void ExportHeightMapToBmp(const char* fileName);
	void Export(FIStream &);
protected:
	void ReadData(FIStream & data);
};

#endif
