#ifndef PACKAGE_MANAGER_H
#define PACKAGE_MANAGER_H

#include "UPackage.h"
#include "typeDefs.h"

#include "Actor.h"
#include "Level.h"

#include <string>
#include <vector>

#include <D3D10_1.h>
#include <D3DX10math.h>

struct ActorsExportEntry {
	struct StaticMeshActor* actor = nullptr;
	struct Terrain* terrain = nullptr;
	struct TerrainComponent* component = nullptr;
	uint32 vOffset = 0;
	uint32 vCount = 0;

	ActorsExportEntry(){}
	ActorsExportEntry(struct StaticMeshActor* a);
	ActorsExportEntry(struct Terrain* a);
	ActorsExportEntry(struct TerrainComponent* a);

	ActorsExportEntry(const ActorsExportEntry& other) {
		actor = other.actor;
		terrain = other.terrain;
		component = other.component;
		vOffset = other.vOffset;
		vCount = other.vCount;
	}

	void operator=(const ActorsExportEntry& other) {
		actor = other.actor;
		terrain = other.terrain;
		component = other.component;
		vOffset = other.vOffset;
		vCount = other.vCount;
	}

	void WriteVertices(std::ofstream& file, uint32 offset);
	void WriteNormals(std::ofstream& file);
	void WriteTexCoods(std::ofstream& file);
	void WriteIndices(std::ofstream& file);
	void AppendVerts(std::vector<D3DXVECTOR3>& verts, uint32 offset);
	void AppendNormals(std::vector<D3DXVECTOR3>& verts);
	void AppendIndices(std::vector<int>& verts);
};

static int32 topIndex;
static char  packagesRootPath[MAX_PATH];

void SetPackagesRootPath(const char* path);
const char* GetPackagesRootPath();
void InitPackageManager();
UPackage* GetActivePackage();
UPackage* LoadPackage(const char* packageFilePath);
int32 GetLoadedPackagesCount();
int32 ScanForPackages(std::vector<std::string>& outPackages);
int32 ReleasePackage();
void PackageManagerRender();
void ClearStaticMesh();
void ClearStaticMeshActor();
void SetStaticMesh(int32 index);
void SetStaticActorMesh(int32 index);
int32 GetCurrentStaticMeshIndex();
int32 GetCurrentStaticActorMeshIndex();
void ClearTerrain();
void SetTerrain(int32 index);
int32 GetCurrentTerrainIndex();

int32 ExportForSkylake(int version, char* name, char* exportpath,
	int continentId, int worldMapWorldId, int worldMapGuardId, int worldMapSectionId, int clusterCellSize);

#endif
