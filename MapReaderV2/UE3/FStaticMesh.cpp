#include "FStaticMesh.h"
#include "UObject.h"
#include "FLightMap.h"

#include "D3D.h"
#include <fstream>

FVert::FVert()
{
}

FVert::FVert(FIStream * stream)
	:FReadable(stream)
{
	pVertex = stream->ReadInt32();
	iSide = stream->ReadInt32();
	shadowTexCoord = FVector2(stream);
	backfaceShadowTexCoord = FVector2(stream);
}

FPositionVertexData::FPositionVertexData()
{
	vertices = nullptr;
}

FPositionVertexData::FPositionVertexData(FIStream * stream)
	:FReadable(stream)
{
	stride = stream->ReadInt32();
	count = stream->ReadInt32();

	int32 temp = stream->ReadInt32();
	if (temp != stride) {
		throw "FPositionVertexData::Incorrect stride ";
	}

	temp = stream->ReadInt32();
	if (temp != count) {
		throw "FPositionVertexData::Incorrect count ";
	}

	if (stride != sizeof(D3DXVECTOR3)) {
		throw "FPositionVertexData::Unknown stride";

	}

	if (stride && count) {
		vertices = new D3DXVECTOR3[count];
		stream->Read((uint8*)vertices, sizeof(D3DXVECTOR3)* count);
	}
}

FPositionVertexData::~FPositionVertexData()
{
	if (vertices) {
		delete[] vertices;
		vertices = nullptr;
	}
}

FUVVertexData::FUVVertexData()
{
}

FUVVertexData::FUVVertexData(FIStream * stream)
	:FReadable(stream)
{
	numUVStets = stream->ReadInt32();
	stride = stream->ReadInt32();
	numUVs = stream->ReadInt32();
	bUseFullUVs = stream->ReadInt32();

	size = 0;
	int32 temp = stream->ReadInt32();
	if (temp != stride) {
		throw "Error, incorrect textstride";
	}

	temp = stream->ReadInt32();
	if (temp != numUVs) {
		throw "Error, incorrect textcount";
	}

	if (bUseFullUVs) {
		switch (numUVStets)
		{
		case 1: {
			size = sizeof(FUVFloat1);
		}break;

		case 2: {
			size = sizeof(FUVFloat2);
		}break;

		case 3: {
			size = sizeof(FUVFloat3);
		}break;

		case 4: {
			size = sizeof(FUVFloat4);
		}break;

		default:
			break;

		}
	}
	else {
		switch (numUVStets)
		{
		case 1: {
			size = sizeof(FUVHalf1);
		}break;

		case 2: {
			size = sizeof(FUVHalf2);
		}break;

		case 3: {
			size = sizeof(FUVHalf3);
		}break;

		case 4: {
			size = sizeof(FUVHalf4);
		}break;

		default:
			break;

		}
	}

	if (!size) {
		throw "Invalid UVs count";
	}

	if (numUVs) {
		data = new uint8[size * numUVs];
		stream->Read(data, size* numUVs);
	}
}

FUVVertexData::~FUVVertexData()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}
}

FStaticMeshSection::FStaticMeshSection()
{
	indexBuffer = nullptr;
	indices = nullptr;
}

FStaticMeshSection::FStaticMeshSection(FIStream * stream)
	:FReadable(stream)
{
	material = stream->ReadInt32();
	enableCollision = stream->ReadInt32();
	oldEnableCollision = stream->ReadInt32();
	bEnableShadowCasting = stream->ReadInt32();
	firstIndex = stream->ReadInt32();
	faceCount = stream->ReadInt32();
	minVertexIndex = stream->ReadInt32();
	maxVertexIndex = stream->ReadInt32();
	materialIndex = stream->ReadInt32();
	fragmentCount = stream->ReadInt32();

	if (fragmentCount) {
		fragments.push_back(FFragmentRange(stream));
	}

	indexBuffer = nullptr;
	indices = nullptr;
}

FStaticMeshSection::~FStaticMeshSection()
{
	if (indexBuffer) {
		indexBuffer->Release();
		indexBuffer = nullptr;
	}

	if (indices) {
		delete[]indices;
		indices = nullptr;
	}
}

