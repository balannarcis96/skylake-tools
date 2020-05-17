#include "UPackage.h"
#include "FArray.h"
#include "CustomFArrays.h"
#include "UObject.h"

//#define ZLIB_WINAPI
//#include <zlib.h>

#include <windows.h>
#include <math.h>
#include <algorithm>
#include <fstream>

#include "TerrainComponent.h"
#include "Terrain.h"
#include "Level.h"

#define LZO_CHUNK_SIZE 0x20000 //131072
#define ZLIB_CHUNK_SIZE LZO_CHUNK_SIZE
#define LZO_MAGIC 0x9E2A83C1
#define ZLIB_MAGIC LZO_MAGIC

#define ENABLE_ZLIB_GZIP 32
typedef struct { uint32 compressedOffset; uint32 compressedSize; uint32 decompressedSize; uint32 decompressedOffset; } CompressedBlock;

extern "C" void *zcalloc(int opaque, int items, int size)
{
	return malloc(items * size);
}

extern "C" void zcfree(int opaque, void *ptr)
{
	free(ptr);
}

//int32 decompressZlib(uint8 *in, uint32 inLen, uint8 * out, uint32 outLen) {
//	uint8* origin = in;
//
//	int32 magic = *(int32*)in; in += 4;
//	if (magic != ZLIB_MAGIC) {
//		return -2;
//	}
//
//	int32 result;
//	/*z_stream infStream;
//	memset(&infStream, 0, sizeof(infStream));
//	result = inflateInit(&infStream, -1);
//	if (result != Z_OK) {
//		throw "Failed to initInfalte [zlib]";
//		return false;
//	}*/
//
//
//	uint32 blockSize = *(uint32*)in; in += 4;
//	uint32 compressedSize = *(uint32*)in; in += 4;
//	uint32 decompressedSize = *(uint32*)in; in += 4;
//
//	if (decompressedSize > 1024 * 1024 * 256) {
//		throw std::exception("TO BIG!!! LARGER THAN 256MB!!!");
//	}
//
//	uint32 totalBlocks = ceilf((float)decompressedSize / (float)blockSize);
//	CompressedBlock *blocks = new CompressedBlock[totalBlocks];
//
//	uint32 compressedOffset = 0;
//	uint32 decompressedOffset = 0;
//
//	uint8 * outputBuffer = new uint8[decompressedSize];
//
//
//	for (uint32 i = 0; i < totalBlocks; i++) {
//		blocks[i].compressedSize = *(uint32*)in; in += 4;;
//		blocks[i].decompressedSize = *(uint32*)in; in += 4;
//		blocks[i].compressedOffset = 16 + compressedOffset + (totalBlocks * 8);
//		blocks[i].decompressedOffset = decompressedOffset;
//
//		compressedOffset += blocks[i].compressedSize;
//		decompressedOffset += blocks[i].decompressedSize;
//
//		uint32 zmagic = *(uint32*)origin + blocks[i].compressedSize;
//
//
//		uint32 decompressedLen = blocks[i].decompressedSize;
//		result = uncompress(
//			out + blocks[i].decompressedOffset,
//			&decompressedLen,
//			origin + blocks[i].compressedOffset,
//			blocks[i].compressedSize);
//		if (result != Z_OK) {
//			return false;
//		}
//
//		/*	infStream.avail_in = blocks[i].compressedSize;
//			infStream.next_in = origin + blocks[i].compressedOffset;
//			infStream.avail_out = blocks[i].decompressedSize;
//			infStream.next_out = outputBuffer + blocks[i].decompressedOffset;
//
//			result = inflate(&infStream, Z_NO_FLUSH);
//			if (result != Z_OK) {
//				return false;
//			}*/
//
//	}
//
//	if (compressedSize != compressedOffset || decompressedSize != decompressedOffset) {
//		throw std::exception("failed to prepare for decompression [Zlib]");
//	}
//
//	//inflateEnd(&infStream);
//
//	/*for (uint32 i = 0; i < totalBlocks; i++) {
//
//
//
//		infStream.avail_in = blocks[i].compressedSize;
//		infStream.next_in = origin + blocks[i].compressedOffset;
//		infStream.avail_out = blocks[i].decompressedSize;
//		infStream.next_out = out + blocks[i].decomressedOffset;
//
//
//		result = inflateInit2(&infStream, -MAX_WBITS);
//		if (result != Z_OK) {
//			throw "Failed to initInfalte [zlib]";
//			return false;
//		}
//
//		result = inflate(&infStream, Z_NO_FLUSH);
//		if (result != Z_OK) {
//			return false;
//		}
//
//		inflateEnd(&infStream);
//	}*/
//
//	return 0;
//}

