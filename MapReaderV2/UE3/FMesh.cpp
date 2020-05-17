#include "FMesh.h"
#include "FIStream.h"

FPackedNormal::FPackedNormal()
{
	memset(normal, 0, 4);
}

FPackedNormal::FPackedNormal(float x, float y, float z)
{
	normal[0] = (uint8)((normal[0] + 1.0f) * 127.0f);
	normal[1] = (uint8)((normal[1] + 1.0f) * 127.0f);
	normal[2] = (uint8)((normal[2] + 1.0f) * 127.0f);
}

Vector3 FPackedNormal::UnpackNormal()
{
	return Vector3{ (normal[0] / 127.5f) - 1.0f, (normal[1] / 127.5f) - 1.0f, (normal[2] / 127.5f) - 1.0f };
}

void FPackedNormal::operator=(FPackedNormal & norm)
{
	normal[0] = norm.normal[0];
	normal[1] = norm.normal[1];
	normal[2] = norm.normal[2];
	normal[3] = norm.normal[3];
}

FMeshBone::FMeshBone()
{
}

FMeshBone::FMeshBone(FIStream * stream)
	:FReadable(stream)
{
	nameIndex = stream->ReadInt64();
	flags = stream->ReadInt32();
	orientation = FVector4(stream);
	position = FVector3(stream);
	childrenCount = stream->ReadInt32();
	parentIndex = stream->ReadInt32();
	unk = stream->ReadInt32();
}

FMeshSection::FMeshSection()
{
}

FMeshSection::FMeshSection(FIStream * stream)
	:FReadable(stream)
{
	material = stream->ReadInt16();
	chunkIndex = stream->ReadInt16();
	firstIndex = stream->ReadInt32();
	faceCount = stream->ReadUInt16();
}

FMultiSizeIndexContainer::FMultiSizeIndexContainer()
{
	rawData = nullptr;
}

FMultiSizeIndexContainer::FMultiSizeIndexContainer(FIStream * stream)
	:FReadable(stream)
{
	elementSize = stream->ReadInt32();
	elementCount = stream->ReadInt32();

	rawData = nullptr;
	uint32 length = elementCount *elementSize;
	if (length) {
		rawData = new uint8[elementCount * elementSize];
		stream->Read(rawData, elementCount * elementSize);
	}
}

FMultiSizeIndexContainer::~FMultiSizeIndexContainer()
{
	if (rawData) {
		delete[] rawData;
		rawData = nullptr;
	}
}

FSkeletalMeshChunk::FSkeletalMeshChunk()
{
	rigidVertices = nullptr;
	softVertices = nullptr;
	boneMap = nullptr;
}

FSkeletalMeshChunk::FSkeletalMeshChunk(FIStream * stream)
	:FReadable(stream)
{
	firstIndex = stream->ReadInt32();

	rigidVerticsCount = stream->ReadInt32();
	if (rigidVerticsCount > 0) {
		rigidVertices = new FRigidVertex[rigidVerticsCount];
		stream->Read((uint8*)rigidVertices, rigidVerticsCount * sizeof(FRigidVertex));
	}

	softVerticesCount = stream->ReadInt32();
	if (softVerticesCount > 0) {
		softVertices = new FSoftVertex[softVerticesCount];
		stream->Read((uint8*)softVertices, softVerticesCount * sizeof(FSoftVertex));
	}

	boneMapCount = stream->ReadInt32();
	if (boneMapCount > 0) {
		boneMap = new int16[boneMapCount];
		stream->Read((uint8*)boneMap, boneMapCount * sizeof(int16));
	}

	int32 test = stream->ReadInt32();
	if (test != rigidVerticsCount) {
		throw "Error, FSkeletalMeshChunk rigid vertices missmatch";
	}

	test = stream->ReadInt32();
	if (test != softVerticesCount) {
		throw "Error, FSkeletalMeshChunk soft vertices missmatch";
	}

	maxBoneInfluences = stream->ReadInt32();
}

FSkeletalMeshChunk::~FSkeletalMeshChunk()
{
	if (rigidVertices) {
		delete[] rigidVertices;
		rigidVertices = nullptr;
	}

	if (softVertices) {
		delete[] softVertices;
		softVertices = nullptr;
	}

	if (boneMap) {
		delete[] boneMap;
		boneMap = nullptr;
	}
}

FGPUSkinBuffer::FGPUSkinBuffer()
{
	vertices = nullptr;
}

FGPUSkinBuffer::FGPUSkinBuffer(FIStream * stream)
	:FReadable(stream)
{
	bUseFullUV = stream->ReadInt32();
	bUsePackedPosition = stream->ReadInt32();
	elementSize = stream->ReadInt32();
	elementCount = stream->ReadInt32();

	if (elementSize != sizeof(FGPUVer3Half)) {
		throw "Element size of FGPUSkinBuffer missmatch expected 32";
	}

	vertices = nullptr;
	if (elementCount) {
		vertices = new FGPUVer3Half[elementCount];
		stream->Read((uint8*)vertices, elementCount * sizeof(FGPUVer3Half));
	}
}