void FStaticMeshSection::Init(FMultiSizeIndexContainer * indexContainer)
{
	if (faceCount == 0) {

		return;
	}

	indexCount = faceCount * 3;

	if (indexContainer->elementSize == sizeof(uint16)) {

		indices = (uint8*)malloc(sizeof(uint16) * indexCount);
		indexSize = sizeof(uint16);
		uint16 * lodIndices = (uint16 *)indexContainer->rawData;
		uint16 * _indices = (uint16*)indices;

		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			_indices[faceIndex * 3 + 0] = lodIndices[firstIndex + ((faceIndex * 3) + 0)];
			_indices[faceIndex * 3 + 1] = lodIndices[firstIndex + ((faceIndex * 3) + 1)];
			_indices[faceIndex * 3 + 2] = lodIndices[firstIndex + ((faceIndex * 3) + 2)];
		}

		dxgiFormat = DXGI_FORMAT_R16_UINT;
	}
	else if (indexContainer->elementSize == sizeof(uint32)) {
		indexSize = sizeof(uint32);
		indices = (uint8*)malloc(sizeof(uint32) * indexCount);
		uint32 * lodIndices = (uint32*)indexContainer->rawData;
		uint32 * _indices = (uint32*)indices;

		for (uint32 faceIndex = 0; faceIndex < faceCount; faceIndex++)
		{
			_indices[faceIndex * 3 + 0] = lodIndices[firstIndex + ((faceIndex * 3) + 0)];
			_indices[faceIndex * 3 + 1] = lodIndices[firstIndex + ((faceIndex * 3) + 1)];
			_indices[faceIndex * 3 + 2] = lodIndices[firstIndex + ((faceIndex * 3) + 2)];
		}

		dxgiFormat = DXGI_FORMAT_R32_UINT;
	}
	else {
		throw  std::exception("Unknown index size");
	}

	auto* d3dDevice = GetD3DDevice();

	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = indexSize* indexCount;

	D3D11_SUBRESOURCE_DATA bData;
	ZeroMemory(&bData, sizeof(bData));
	bData.pSysMem = indices;

	HRESULT result = d3dDevice->CreateBuffer(&bDesc, &bData, &indexBuffer);
	if (FAILED(result)) {
		throw std::exception("Failed to create index buffer");
	}
}

void FStaticMeshSection::Render(ID3D11DeviceContext* context)
{
	if (!indexCount) {
		return;
	}

	if (!fragmentCount || !fragments[0].numPrimitives) {
		return;
	}

	context->IASetIndexBuffer(indexBuffer, dxgiFormat, 0);
	context->DrawIndexed(indexCount, 0, 0);
}

FStaticColorData::FStaticColorData()
{
}

FStaticColorData::FStaticColorData(FIStream * stream)
	:FReadable(stream)
{
	stride = stream->ReadInt32();
	count = stream->ReadInt32();

	if (stride != 4) {
		throw "FStaticColorData::Incorrect colorstride";
	}

	int32 temp = stream->ReadInt32();
	if (temp != stride) {
		throw "FStaticColorData::Colorstride missamtch";
	}

	temp = stream->ReadInt32();
	if (temp != count) {
		throw "FStaticColorData::Color count missamtch";
	}

	if (stride && count) {
		data = new uint8[stride * count];
		stream->Read(data, stride* count);
	}
}

FStaticColorData::~FStaticColorData()
{
	if (data) {
		delete[] data;
		data = nullptr;
	}
}

FStaticMeshComponentLODInfo::FStaticMeshComponentLODInfo()
{
}

FStaticMeshComponentLODInfo::FStaticMeshComponentLODInfo(FIStream * stream)
	:FReadable(stream)
{
	shadowMaps = FArray<UObject*>(stream);
	shadowVertexBuffers = FArray<UObject*>(stream);
	lightMap = FLightMap::FromStream(stream);
	unkShadowMap = UObject::ReadFromStream(stream);
}

FStaticMeshComponentLODInfo::~FStaticMeshComponentLODInfo()
{
	if (lightMap) {
		delete lightMap;
		lightMap = nullptr;
	}
}

FStaticLodInfo::FStaticLodInfo()
	:FReadable()
{
	vertices = nullptr;
	possitionBuffer = nullptr;
	vertexBuffer = nullptr;
	legacyEdges = nullptr;
	indexContainer = nullptr;
	textureBuffer = nullptr;
}

void ReadFStaticLodInfo(FIStream * stream, FStaticLodInfo* lod) {
	int32 bulkDataCount, bulkDataSize;
	stream->ReadInt32();
	bulkDataCount = stream->ReadInt32();
	bulkDataSize = stream->ReadInt32();
	stream->ReadInt32();

	if (bulkDataCount && bulkDataSize) {
		throw std::exception("Unk data found!!");
		return;
	}
	lod->sectionsCount = stream->ReadInt32();
	if (lod->sectionsCount) {
		lod->sections = new FStaticMeshSection[lod->sectionsCount];
		for (int32 i = 0; i < lod->sectionsCount; i++)
		{
			lod->sections[i] = FStaticMeshSection(stream);
		}
	}

	lod->possitionBuffer = new FPositionVertexData(stream);
	lod->textureBuffer = new FUVVertexData(stream);
	lod->colorBuffer = FStaticColorData(stream);

	lod->numVertices = stream->ReadInt32();

	lod->indexContainer = new FMultiSizeIndexContainer(stream);
	lod->wireframeIndexBuffer = FMultiSizeIndexContainer(stream);

	int32 test = stream->ReadInt32();
	if (test != sizeof(FEdge)) {
		throw std::exception("Incorrect FEdge size");
		return;
	}

	lod->legacyEdgeCount = stream->ReadInt32();
	if (lod->legacyEdgeCount) {
		lod->legacyEdges = new FEdge[lod->legacyEdgeCount];
		stream->Read((uint8*)lod->legacyEdges, lod->legacyEdgeCount * sizeof(FEdge));
	}

	test = stream->ReadInt32();
	if (test) {
		throw std::exception("unk ending");
	}
}