bool decompressPackage(UPackage * package)
{
	if (!package) {
		return false;
	}

	//if (package->compression == COMPRESSION_NONE) {
	//	return true;
	//}

	//u_int64 temp = 0;
	//int32 result;
	//int32 totalCompressedSize = 0;
	//int32 totalDecompressedSize = 0;
	//int32 startOffset = INT32_MAX;
	//int32 startOffset2 = INT32_MAX;

	//for (size_t i = 0; i < package->compressedChunksCount; i++)
	//{
	//	totalCompressedSize += package->compressedChunks[i].compressedSize;
	//	totalDecompressedSize += package->compressedChunks[i].decompressedSize;
	//	startOffset = min(startOffset, package->compressedChunks[i].decompressedOffset);
	//	startOffset2 = min(startOffset2, package->compressedChunks[i].compressedOffset);
	//}
	//FIStream & s = package->stream;

	//FIStream decompressed = FIStream(totalDecompressedSize + startOffset);
	//decompressed.Write(s._raw, startOffset);

	//int32 nFlags = package->flags;
	//nFlags &= ~PKG_StoreCompressed;
	//int32 flagsOffset = 12 + package->folderName.StrLen() + 4 + 1;
	//temp = decompressed._pos;
	//decompressed._pos = flagsOffset;
	//decompressed.WriteInt32(nFlags);
	//decompressed._pos = temp;

	//for (size_t i = 0; i < package->compressedChunksCount; i++)
	//{
	//	s._pos = package->compressedChunks[i].compressedOffset;
	//	decompressed._pos = package->compressedChunks[i].decompressedOffset;

	//	if (package->compression == COMPRESSION_LZO) {
	//		/*	lzo1x_decompress(s.GetPtr(), package->compressedChunks[i].compressedSize,
	//				decompressed.GetPtr(), (uint32*)&temp, NULL);*/
	//		MessageBox(NULL, "ERROR", "PACKAGE COMPRESSED WITH LZO [TODO]", MB_OK);
	//	}
	//	else if (package->compression == COMPRESSION_ZLIB) {
	//		result = decompressZlib(
	//			s._raw + package->compressedChunks[i].compressedOffset,
	//			package->compressedChunks[i].compressedSize,
	//			decompressed._raw + package->compressedChunks[i].decompressedOffset,
	//			package->compressedChunks[i].decompressedSize);
	//	}
	//	else {
	//		throw "Unknow compression";
	//	}
	//}

	//delete[] package->compressedChunks;
	//package->compressedChunks = nullptr;
	//package->compressedChunksCount = 0;

	//std::ofstream test = std::ofstream("E:\\test_package.bin", std::iostream::binary);
	//test.write((const char*)decompressed._raw, decompressed._size);
	//test.close();

	return true;
}

UPackage::UPackage()
{
	exports = nullptr;
	imports = nullptr;
	depends = nullptr;
	compressedChunks = nullptr;
	generations = nullptr;
}

UPackage::UPackage(const char * packageFilePath) {
	exports = nullptr;
	imports = nullptr;
	depends = nullptr;
	compressedChunks = nullptr;
	generations = nullptr;

	if (!stream.load_from_file(packageFilePath)) {
		bFailedToLoad = 1;
		return;
	}

	std::string path = std::string((char*)packageFilePath);
	PackageName = path.substr(path.find_last_of("/\\") + 1);
	PackageName.erase(PackageName.begin() + PackageName.find_last_of("."), PackageName.end());
	stream._package = this;

	readFromStream();
}

UPackage::~UPackage()
{
	topExports.clear();
	if (exports) {
		delete[]exports;
		exports = nullptr;
	}

	topImports.clear();
	if (imports) {
		delete[] imports;
		imports = nullptr;
	}

	if (depends) {
		delete[] depends;
		depends = nullptr;
	}

	if (compressedChunks) {
		delete[]compressedChunks;
		compressedChunks = nullptr;
	}

	if (generations) {
		delete generations;
		generations = nullptr;
	}
}

