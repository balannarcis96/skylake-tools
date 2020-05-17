#include "Terrain.h"
#include "FPropertyTag.h"
#include <Windows.h>
#include <fstream>
#include "D3D.h"
#include "TerrainComponent.h"

#define SKYLAKE_RESOURCE_MAGIC 0x5f75990c
#define SKYLAKE_RESOURCE_LEVEL 1
#define SKYLAKE_RESOURCE_TERRAIN 2

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

Terrain::~Terrain()
{
	if (heights) {
		delete[] heights;
		heights = nullptr;
	}

	if (layers) {
		delete[] layers;
		layers = nullptr;
	}

	if (infoData) {
		delete[] infoData;
		infoData = nullptr;
	}
}

void Terrain::Init() {
	if (exportObject->objectFlags & RF_ClassDefaultObject) {
		return;
	}

	ZeroMemory(exportData.name, 256);
	exportData.version = 1;
	strcpy_s(exportData.name, package->Name());

	terrainSizeX = (numPatchesX * DrawScale3D.x);
	terrainSizeY = (numPatchesX * DrawScale3D.y);

	for (size_t i = 0; i < components.size(); i++) {
		if (components[i]) {
			components[i]->Init(this);

			minVertex.x = std::fmin(minVertex.x, components[i]->minVertex.x);
			minVertex.y = std::fmin(minVertex.y, components[i]->minVertex.y);
			minVertex.z = std::fmin(minVertex.z, components[i]->minVertex.z);

			maxVertex.x = std::fmax(maxVertex.x, components[i]->maxVertex.x);
			maxVertex.y = std::fmax(maxVertex.y, components[i]->maxVertex.y);
			maxVertex.z = std::fmax(maxVertex.z, components[i]->maxVertex.z);
		}
	}

	TerrainComponent* c = components[0];

	INT QuadSize = c->TrueSectionSizeX / c->SectionSizeX;

	INT DrawScale = (drawScale * DrawScale3D.x  * DrawScale3D.y);

	INT ScaleX = (numPatchesX * DrawScale);
	INT ScaleY = (numPatchesY * DrawScale);

	width = ScaleX * QuadSize;
	height = ScaleY * QuadSize;

}

void Terrain::Render()
{
	if (exportObject->objectFlags & RF_ClassDefaultObject) {
		return;
	}

	ID3D11DeviceContext * context = GetD3DDeviceContext();

	//UINT Stride = sizeof(D3DVertex);
	//UINT Offset = 0;
	//
	//context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	//context->IASetVertexBuffers(0, 1, &vertexBuffer, &Stride, &Offset);
	//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//
	//context->DrawIndexed(NumVisibleTriangles * 3, 0, 0);

	for (size_t i = 0; i < components.size(); i++)
	{
		components[i]->Render(context);
	}
}

void Terrain::ExportHeightMapToBmp(const char* fileName)
{
	FBitmapFileHeader bmf;
	FBitmapInfoHeader bmhdr;

	bmf.bfType = 'B' + (256 * (INT)'M');
	bmf.bfReserved1 = 0;
	bmf.bfReserved2 = 0;
	INT biSizeImage;

	biSizeImage = numVerticesX * numVerticesY * 2;
	bmf.bfOffBits = sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader);
	bmhdr.biBitCount = 16;

	bmf.bfSize = bmf.bfOffBits + biSizeImage;
	// Info header.
	bmhdr.biSize = sizeof(FBitmapInfoHeader);
	bmhdr.biWidth = numVerticesX;
	bmhdr.biHeight = numVerticesY;
	bmhdr.biPlanes = 1;
	bmhdr.biCompression = 0;
	bmhdr.biSizeImage = biSizeImage;
	bmhdr.biXPelsPerMeter = 0;
	bmhdr.biYPelsPerMeter = 0;
	bmhdr.biClrUsed = 0;
	bmhdr.biClrImportant = 0;
	//fileStream.Resize(numVerticesX * numVerticesY * 2);

	//for (INT i = numVerticesY - 1; i >= 0; i--)
	//{
	//	fileStream.Write((uint8*)(&heights[(i * numVerticesX)]), (numVerticesX * 2));
	//}
	FIStream  fileStream;
	fileStream.WriteInt32(numVerticesX);
	fileStream.WriteInt32(numVerticesY);
	fileStream.WriteInt32(numVerticesX * numVerticesY * 2);
	fileStream.Write((uint8*)heights, numVerticesX * numVerticesY * 2);

	fileStream.save_to_file(fileName);
}

void Terrain::Export(FIStream & file)
{
	//file.Write((uint8*)heights, (numVerticesX * numVerticesY) * sizeof(uint16));
	//
	//
	//
	//return;

	TerrainComponent * component = components[0];

	INT SectSizeX = component->SectionSizeX;
	INT SectSizeY = component->SectionSizeY;

	INT TrueSectSizeX = component->TrueSectionSizeX;
	INT TrueSectSizeY = component->TrueSectionSizeY;

	INT QuadSizeX = (TrueSectSizeX / SectSizeX);
	INT QuadSizeY = (TrueSectSizeY / SectSizeY);

	INT SectX = this->numSectionsX;
	INT SectY = this->numSectionsY;

	INT NumPatchesX = this->numVerticesX - 1;
	INT NumPatchesY = this->numVerticesY - 1;

	INT TNumSectionsX = this->numSectionsX;
	INT TNumSectionsY = this->numSectionsY;

	INT TNumComponents = (INT)this->components.size();

	file.WriteFloat(location.x);
	file.WriteFloat(location.y);
	file.WriteFloat(location.z);

	//start position to be filled by other tool
	file.WriteFloat(0.0f);
	file.WriteFloat(0.0f);

	file.WriteInt32(numVerticesX);
	file.WriteInt32(numVerticesY);

	INT ScaleX = drawScale * DrawScale3D.x * QuadSizeX;
	INT ScaleY = drawScale * DrawScale3D.y * QuadSizeY;

	//width
	UINT32 width = (UINT32)(numPatchesX * drawScale * DrawScale3D.x * QuadSizeX);
	width = numPatchesX * 64 * 2;
	file.WriteUInt32(width);

	//height
	UINT32 height = (UINT32)(numPatchesX * drawScale * DrawScale3D.y * QuadSizeY);
	height = numPatchesY * 64 * 2;
	file.WriteUInt32(height);

	file.Write((uint8*)heights, (numVerticesX * numVerticesY) * sizeof(uint16));
	file.WriteInt32((numVerticesX * numVerticesY) * sizeof(uint16));
}