FGPUSkinBuffer::~FGPUSkinBuffer()
{
	if (vertices) {
		delete[] vertices;
		vertices = nullptr;
	}
}

FLodInfo::FLodInfo()
{
}

FLodInfo::FLodInfo(FIStream * stream)
	:FReadable(stream)
{
	sections = FArray<FMeshSection>(stream);
	indexContainer = FMultiSizeIndexContainer(stream);

	legacyShadowIndicesCount = stream->ReadInt32();
	if (legacyShadowIndicesCount) {
		legacyShadowIndices = new int16[legacyShadowIndicesCount];
		stream->Read((uint8*)legacyShadowIndices, legacyShadowIndicesCount * sizeof(int16));
	}

	activeBoneIndicesCount = stream->ReadInt32();
	if (activeBoneIndicesCount) {
		activeBoneIndices = new uint16[activeBoneIndicesCount];
		stream->Read((uint8*)activeBoneIndices, activeBoneIndicesCount * sizeof(uint16));
	}

	legacyShadowTriangleDoubleSidedCount = stream->ReadInt32();
	if (legacyShadowTriangleDoubleSidedCount) {
		legacyShadowTriangleDoubleSided = new uint8[legacyShadowTriangleDoubleSidedCount];
		stream->Read(legacyShadowTriangleDoubleSided, legacyShadowTriangleDoubleSidedCount);
	}

	chunks = FArray<FSkeletalMeshChunk>(stream);
	size = stream->ReadInt32();
	vertexCount = stream->ReadInt32();

	legacyEdgeCount = stream->ReadInt32();
	if (legacyEdgeCount) {
		legacyEdges = new FEdge[legacyEdgeCount];
		stream->Read((uint8*)legacyEdges, legacyEdgeCount * sizeof(FEdge));
	}

	requiredBonesCount = stream->ReadInt32();
	if (requiredBonesCount) {
		requiredBones = new uint8[requiredBonesCount];
		stream->Read(requiredBones, requiredBonesCount);
	}

	rawPoints = FBulkData(stream);

	vertexBufferGPUSkin = FGPUSkinBuffer(stream);
	extraVertexInfluenceCount = stream->ReadInt32();
}

FLodInfo::~FLodInfo()
{
	if (legacyShadowIndices) {
		delete[] legacyShadowIndices;
		legacyShadowIndices = nullptr;
	}

	if (legacyEdges) {
		delete[] legacyEdges;
		legacyEdges = nullptr;
	}

	if (activeBoneIndices) {
		delete[] activeBoneIndices;
		activeBoneIndices = nullptr;
	}

	if (legacyShadowTriangleDoubleSided) {
		delete[] legacyShadowTriangleDoubleSided;
		legacyShadowTriangleDoubleSided = nullptr;
	}

	if (requiredBones) {
		delete[] requiredBones;
		requiredBones = nullptr;
	}
}

FSoftVertex * FLodInfo::vertices()
{
	FSoftVertex* verts = (FSoftVertex*)calloc(sizeof(FSoftVertex), vertexCount);
	FSoftVertex * v = verts;
	for (size_t chunkIndex = 0; chunkIndex < chunks.count; ++chunkIndex)
	{
		const FSkeletalMeshChunk* chunk = &chunks.data[chunkIndex];

		for (size_t vertexIndex = 0; vertexIndex < chunk->rigidVerticsCount; vertexIndex++, v++)
		{
			FRigidVertex& rVertex = chunk->rigidVertices[vertexIndex];

			memcpy_s(&v->position, sizeof(Vector3), &rVertex.position, sizeof(Vector3));
			memcpy_s(v->normals, sizeof(FPackedNormal) * 3, rVertex.normals, sizeof(FPackedNormal) * 3);
			memcpy_s(&v->uv, sizeof(FMeshFloatUV), &rVertex.uv, sizeof(FMeshFloatUV));

			v->boneIndex = rVertex.boneIndex;
			v->boneWeight = 255; //max
		}

		memcpy_s(v, chunk->softVerticesCount * sizeof(FSoftVertex), chunk->softVertices, chunk->softVerticesCount * sizeof(FSoftVertex));
		v += chunk->softVerticesCount;
	}

	return verts;
}

void _UnpackNormal(uint8 * normal, D3DXVECTOR3 * out_normal)
{
	out_normal->x = ((float)(normal[0] / 127.5f)) - 1.0f;
	out_normal->y = ((float)(normal[1] / 127.5f)) - 1.0f;
	out_normal->z = ((float)(normal[2] / 127.5f)) - 1.0f;
}