const char * UPackage::Name() const
{
	return PackageName.c_str();
}

const char * UPackage::nameForIndex(int32 index) const
{
	if (index < 0 || index >= namesCount)
	{
		throw new std::exception("bad name index", 23);
	}

	return names[index].string.c_str();
}

UObject * UPackage::uObjectForIndex(int32 index) const
{
	if (index < 0 && (-index - 1) < importsCount) {
		return imports[-index - 1].uObject;
	}
	else {
		if (index - 1 >= 0 && index <= exportsCount)
			return exports[index - 1].uObject;
	}

	return nullptr;
}

FObject * UPackage::fObjectForIndex(int32 index)
{
	if (index < 0) {
		return &imports[-index - 1];
	}
	else {
		if (index - 1 >= 0)
			return &exports[index - 1];
	}

	return nullptr;
}

void UPackage::Init()
{
	readTables();
	buildObjectTrees();

	extern UPackage* GetPackage(const char* name);
	for (size_t i = 0; i < importsCount; i++)
	{
		if (!strcmp(imports[i].Class(), "Package")) {
			if (!strcmp(imports[i].Name(), this->Name())) {
				dependentPackages.push_back(this);
				continue;
			}

			UPackage * package = GetPackage(imports[i].Name());
			if (!package) {
				continue;
			}

			dependentPackages.push_back(package);
		}
	}

	staticMeshesCount = 0;
	//for (uint32 i = 0; i < exportsCount; i++) {
	//	if (exports[i].uObject && !strcmp(exports[i].uObject->Class(), "StaticMesh")) {
	//		exports[i].uObject->ReadObject(false);
	//		staticMeshees.push_back(exports[i].uObject);
	//		staticMeshesCount++;
	//	}
	//}
	//
	//for (uint32 i = 0; i < exportsCount; i++) {
	//	if (exports[i].uObject && !strcmp(exports[i].uObject->Class(), "StaticMeshActor")) {
	//		exports[i].uObject->ReadObject(false);
	//		actorsMeshees.push_back((StaticMeshActor*)exports[i].uObject);
	//	}
	//}

	for (uint32 i = 0; i < exportsCount; i++) {
		if (exports[i].uObject && !strcmp(exports[i].uObject->Class(), "TerrainComponent")) {
			exports[i].uObject->ReadObject(false);
			terrainComponents.push_back((TerrainComponent*)exports[i].uObject);
		}
	}

	for (uint32 i = 0; i < exportsCount; i++) {
		if (exports[i].uObject && !strcmp(exports[i].uObject->Class(), "Terrain")) {
			exports[i].uObject->ReadObject(false);
			((class Terrain*)exports[i].uObject)->Init();
			terrainTiles.push_back((class Terrain*)exports[i].uObject);
		}
	}

	for (uint32 i = 0; i < exportsCount; i++) {
		if (exports[i].uObject && !strcmp(exports[i].uObject->Class(), "Level")) {
			exports[i].uObject->ReadObject(false);
			((Level*)exports[i].uObject)->Init();
		}
	}

	////resolved imports
	
}

void UPackage::Export(FIStream &data)
{
	if (!levels.size()) {
		data.WriteUInt16(0);
		return;
	}

	data.WriteUInt16(1);
	Level * level = levels[0];
	level->Export(data);
}

void UPackage::readFromStream() {
	stream._pos = 0;

	int32 magic = stream.ReadInt32();
	if (magic != PACKAGE_MAGIC) {
		bFailedToLoad = 2;
		return;
	}

	fileVersion = stream.ReadInt16();
	licenseVersion = stream.ReadInt16();

	//TEST FILE VERSION AND LICENCE VERSION HERE

	headerSize = stream.ReadInt32();
	folderName = FString(&stream);
	flags = (EPackageFlags)stream.ReadInt32();
	namesCount = stream.ReadInt32();
	namesOffset = stream.ReadInt32();
	exportsCount = stream.ReadInt32();
	exportsOffset = stream.ReadInt32();
	importsCount = stream.ReadInt32();
	importsOffset = stream.ReadInt32();
	dependsOffset = stream.ReadInt32();
	guid = FGUID(&stream);
	generations = new FArray<FGeneration>(&stream);
	if (generations->count == 0) {
		throw std::exception("0 generations on package");
	}

	engineVersion = stream.ReadInt32();
	coockedContentVersion = stream.ReadInt32();

	compression = stream.ReadInt32();
	compressedChunksCount = stream.ReadInt32();
	if (compressedChunksCount) {
		compressedChunks = new FCompressedChunk[compressedChunksCount];
		stream.Read((uint8*)compressedChunks, compressedChunksCount * sizeof(FCompressedChunk));
	}

	packageSource = stream.ReadInt32();

	//if (!self.additionalPackagesToCook) // TODO: this check should not exists in the first place. What kind of value is this after decompression?
	//	self.additionalPackagesToCook = [FArray readFrom : s type : [FString class]];

	if (flags & PKG_StoreCompressed && compressedChunksCount) {
		this->bFailedToLoad = 50;
		return;
		if (!decompressPackage(this)) {
			throw "Failed to decompress package";
		}
	}

	if (licenseVersion == 14 && fileVersion == 610 && coockedContentVersion) {
		namesCount -= namesOffset;
	}
}