int interpolate(int startValue, int endValue, int stepNumber, int lastStepNumber)
{
	return (endValue - startValue) * stepNumber / lastStepNumber + startValue;
}

float __lerp(float v0, float v1, float t) {
	return (1 - t) * v0 + t * v1;
}

void LerpRGB(RGBTRIPLE* out_color, float t)
{
	out_color->rgbtRed = 128 * t;
	out_color->rgbtGreen = 128 * t;
	out_color->rgbtBlue = 128 * t;
}

void Terrain::ReadData(FIStream & data)
{
	UObject::ReadData(data);

	FPropertyTag* tag = GetPropByName("NumVerticesX");
	numVerticesX = *(int32*)tag->value;

	tag = GetPropByName("NumVerticesY");
	numVerticesY = *(int32*)tag->value;

	tag = GetPropByName("NumSectionsX");
	numSectionsX = *(int32*)tag->value;

	tag = GetPropByName("NumSectionsY");
	numSectionsY = *(int32*)tag->value;

	tag = GetPropByName("NumPatchesX");
	numPatchesX = *(int32*)tag->value;

	tag = GetPropByName("NumPatchesY");
	numPatchesY = *(int32*)tag->value;

	tag = GetPropByName("MaxTesselationLevel");
	if (tag) {
		maxTesselationLevel = *(int32*)tag->value;
	}
	else {
		maxTesselationLevel = 1;
	}

	tag = GetPropByName("TerrainComponents");
	if (tag) {
		for (size_t i = 0; i < tag->elementCount; i++)
		{
			components.push_back((TerrainComponent*)package->uObjectForIndex(*((int32*)tag->value + i)));
		}

	}

	tag = GetPropByName("Location");
	if (tag) {
		location.x = ((FVector3*)tag->value)->x;
		location.y = ((FVector3*)tag->value)->y;
		location.z = ((FVector3*)tag->value)->z;
	}
	else {
		location.x = 0.0f;
		location.y = 0.0f;
		location.z = 0.0f;
	}

	tag = GetPropByName("DrawScale");
	if (tag) {
		drawScale = 1.0f / *(float*)tag->value;
	}
	else {
		drawScale = 1.0f;
	}

	tag = GetPropByName("DrawScale3D");
	if (tag) {
		FVector3 * drawScale3D = (FVector3*)tag->value;
		DrawScale3D.x = drawScale3D->x * drawScale;
		DrawScale3D.y = drawScale3D->y * drawScale;
		DrawScale3D.z = drawScale3D->z * drawScale;
	}
	else {
		DrawScale3D.x = 1.0f * drawScale;
		DrawScale3D.y = 1.0f * drawScale;
		DrawScale3D.z = 1.0f * drawScale;
	}

	data._pos = rawDataOffset;

	heightsCount = data.ReadInt32();
	if (heightsCount) {
		heights = new int16[heightsCount];
		ZeroMemory(heights, heightsCount * sizeof(int16));
		data.Read((uint8*)heights, sizeof(int16) * heightsCount);
	}
	else {
		heights = nullptr;
	}

	infoDataCount = data.ReadInt32();
	if (infoDataCount) {
		infoData = new FTerrainInfoData[infoDataCount];
		ZeroMemory(infoData, sizeof(FTerrainInfoData) * infoDataCount);

		for (size_t i = 0; i < infoDataCount; i++)
		{
			infoData[i].Data = data.ReadUInt8();
		}
	}
	else {
		infoData = nullptr;
	}

	alphaMaps = data.ReadInt32();
	weightedTextureMaps = FArray<UObject>(&data);

	//read chached materials

	//vertices = new D3DVertex[numVerticesX * numVerticesY];
	//ZeroMemory(vertices, (numVerticesX * numVerticesY) * sizeof(D3DVertex));

	////read from UnTerrainRender.cpp:1216

	//float scale = 130;
	//for (size_t y = 0; y < numVerticesY; ++y)
	//{
	//	for (size_t x = 0; x < numVerticesX; ++x)
	//	{
	//		vertices[(x * numVerticesX) + y].position.x = (float)(x * scale) + location.x;
	//		vertices[(x * numVerticesX) + y].position.y = (float)((((-32768.0f + Height_T(x, y, numVerticesX, numVerticesY, heights)) * TERRAIN_ZSCALE) * scale)) + location.y;
	//		vertices[(x * numVerticesX) + y].position.z = (float)(y * scale) + location.z;
	//	}
	//}

	//CalculateNormals();
	//ExportHeightMapToBmp("E:\\test_heightmap.bmp");

	if (package) {
		package->terrainTiles.push_back(this);
	}
}

int32 clampI32(int32 min, int32 max, int32 val)
{
	if (val >= max) {
		return max;
	}

	if (val <= min) {
		return min;
	}

	return val;
}
