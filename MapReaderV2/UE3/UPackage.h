#ifndef UPACKAGE_H
#define UPACKAGE_H

#include "typeDefs.h"
#include "FIStream.h"
#include "FGUID.h"
#include "FString.h"
#include "FReadable.h"

#include <vector>
#include <string>

struct UObject;
struct StaticMeshActor;
struct Terrain;
struct Level;
struct TerrainComponent;

struct FCompressedChunk
{
	int32 decompressedOffset;
	int32 decompressedSize;
	int32 compressedOffset;
	int32 compressedSize;
};

struct FGeneration : FReadable {
	int32 exports;
	int32 names;
	int32 imports;

	FGeneration();
	FGeneration(FIStream * stream);
};

struct RootExportObject :FObjectExport {

};

struct RootImportObject :FObjectImport {

};

struct UPackage {
	FIStream stream;
	int32 packageSource;
	FGUID guid;
	EPackageFlags flags;
	int32 compression;
	int16 fileVersion;
	int32 coockedContentVersion;
	int32 engineVersion;
	int16 licenseVersion;
	int32 headerSize;
	FString folderName;
	int32 compressedChunksCount;
	FCompressedChunk * compressedChunks;
	RootImportObject * rootImports;
	RootExportObject * rootExports;
	FArray<FGeneration>* generations;

	std::vector<UPackage*> dependentPackages;
	std::vector<UPackage*> failedToLoadPackages;
	std::vector<UObject*> staticMeshees;
	std::vector<StaticMeshActor*> actorsMeshees;
	std::vector<TerrainComponent*> terrainComponents;
	std::vector<Terrain*> terrainTiles;
	std::vector<Level*> levels;

	int32	namesCount;
	int32	namesOffset;
	int32	exportsCount;
	int32	exportsOffset;
	int32	importsCount;
	int32	importsOffset;
	int32	dependsCount;
	int32	dependsOffset;
	int32	staticMeshesCount;

	FNamePair* names;
	FObjectExport* exports;
	std::vector<FObjectExport*> topExports;
	FObjectImport* imports;
	std::vector<FObjectImport*> topImports;
	FObjectRef* depends;

	int32 commpressionChunkOffset;
	int32 bFailedToLoad = FALSE;
	std::string PackageName;
	std::string PackageNameWithExtension;

	//UI
	bool Visible = TRUE;

	UPackage();
	UPackage(const char* path);
	~UPackage();

	const char * Name() const;
	const char * nameForIndex(int32 index) const;
	UObject * uObjectForIndex(int32 index) const;
	FObject * fObjectForIndex(int32 index);
	void Init();

	void Export(FIStream &);

private:
	void readFromStream();
	void readTables();
	void buildObjectTrees();
};
#endif