void __loadFObjectImport(FIStream * stream, FObjectImport* imp) {
	imp->package = stream->_package;
	imp->fObjectType = FT_FObjectImport;

	imp->ClassPackage = new FName(stream);
	imp->ClassName = new FName(stream);
	imp->ParentIndex = stream->ReadInt32();
	imp->nameIndex = stream->ReadInt64();

}

void __loadFObjectExport(FIStream * stream, FObjectExport* exp) {
	exp->package = stream->_package;
	exp->fObjectType = FT_FObjectExport;

	exp->classObjectIndex = stream->ReadInt32();
	exp->superObjectIndex = stream->ReadInt32();
	exp->parentIndex = stream->ReadInt32();
	exp->nameIndex = stream->ReadInt32();

	//if (exp->nameIndex > exp->package->namesCount || exp->nameIndex < 0) {
	//	throw std::exception("wrong name index");
	//}

	exp->archetypeIndex = stream->ReadInt64();
	exp->objectFlags = (EObjectFlags)stream->ReadInt64();
	exp->serialSize = stream->ReadUInt32();
	if (exp->serialSize > 0)
	{
		exp->serialOffset = stream->ReadUInt32();
		exp->originalOffset = exp->serialOffset;
	}

	exp->exportFlags = (EFExportFlags)stream->ReadInt32();
	exp->generationNetObjectCount = new FIntArray(stream);
	exp->packageGuid = new FGUID(stream);
	exp->packageFlags = (EPackageFlags)stream->ReadInt32();
}

__forceinline void __loadFObjectRef(FIStream * stream, FObjectRef& ref) {
	ref.package = stream->_package;
	ref.value = stream->ReadInt32();
}


void UPackage::readTables()
{
	names = new FNamePair[namesCount];
	stream._pos = namesOffset;
	for (int32 i = 0; i < namesCount; i++)
	{
		names[i] = FNamePair(&stream);
	}

	imports = new FObjectImport[importsCount];
	stream._pos = importsOffset;
	for (int32 i = 0; i < importsCount; i++)
	{
		__loadFObjectImport(&stream, &imports[i]);
	}

	exports = new FObjectExport[exportsCount];
	stream._pos = exportsOffset;
	for (int32 i = 0; i < exportsCount; i++)
	{
		__loadFObjectExport(&stream, &exports[i]);
	}

	stream._pos = dependsOffset;
	depends = new FObjectRef[exportsCount];
	for (int32 i = 0; i < exportsCount; i++)
	{
		__loadFObjectRef(&stream, depends[i]);
	}
}

void UPackage::buildObjectTrees()
{
	for (size_t i = 0; i < importsCount; i++) {
		imports[i].serialize();
	}

	for (size_t i = 0; i < exportsCount; i++) {
		exports[i].serialize();
	}

	for (size_t i = 0; i < importsCount; i++) {
		imports[i].Path();
		if (!imports[i].Parent()) {
			topImports.push_back(&imports[i]);
		}
	}

	for (size_t i = 0; i < exportsCount; i++) {
		exports[i].Path();
		if (!exports[i].Parent()) {
			topExports.push_back(&exports[i]);
		}
	}
}

FGeneration::FGeneration()
{

}

FGeneration::FGeneration(FIStream * stream)
{
	exports = stream->ReadInt32();
	names = stream->ReadInt32();
	imports = stream->ReadInt32();
}
