#include "TerrainComponent.h"
#include "FPropertyTag.h"
#include "D3D.h"
#include "Terrain.h"

#include <algorithm>
#include "UMath.h"


TerrainComponent::~TerrainComponent()
{
	if (collisionVertices) {
		delete[] collisionVertices;
		collisionVertices = nullptr;
	}

	if (BVTree) {
		delete BVTree;
		BVTree = nullptr;
	}

	if (vertexBuffer) {
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	if (indexBuffer) {
		indexBuffer->Release();
		indexBuffer = nullptr;
	}

	if (vertices) {
		delete[] vertices;
		vertices = nullptr;
	}

	if (indices) {
		delete[] indices;
		indices = nullptr;
	}
}

const D3DXMATRIX TerrainComponent::LocalToWorld(D3DXVECTOR3 terrainLocation) const
{
	D3DXMATRIX Result;

	const Vector3 vDrawScale3D = scale3D();
	const Vector3 vLocation = location();
	const Vector3 vTranslation = translation();
	const Vector3 vPosition = vLocation + vTranslation;
	const D3DXVECTOR3 Location = { vLocation.v[0],vPosition.v[1],vPosition.v[2] };
	const D3DXVECTOR3 DrawScale3D = { vDrawScale3D.v[0], vDrawScale3D.v[1], vDrawScale3D.v[2] };
	const float DrawScale = scale();
	const FRotator Rotation = rotator();

	const FLOAT	SR = GMath.SinTab(Rotation.roll),
		SP = GMath.SinTab(Rotation.pitch),
		SY = GMath.SinTab(Rotation.yaw),
		CR = GMath.CosTab(Rotation.roll),
		CP = GMath.CosTab(Rotation.pitch),
		CY = GMath.CosTab(Rotation.yaw);

	const FLOAT	LX = Location.x,
		LY = Location.y,
		LZ = Location.z,
		PX = terrainLocation.x,	   // prepivot on the terrain location
		PY = terrainLocation.y,	   // prepivot on the terrain location
		PZ = terrainLocation.z;	   // prepivot on the terrain location

	const FLOAT	DX = DrawScale3D.x * DrawScale,
		DY = DrawScale3D.y * DrawScale,
		DZ = DrawScale3D.z * DrawScale;

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

void TerrainComponent::ReadData(FIStream & data)
{
	PrimitiveComponent::ReadData(data);

	collisionVerticesCount = data.ReadInt32();
	if (collisionVerticesCount) {
		collisionVertices = new FVector3[collisionVerticesCount];
		for (size_t i = 0; i < collisionVerticesCount; i++)
		{
			collisionVertices[i].x = data.ReadFloat();
			collisionVertices[i].y = data.ReadFloat();
			collisionVertices[i].z = data.ReadFloat();
		}
	}
	else {
		collisionVertices = nullptr;
	}

	BVTree = new FTerrainBVTree(&data);

	FPropertyTag * tag = GetPropByName("SectionBaseX");
	if (tag) {
		SectionBaseX = *(INT*)tag->value;
	}
	else {
		SectionBaseX = 0;
	}

	tag = GetPropByName("SectionBaseY");
	if (tag) {
		SectionBaseY = *(INT*)tag->value;
	}
	else {
		SectionBaseY = 0;
	}

	tag = GetPropByName("SectionSizeX");
	if (tag) {
		SectionSizeX = *(INT*)tag->value;
	}
	else {
		SectionSizeX = 0;
	}

	tag = GetPropByName("SectionSizeY");
	if (tag) {
		SectionSizeY = *(INT*)tag->value;
	}
	else {
		SectionSizeY = 0;
	}

	tag = GetPropByName("TrueSectionSizeX");
	if (tag) {
		TrueSectionSizeX = *(INT*)tag->value;
	}
	else {
		TrueSectionSizeX = 0;
	}

	tag = GetPropByName("TrueSectionSizeY");
	if (tag) {
		TrueSectionSizeY = *(INT*)tag->value;
	}
	else {
		TrueSectionSizeY = 0;
	}
}

void TerrainComponent::Init(Terrain * terrain) {
	auto* d3dDevice = GetD3DDevice();
	//vertex buffer

	INT TessellationLevel = 1;
	const D3DXVECTOR3& drawScale3D = terrain->DrawScale3D;
	FLOAT DrawScaleXY = terrain->DrawScale3D.x * terrain->drawScale;

	INT SectSizeX = SectionSizeX;
	INT SectSizeY = SectionSizeY;
	vertexCount = (SectSizeX * TessellationLevel + 1) * (SectSizeY * TessellationLevel + 1);
	INT QuadSizeX = (TrueSectionSizeX / SectionSizeX);
	INT QuadSizeY = (TrueSectionSizeY / SectionSizeY);
	INT PackedCount = 0;

	FLOAT ScaleX = terrain->DrawScale3D.z / terrain->DrawScale3D.x;
	FLOAT ScaleY = terrain->DrawScale3D.z / terrain->DrawScale3D.y;
	INT TerrainMaxTessLevel = 1;

	const WORD GridLength = TerrainMaxTessLevel / TessellationLevel;

	float DrawScaleZ = terrain->drawScale * terrain->DrawScale3D.z;

	vertices = new D3DVertex[vertexCount];
	ZeroMemory(vertices, sizeof(D3DVertex) * vertexCount);

	float sscale = 1;
	D3DVertex* DestVertex = vertices;

	for (INT Y = 0; Y <= SectSizeY; Y++)
	{
		for (INT X = 0; X <= SectSizeX; X++)
		{
			for (INT SmoothY = 0; SmoothY < (Y < SectSizeY ? TessellationLevel : 1); SmoothY++)
			{
				for (INT SmoothX = 0; SmoothX < (X < SectSizeX ? TessellationLevel : 1); SmoothX++)
				{
					INT OffsetX = SmoothX * GridLength;
					INT OffsetY = SmoothY * GridLength;
					INT LocalX = X * QuadSizeX + OffsetX;
					INT LocalY = Y * QuadSizeY + OffsetY;

					INT TrueSectionX = (SectionBaseX + LocalX);
					INT TrueSectionY = (SectionBaseY + LocalY);

					INT TrueX = terrain->location.x + (TrueSectionX * 64);
					INT TrueY = terrain->location.y + (TrueSectionY * 64);

					DestVertex->position.x = TrueX;
					DestVertex->position.y = TrueY;

					WORD Z = terrain->Height(TrueSectionX, TrueSectionY);
					//(-32767.5f + heights[Y * numVerticesX + X]) / 2.0f;
					//DestVertex->position.z = (float)((-32767.5f + Z) * TERRAIN_ZSCALE);
					//DestVertex->position.z = (float)((-32768.0f + Z)  / 2.f);
					DestVertex->position.z = (-32768.0f + (FLOAT)Z);
					DestVertex->position.z *= TERRAIN_ZSCALE;
					DestVertex->position.z *=/* DrawScaleZ **/ 64;

					//flip terrain
					//DestVertex->position.x = TrueX;
					float temp = DestVertex->position.y;
					DestVertex->position.y = DestVertex->position.z;
					DestVertex->position.z = -temp;

					minVertex.x = std::fmin(minVertex.x, DestVertex->position.x);
					minVertex.y = std::fmin(minVertex.y, DestVertex->position.y);
					minVertex.z = std::fmin(minVertex.z, DestVertex->position.z);

					maxVertex.x = std::fmax(maxVertex.x, DestVertex->position.x);
					maxVertex.y = std::fmax(maxVertex.y, DestVertex->position.y);
					maxVertex.z = std::fmax(maxVertex.z, DestVertex->position.z);

					DestVertex++;
					PackedCount++;
				}
			}
		}
	}

	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale,
		1.0f,
		DrawScaleZ,
		1.0f);
	
	//for (INT i = 0; i < vertexCount; i++)
	//{
	//	D3DXVec3TransformCoord(&vertices[i].position, &vertices[i].position, &scale);
	//}

	//for (INT Y = 0; Y <= SectSizeY; Y++)
	//{
	//	for (INT X = 0; X <= SectSizeX; X++)
	//	{
	//		INT LocalX = X + (QuadSizeX);
	//		INT LocalY = Y + (QuadSizeY);
	//		INT TrueX = SectionBaseX + LocalX;
	//		INT TrueY = SectionBaseY + LocalY;
	//
	//		DestVertex->position.x = LocalX;
	//		DestVertex->position.y = LocalY;
	//		//DestVertex->position.z = terrain->location.z + (((-32768.0f + (FLOAT)(terrain->Height(TrueX, TrueY)))) * TERRAIN_ZSCALE)* sscale;
	//		DestVertex->position.z = (-32768.0f + terrain->Height(TrueX, TrueY)) * TERRAIN_ZSCALE;
	//
	//		DestVertex++;
	//	}
	//}

	//D3DXMATRIX transform = LocalToWorld(D3DXVECTOR3(SectionBaseX, SectionBaseY, 0.0f));
	//D3DXMATRIX transform;
	//
	//D3DXMatrixTranslation(&transform, terrain->location.x, terrain->location.y, terrain->location.z);
	//
	//for (size_t i = 0; i < vertexCount; i++)
	//{
	//	D3DXVec3TransformCoord(&vertices[i].position, &vertices[i].position, &transform);
	//}

	//normals

	CalculateNormals();

	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(D3DVertex) * vertexCount;
	bDesc.StructureByteStride = 0;
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA bData;
	bData.pSysMem = vertices;
	bData.SysMemPitch = 0;
	bData.SysMemSlicePitch = 0;

	HRESULT result = d3dDevice->CreateBuffer(&bDesc, &bData, &vertexBuffer);
	if (FAILED(result)) {
		throw std::exception("Failed to create vertex buffer for terrain component");
	}

	//index buffer
	INT SectBaseX = SectionBaseX;
	INT SectBaseY = SectionBaseY;
	UINT Stride = sizeof(uint16);
	UINT Count = 2 * 3 * SectSizeX * SectSizeY;
	indexCount = Count;
	indices = new uint16[Count];
	ZeroMemory(indices, sizeof(uint16) * Count);

	uint16* DestIndex = indices;
	NumVisibleTriangles = 0;

	for (INT Y = 0; Y < SectSizeY; Y++)
	{
		for (INT X = 0; X < SectSizeX; X++)
		{
			//if (terrain->IsTerrainQuadVisible(SectBaseX + X, SectBaseY + Y) == FALSE)
			//{
			//	continue;
			//}

			INT	V1 = Y*(SectSizeX + 1) + X;
			INT	V2 = V1 + 1;
			INT	V3 = (Y + 1)*(SectSizeX + 1) + (X + 1);
			INT	V4 = V3 - 1;

			if (terrain->IsTerrainQuadFlipped(SectBaseX + X, SectBaseY + Y) == FALSE)
			{
				*DestIndex++ = V1;
				*DestIndex++ = V4;
				*DestIndex++ = V3;
				NumVisibleTriangles++;

				*DestIndex++ = V3;
				*DestIndex++ = V2;
				*DestIndex++ = V1;
				NumVisibleTriangles++;
			}
			else
			{
				*DestIndex++ = V1;
				*DestIndex++ = V4;
				*DestIndex++ = V2;
				NumVisibleTriangles++;

				*DestIndex++ = V2;
				*DestIndex++ = V4;
				*DestIndex++ = V3;
				NumVisibleTriangles++;
			}
		}
	}

	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = Count * Stride;

	bData.pSysMem = indices;

	result = d3dDevice->CreateBuffer(&bDesc, &bData, &indexBuffer);
	if (FAILED(result)) {
		throw std::exception("Failed to create index buffer for terrain component");
	}
}

void TerrainComponent::Render(ID3D11DeviceContext * context)
{
	UINT Stride = sizeof(D3DVertex);
	UINT Offset = 0;

	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &Stride, &Offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->DrawIndexed(NumVisibleTriangles * 3, 0, 0);
}

void TerrainComponent::CalculateNormals()
{
	int width = SectionSizeX + 1;
	int height = SectionSizeY + 1;

	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;

	// Go through all the faces in the mesh and calculate their normals.
	for (i = 0; i < height - 1; i++)
	{
		for (j = 0; j < width - 1; j++)
		{
			index1 = (j * height) + i;
			index2 = (j * height) + (i + 1);
			index3 = ((j + 1) * height) + i;

			// Get three vertices from the face.
			vertex1[0] = vertices[index1].position.x;
			vertex1[1] = vertices[index1].position.y;
			vertex1[2] = vertices[index1].position.z;

			vertex2[0] = vertices[index2].position.x;
			vertex2[1] = vertices[index2].position.y;
			vertex2[2] = vertices[index2].position.z;

			vertex3[0] = vertices[index3].position.x;
			vertex3[1] = vertices[index3].position.y;
			vertex3[2] = vertices[index3].position.z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * height) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			vertices[index].normal.x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			vertices[index].normal.y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			vertices[index].normal.z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);

		}
	}
}

FTerrainPatchBounds::FTerrainPatchBounds(FIStream * stream)
{
	minHeight = stream->ReadFloat();
	maxHeight = stream->ReadFloat();
	maxDisplacement = stream->ReadFloat();
}

FWeightmapLayerAllocationInfo::FWeightmapLayerAllocationInfo(FIStream * stream)
{
	LayerName = FName(stream);
	WeightmapTextureIndex = stream->ReadUInt8();
	WeightmapTextureChannel = stream->ReadUInt8();
}

LandscapeComponent::LandscapeComponent(FIStream * stream)
	:PrimitiveComponent()
{
	/*if (Ar.Ver() >= VER_LANDSCAPECOMPONENT_LIGHTMAPS)
	{
		Ar << LightMap;
	}*/
}