FStaticLodInfo::FStaticLodInfo(FIStream * stream)
	:FReadable(stream)
{
	vertices = nullptr;
	possitionBuffer = nullptr;
	vertexBuffer = nullptr;
	textureBuffer = nullptr;
	indexContainer = nullptr;
	legacyEdges = nullptr;
}

FStaticLodInfo::~FStaticLodInfo()
{
	if (vertexBuffer) {
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}

	if (legacyEdges) {
		delete[] legacyEdges;
		legacyEdges = nullptr;
	}

	if (vertices) {
		delete[] vertices;
		vertices = nullptr;
	}

	if (possitionBuffer) {
		delete possitionBuffer;
		possitionBuffer = nullptr;
	}

	if (textureBuffer) {
		delete textureBuffer;
		textureBuffer = nullptr;
	}

	if (indexContainer) {
		delete indexContainer;
		indexContainer = nullptr;
	}

}

void FStaticLodInfo::Render(bool defaultVBuffer)
{
	auto* d3dDeviceContext = GetD3DDeviceContext();

	if (defaultVBuffer) {
		UINT offset = 0;
		UINT stride = sizeof(D3DVertex);
		d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3dDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	}

	for (int32 i = 0; i < sectionsCount; i++) {
		sections[i].Render(d3dDeviceContext);
	}
}

void FStaticLodInfo::Init() {
	if (!numVertices) {
		return;
	}

	float scale = 0.1f;
	HRESULT result;

	vertices = new D3DVertex[numVertices];
	ZeroMemory(vertices, sizeof(D3DVertex)* numVertices);
	D3DVertex* v = vertices;
	D3DXVECTOR3 * pos = possitionBuffer->vertices;

	for (uint32 i = 0; i < numVertices; i++, pos++, v++) {
		//v->position.x = pos->y;	//-pos->y;	  pos->x
		//v->position.y = pos->z;	// pos->z;	  pos->y
		//v->position.z = pos->x;	// pos->x;	  pos->z
		v->position.x = pos->x;
		v->position.y = pos->y;
		v->position.z = pos->z;

		if (textureBuffer->bUseFullUVs) {
			if (textureBuffer->numUVStets == 2) {
				FUVFloat2 * cuvs = (FUVFloat2 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs[0].uv[0];
				v->uv.y = cuvs[i].UVs[0].uv[1];
			}
			else if (textureBuffer->numUVStets == 3) {
				FUVFloat3 * cuvs = (FUVFloat3 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs[0].uv[0];
				v->uv.y = cuvs[i].UVs[0].uv[1];
			}
			else if (textureBuffer->numUVStets == 4) {
				FUVFloat4 * cuvs = (FUVFloat4 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs[0].uv[0];
				v->uv.y = cuvs[i].UVs[0].uv[1];
			}
			else {
				if (textureBuffer->numUVStets != 1) {
					throw std::exception("unexptected uv count");
				}

				FUVFloat1 * cuvs = (FUVFloat1 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs.uv[0];
				v->uv.y = cuvs[i].UVs.uv[1];
			}
		}
		else {
			if (textureBuffer->numUVStets == 2) {
				FUVHalf2* cuvs = (FUVHalf2 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs[0].uv[0];
				v->uv.y = cuvs[i].UVs[0].uv[1];
			}
			else if (textureBuffer->numUVStets == 3) {
				FUVHalf3* cuvs = (FUVHalf3 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs[0].uv[0];
				v->uv.y = cuvs[i].UVs[0].uv[1];
			}
			else if (textureBuffer->numUVStets == 4) {
				FUVHalf3* cuvs = (FUVHalf3 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs[0].uv[0];
				v->uv.y = cuvs[i].UVs[0].uv[1];
			}
			else {
				if (textureBuffer->numUVStets != 1) {
					throw std::exception("unexptected uv count");
				}

				FUVHalf1 * cuvs = (FUVHalf1 *)textureBuffer->data;
				_UnpackNormal(cuvs[i].normal[1].normal, &v->normal);

				v->uv.x = cuvs[i].UVs.uv[0];
				v->uv.y = cuvs[i].UVs.uv[1];
			}
		}
	}

	delete possitionBuffer;
	possitionBuffer = nullptr;

	for (int32 i = 0; i < sectionsCount; i++) {
		sections[i].Init(indexContainer);
	}

	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(D3DVertex) * numVertices;
	bDesc.StructureByteStride = 0;
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA bData;
	bData.pSysMem = vertices;
	bData.SysMemPitch = 0;
	bData.SysMemSlicePitch = 0;

	auto* d3dDevice = GetD3DDevice();

	result = d3dDevice->CreateBuffer(&bDesc, &bData, &vertexBuffer);
	if (FAILED(result)) {
		throw std::exception("Failed to create vertex buffer");
	}
}
