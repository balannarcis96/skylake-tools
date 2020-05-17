#include "StaticMesh.h"
#include "D3D.h"
#include "UPackage.h"

#include <cstdio>
#include <string>

StaticMesh::~StaticMesh()
{
	if (kDOPNodes) {
		delete[] kDOPNodes;
		kDOPNodes = nullptr;
	}

	if (kDOPTriangles) {
		delete[] kDOPTriangles;
		kDOPTriangles = nullptr;
	}

	if (strings) {
		delete[] strings;
		strings = nullptr;
	}

	if (lodInfo) {
		delete[] lodInfo;
		lodInfo = nullptr;
	}
}

void StaticMesh::Render(bool defaultVBuffer) {
	if (!lodInfoCount) {
		return;
	}

	lodInfo[0].Render(defaultVBuffer);
}

void StaticMesh::ReadData(FIStream& data)
{
	if (isInit) {
		return;
	}

	UObject::ReadData(data);

	data._pos = rawDataOffset;
	FIStream* stream = &data;

	sourceFile = FString(stream);
	bounds = FBoxSphereBounds(stream);
	bodySetup = UObject::ReadFromStream(stream);

	int32 temp = data.ReadInt32();
	if (temp != sizeof(kDOPNode)) {
		throw std::exception("Error KDOPNode has wrong size");
		return;
	}

	kDOPNodeCount = data.ReadInt32();
	if (kDOPNodeCount) {
		kDOPNodes = new kDOPNode[kDOPNodeCount];
		data.Read((uint8*)kDOPNodes, kDOPNodeCount * sizeof(kDOPNode));
	}

	temp = data.ReadInt32();
	if (temp != sizeof(_int64)) {
		throw std::exception("Error KDOPNode face has wrong size");
		return;
	}

	kDOPTriangleCount = data.ReadInt32();
	if (kDOPTriangleCount) {
		kDOPTriangles = new _int64[kDOPTriangleCount];
		data.Read((uint8*)kDOPTriangles, kDOPTriangleCount * sizeof(_int64));
	}

	version = data.ReadInt32();
	stringsCount = data.ReadInt32();
	if (stringsCount) {
		strings = new FString[stringsCount];
		for (int32 i = 0; i < stringsCount; i++) {
			strings[i] = FString(stream);
		}
	}
	lodInfoCount = data.ReadInt32();
	if (lodInfoCount) {
		lodInfo = new FStaticLodInfo[lodInfoCount];
		for (int32 i = 0; i < lodInfoCount; i++)
		{
			ReadFStaticLodInfo(stream, &lodInfo[i]);
		}
	}
	temp = stream->ReadInt32();
	if (temp != lodInfoCount) {
		throw std::exception("lod missmatch");
		return;
	}

	thumbnailAngle = FRotator(stream);
	thumbNailDistance = stream->ReadInt32();
	physMeshScale3D = FArray<FVector3>(stream);
	unk = stream->ReadInt32();

	if (exportObject->originalOffset + exportObject->serialSize != stream->_pos) {
		throw std::exception("found unexpected data at end of StaticMesh");
	}

	Init();
}

void StaticMesh::Init()
{
	for (int32 i = 0; i < lodInfoCount; i++) {
		lodInfo[i].Init();
	}

	package->staticMeshees.push_back(this);

	isInit = TRUE;
}

void StaticMesh::ExportObj(std::ofstream& file) {
	if (!lodInfoCount || !lodInfo) {
		return;
	}

	FStaticLodInfo* lod = lodInfo + 0;
	FStaticMeshSection* section = lod->sections + 0;

	uint32 indexCount = lod->indexContainer->elementCount;
	uint32 faceCount = indexCount / 3;
	uint32 vertexCount = lod->numVertices;
	D3DVertex* vertices = lod->vertices;
	uint16* lodIndices = (uint16*)lod->indexContainer->rawData;
	//uint32* _indices = new uint32[lod->indexContainer->elementCount];
	//uint16 firstIndex = 0;
	//
	//for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
	//{
	//	_indices[faceIndex * 3 + 0] = lodIndices[firstIndex + ((faceIndex * 3) + 0)];
	//	_indices[faceIndex * 3 + 1] = lodIndices[firstIndex + ((faceIndex * 3) + 1)];
	//	_indices[faceIndex * 3 + 2] = lodIndices[firstIndex + ((faceIndex * 3) + 2)];
	//}

	char lineBuffer[256];

	//vertices
	for (size_t i = 0; i < vertexCount; i++)
	{
		D3DVertex* v = vertices + i;
		snprintf(lineBuffer, 256, "v %f %f %f\n", v->position.x, v->position.y, v->position.z);
		size_t lineSize = strnlen_s(lineBuffer, 256);
		file.write(lineBuffer, lineSize);
	}

	//uv coords
	for (size_t i = 0; i < vertexCount; i++)
	{
		D3DVertex* v = vertices + i;

		snprintf(lineBuffer, 256, "vt %f %f\n", v->uv.x, v->uv.y);
		size_t lineSize = strnlen_s(lineBuffer, 256);
		file.write(lineBuffer, lineSize);
	}

	//normals
	for (size_t i = 0; i < vertexCount; i++)
	{
		D3DVertex* v = vertices + i;

		snprintf(lineBuffer, 256, "vn %f %f %f\n", v->normal.x, v->normal.y, v->normal.z);
		size_t lineSize = strnlen_s(lineBuffer, 256);
		file.write(lineBuffer, lineSize);
	}

	const char mtl[] = "usemtl default\n";
	file.write(mtl, sizeof(mtl) - 1);

	//faces
	for (size_t faceIndex = 0; faceIndex < faceCount; faceIndex++)
	{
		snprintf(lineBuffer, 256, "f %d %d %d\n",
			lodIndices[faceIndex * 3 + 2] + 1,
			lodIndices[faceIndex * 3 + 1] + 1,
			lodIndices[faceIndex * 3 + 0] + 1);

		size_t lineSize = strnlen_s(lineBuffer, 256);
		file.write(lineBuffer, lineSize);
	}


}
