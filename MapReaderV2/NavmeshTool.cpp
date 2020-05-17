#include "App.h"

#include "UE3/TerrainComponent.h"
#include "UE3/Terrain.h"
#include "UE3/StaticMesh.h"

//Utils
inline unsigned int duRGBA(int r, int g, int b, int a)
{
	return ((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16) | ((unsigned int)a << 24);
}

inline unsigned int duRGBAf(float fr, float fg, float fb, float fa)
{
	unsigned char r = (unsigned char)(fr * 255.0f);
	unsigned char g = (unsigned char)(fg * 255.0f);
	unsigned char b = (unsigned char)(fb * 255.0f);
	unsigned char a = (unsigned char)(fa * 255.0f);
	return duRGBA(r, g, b, a);
}

inline unsigned int duMultCol(const unsigned int col, const unsigned int d)
{
	const unsigned int r = col & 0xff;
	const unsigned int g = (col >> 8) & 0xff;
	const unsigned int b = (col >> 16) & 0xff;
	const unsigned int a = (col >> 24) & 0xff;
	return duRGBA((r * d) >> 8, (g * d) >> 8, (b * d) >> 8, a);
}

inline int bit(int a, int b)
{
	return (a & (1 << b)) >> b;
}

unsigned int duIntToCol(int i, int a)
{
	int	r = bit(i, 1) + bit(i, 3) * 2 + 1;
	int	g = bit(i, 2) + bit(i, 4) * 2 + 1;
	int	b = bit(i, 0) + bit(i, 5) * 2 + 1;
	return duRGBA(r * 63, g * 63, b * 63, a);
}

void duIntToCol(int i, float* col)
{
	int	r = bit(i, 0) + bit(i, 3) * 2 + 1;
	int	g = bit(i, 1) + bit(i, 4) * 2 + 1;
	int	b = bit(i, 2) + bit(i, 5) * 2 + 1;
	col[0] = 1 - r * 63.0f / 255.0f;
	col[1] = 1 - g * 63.0f / 255.0f;
	col[2] = 1 - b * 63.0f / 255.0f;
}

void duCalcBoxColors(unsigned int* colors, unsigned int colTop, unsigned int colSide)
{
	if (!colors) return;

	colors[0] = duMultCol(colTop, 250);
	colors[1] = duMultCol(colSide, 140);
	colors[2] = duMultCol(colSide, 165);
	colors[3] = duMultCol(colSide, 217);
	colors[4] = duMultCol(colSide, 165);
	colors[5] = duMultCol(colSide, 217);
}

bool NavmeshTool::Initialize()
{
	Close();

	if (!GGUIManager.RegisterUI<UIElementType::NavmeshTool>(this)) {
		return 1;
	}

	m_ctx = new BuildContext();

	return true;
}

void NavmeshTool::Render(ID3D11DeviceContext* Context, float dt)
{
	if (PreparedMesh.Get()) {
		PreparedMesh->Render(Context);
	}

	for (auto* mesh : DrawMeshes)
	{
		mesh->Render(Context);
	}
}

void NavmeshTool::RenderUI()
{
	auto pTool = GGUIManager.GetElementAs<PackagesTool>();
	auto Package = pTool->GetSelectedPackage();

	ImGui::Begin("Manage Navmesh", &bDisplay);
	if (!Package.Get()) {
		ImGui::TextColored(INFO_COLOR, "Please select a target package containig a level in Packages Tool");

		if (ImGui::SmallButton("Show Packages Tool")) {
			GGUIManager.TryShowElement<UIElementType::PackagesTool>();
		}

		ImGui::End();
		return;
	}

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Prepare"))
		{
			ImGui::TextColored(INFO_COLOR, "AABB");
			ImGui::Text("Min x:%.2f y:%.2f z:%.2f", GScene.AABB.Box.Min.X, GScene.AABB.Box.Min.Y, GScene.AABB.Box.Min.Z);
			ImGui::Text("Max x:%.2f y:%.2f z:%.2f", GScene.AABB.Box.Max.X, GScene.AABB.Box.Max.Y, GScene.AABB.Box.Max.Z);

			float AABBHeight = 0.0f;
			if (ImGui::SliderFloat("AABB Height", &AABBHeight, 10.0f, 8000.0f)) {
				GScene.AABB.Box.Max.Y = AABBHeight;
				GScene.AABB.SetHeight(AABBHeight);
			}

			ImGui::TextColored(INFO_COLOR, "Resterization");
			ImGui::SliderFloat("Cell Size", &CellSize, 0.5f, 50.0f);
			ImGui::SliderFloat("Cell Height", &CellHeight, 0.5f, 50.0f);

			ImGui::TextColored(INFO_COLOR, "Agent");
			ImGui::SliderFloat("Height", &AgentHeight, 10.0f, 200.0f);
			ImGui::SliderFloat("Radius", &AgentRadius, 10.0f, 200.0f);
			ImGui::SliderFloat("Max Climb", &AgentMaxClimb, 1.0f, 200.0f);
			ImGui::SliderFloat("Max Slope", &AgentMaxSlope, 1.0f, 90.0f);

			ImGui::TextColored(INFO_COLOR, "Region");
			ImGui::SliderInt("Min Region Size", &RegionMinSize, 1, 350);
			ImGui::SliderInt("Merged Region Size", &RegionMergedSize, 1, 350);

			ImGui::TextColored(INFO_COLOR, "Partitioning");
			ImGui::Text("Whatershed");

			ImGui::TextColored(INFO_COLOR, "Polygonization");
			ImGui::SliderInt("Max Edge Length", &PolyMaxEdgeLength, 1, 350);
			ImGui::SliderFloat("Max Edge Error", &PolyMaxEdgeError, 0.1, 3.0f);
			ImGui::SliderInt("Verts Per Poly", &VertsPerPoly, 3, 12);

			ImGui::TextColored(INFO_COLOR, "Detail Mesh");
			ImGui::SliderInt("Sample Distance", &SampleDistance, 0, 16);
			ImGui::SliderInt("Max Sample Error", &MaxSampleError, 0, 16);

			ImGui::TextColored(INFO_COLOR, "Tiling");
			ImGui::SliderInt("Tile Size", &TileSize, 16, 2048);

			if (ImGui::Button("Prepare Navmesh Source")) {
				SourceGeom = BuildSceneInputGeom();
				if (!SourceGeom.Get()) {
					Message("Failed to Prepare Navmesh Source");
				}
				else {
					Calculate();
				}
			}

			ImGui::EndTabItem();
		}

		if (SourceGeom.Get()) {

			if (ImGui::BeginTabItem("Second"))
			{
				ImGui::TextColored(INFO_COLOR, "Max Tiles");
				ImGui::Text("%d", MaxTiles);
				ImGui::TextColored(INFO_COLOR, "Max Polys Per Tile");
				ImGui::Text("%d", MaxPolysPerTile);
				ImGui::TextColored(INFO_COLOR, "Tile Width");
				ImGui::Text("%d", TileWidth);
				ImGui::TextColored(INFO_COLOR, "Tile Height");
				ImGui::Text("%d", TileHeight);

				if (ImGui::Button("Build Navmesh")) {
					if (!BuildNavmesh(SourceGeom.Get())) {
						Message("Failed to Build Navmesh");
						Clear();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Clear")) {
					Clear();
				}

				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

TRef<InputGeom> NavmeshTool::BuildSceneInputGeom(INT lod)
{
	Clear();

	auto pMesh = std::make_unique<NavmeshMesh2>();

	TRef<Terrain> terrain = nullptr;
	std::vector<StaticMeshActor*> actors;

	if (!GScene.GetAllNavmeshObjects(terrain, actors)) {
		Message("Failed to get scene target objects");
		return nullptr;
	}

	ULONG TotalIndicesCount = 0;
	ULONG TotalVertsCount = 0;

	//Used for indices offseting
	ULONG Offset = 0;

	//Calculate elements count
	for (auto* actor : actors)
	{
		TotalVertsCount += actor->vertexCount;

		for (size_t i = 0; i < actor->mesh->lodInfo[lod].sectionsCount; i++) {
			TotalIndicesCount += actor->mesh->lodInfo[lod].sections[i].indexCount;
		}
	}

	if (terrain.Get()) {
		for (auto* component : terrain->components)
		{
			TotalIndicesCount += component->indexCount;
			TotalVertsCount += component->vertexCount;
		}
	}

	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<int> indices;

	vertices.reserve(TotalVertsCount * 3);
	normals.reserve(TotalVertsCount * 3);
	indices.reserve(TotalIndicesCount);

	if (terrain.Get()) {
		//Parse terrain 
		for (size_t j = 0; j < terrain->components.size(); j++)
		{
			auto* component = terrain->components[j];

			for (size_t i = 0; i < component->vertexCount; i++)
			{
				vertices.push_back(component->vertices[i].position.x);
				vertices.push_back(component->vertices[i].position.y);
				vertices.push_back(component->vertices[i].position.z);

				//pMesh->TempVertices.push_back(component->vertices[i]);

				normals.push_back(component->vertices[i].normal.x);
				normals.push_back(component->vertices[i].normal.y);
				normals.push_back(component->vertices[i].normal.z);
			}

			auto faceCount = component->indexCount / 3;

			for (size_t i = 0; i < faceCount; i++)
			{
				indices.push_back((int)(component->indices[i * 3 + 2]) + Offset);
				indices.push_back((int)(component->indices[i * 3 + 1]) + Offset);
				indices.push_back((int)(component->indices[i * 3 + 0]) + Offset);

				/*		pMesh->TempIndices.push_back((int)(component->indices[i * 3 + 2]) + 1 + Offset);
						pMesh->TempIndices.push_back((int)(component->indices[i * 3 + 1]) + 1 + Offset);
						pMesh->TempIndices.push_back((int)(component->indices[i * 3 + 0]) + 1 + Offset);*/
			}

			Offset += component->vertexCount;
		}
	}

	//Parse actors 
	bool failed = false;
	for (auto* actor : actors)
	{
		for (size_t i = 0; i < actor->mesh->lodInfo[lod].numVertices; i++)
		{
			vertices.push_back(actor->vertices[i].position.x);
			vertices.push_back(actor->vertices[i].position.y);
			vertices.push_back(actor->vertices[i].position.z);

			normals.push_back(actor->vertices[i].normal.x);
			normals.push_back(actor->vertices[i].normal.y);
			normals.push_back(actor->vertices[i].normal.z);
		}

		for (size_t i = 0; i < actor->mesh->lodInfo[lod].sectionsCount; i++) {
			for (size_t j = 0; j < actor->mesh->lodInfo[lod].sections[i].indexCount; j++)
			{
				if (actor->mesh->lodInfo[lod].sections[i].indexSize == sizeof(short)) {
					indices.push_back((int)(((short*)actor->mesh->lodInfo[lod].sections[i].indices)[j]) + Offset);
				}
				else if (actor->mesh->lodInfo[lod].sections[i].indexSize == sizeof(int)) {
					indices.push_back(((int*)actor->mesh->lodInfo[lod].sections[i].indices)[j] + Offset);
				}
				else {
					Message("Unsupported index buffer element size %d", actor->mesh->lodInfo[lod].sections[i].indexSize);
					failed = true;
					break;
				}
			}
		}

		if (failed) {
			break;
		}

		Offset += actor->mesh->lodInfo[lod].numVertices;
	}

	if (failed) {
		return nullptr;
	}

	auto iGeom = std::make_unique<InputGeom>();
	auto meshLoader = std::make_unique<rcMeshLoaderObj>();

	meshLoader->setData(vertices.data(), (int)vertices.size(), normals.data(), indices.data(), (int)indices.size(), "SceneMesh");

	if (!iGeom->useMesh(meshLoader.release())) {
		Message("Failed to crete InputGeom");
		return nullptr;
	}

	pMesh->TempVertices = vertices;
	pMesh->TempIndices = indices;

	if (!pMesh->Intialize()) {
		Message("Failed to created prepared color mesh!");
		return nullptr;
	}

	PreparedMesh = pMesh.release();

	return iGeom.release();
}

bool NavmeshTool::BuildNavmesh(TRef<InputGeom> IGeom)
{
	if (!IGeom.Get() || !IGeom->getMesh()) {
		return false;
	}

	if (NavMesh.Get()) {
		dtFreeNavMesh(NavMesh.Get());
		NavMesh = nullptr;
	}

	if (NavMeshQuery.Get()) {
		dtFreeNavMeshQuery(NavMeshQuery.Get());
		NavMeshQuery = nullptr;
	}

	NavMesh = dtAllocNavMesh();
	if (!NavMesh.Get())
	{
		Message("Failed to allocate navmesh");
		return false;
	}

	NavMeshQuery = dtAllocNavMeshQuery();
	if (!NavMeshQuery.Get())
	{
		Message("Failed to allocate navmeshquery");
		return false;
	}

	dtNavMeshParams params;
	rcVcopy(params.orig, IGeom->getNavMeshBoundsMin());
	params.tileWidth = TileSize * CellSize;
	params.tileHeight = TileSize * CellSize;
	params.maxTiles = MaxTiles;
	params.maxPolys = MaxPolysPerTile;

	dtStatus status;

	status = NavMesh->init(&params);
	if (dtStatusFailed(status))
	{
		return false;
	}

	status = NavMeshQuery->init(NavMesh.Get(), 2048);
	if (dtStatusFailed(status))
	{
		Message(" Could not init Detour navmesh query");
		return false;
	}

	BuildAllTiles();

	BuilDrawMesh();

	return true;
}

unsigned short sampleAreaToFlags(unsigned char area)
{
	unsigned char areaType = (area & SAMPLE_POLYAREA_TYPE_MASK);
	unsigned short flags = (unsigned short)((areaType == SAMPLE_POLYAREA_TYPE_WATER) ? SAMPLE_POLYFLAGS_SWIM : SAMPLE_POLYFLAGS_WALK);
	if ((areaType & SAMPLE_POLYAREA_FLAG_DOOR) != 0)
	{
		flags |= SAMPLE_POLYFLAGS_DOOR;
	}
	if ((areaType & SAMPLE_POLYAREA_FLAG_JUMP) != 0)
	{
		flags |= SAMPLE_POLYFLAGS_JUMP;
	}
	return flags;
}

unsigned char* NavmeshTool::BuildTileMesh(const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize)
{
	if (!SourceGeom.Get() || !SourceGeom.Get()->getMesh() || !SourceGeom.Get()->getChunkyMesh())
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Input mesh is not specified.");
		Message("BuildTileMesh: Input mesh is not specified.");
		return 0;
	}

	TileMemUsage = 0;
	TileBuildTime = 0;

	cleanup();

	const float* verts = SourceGeom.Get()->getMesh()->getVerts();
	const int nverts = SourceGeom.Get()->getMesh()->getVertCount();
	const int ntris = SourceGeom.Get()->getMesh()->getTriCount();
	const rcChunkyTriMesh* chunkyMesh = SourceGeom.Get()->getChunkyMesh();

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = CellSize;
	m_cfg.ch = CellHeight;
	m_cfg.walkableSlopeAngle = AgentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(AgentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(AgentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(AgentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(PolyMaxEdgeLength / CellSize);
	m_cfg.maxSimplificationError = PolyMaxEdgeError;
	m_cfg.minRegionArea = (int)rcSqr(RegionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(RegionMergedSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)VertsPerPoly;
	m_cfg.tileSize = (int)TileSize;
	m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
	m_cfg.width = m_cfg.tileSize + m_cfg.borderSize * 2;
	m_cfg.height = m_cfg.tileSize + m_cfg.borderSize * 2;
	m_cfg.detailSampleDist = SampleDistance < 0.9f ? 0 : CellSize * SampleDistance;
	m_cfg.detailSampleMaxError = CellHeight * MaxSampleError;

	// Expand the heighfield bounding box by border size to find the extents of geometry we need to build this tile.
	//
	// This is done in order to make sure that the navmesh tiles connect correctly at the borders,
	// and the obstacles close to the border work correctly with the dilation process.
	// No polygons (or contours) will be created on the border area.
	//
	// IMPORTANT!
	//
	//   :''''''''':
	//   : +-----+ :
	//   : |     | :
	//   : |     |<--- tile to build
	//   : |     | :  
	//   : +-----+ :<-- geometry needed
	//   :.........:
	//
	// You should use this bounding box to query your input geometry.
	//
	// For example if you build a navmesh for terrain, and want the navmesh tiles to match the terrain tile size
	// you will need to pass in data from neighbour terrain tiles too! In a simple case, just pass in all the 8 neighbours,
	// or use the bounding box below to only pass in a sliver of each of the 8 neighbours.
	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);
	m_cfg.bmin[0] -= m_cfg.borderSize * m_cfg.cs;
	m_cfg.bmin[2] -= m_cfg.borderSize * m_cfg.cs;
	m_cfg.bmax[0] += m_cfg.borderSize * m_cfg.cs;
	m_cfg.bmax[2] += m_cfg.borderSize * m_cfg.cs;

	// Reset build times gathering.
	m_ctx->resetTimers();

	// Start the build process.
	m_ctx->startTimer(RC_TIMER_TOTAL);

	m_ctx->log(RC_LOG_PROGRESS, "Building navigation:");
	m_ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
	m_ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);

	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		Message("buildNavigation: Out of memory 'solid'.");
		return 0;
	}
	if (!rcCreateHeightfield(nullptr, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		Message("buildNavigation: Could not create solid heightfield.");
		return 0;
	}

	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triareas = new unsigned char[chunkyMesh->maxTrisPerChunk];
	if (!m_triareas)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
		Message("buildNavigation: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
		return 0;
	}

	float tbmin[2], tbmax[2];
	tbmin[0] = m_cfg.bmin[0];
	tbmin[1] = m_cfg.bmin[2];
	tbmax[0] = m_cfg.bmax[0];
	tbmax[1] = m_cfg.bmax[2];
	int cid[512];// TODO: Make grow when returning too many items.
	const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
	if (!ncid)
		return 0;

	TileTriCount = 0;

	for (int i = 0; i < ncid; ++i)
	{
		const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		const int* ctris = &chunkyMesh->tris[node.i * 3];
		const int nctris = node.n;

		TileTriCount += nctris;

		memset(m_triareas, 0, nctris * sizeof(unsigned char));
		rcMarkWalkableTriangles(nullptr, m_cfg.walkableSlopeAngle,
			verts, nverts, ctris, nctris, m_triareas, SAMPLE_AREAMOD_GROUND);

		if (!rcRasterizeTriangles(nullptr, verts, nverts, ctris, m_triareas, nctris, *m_solid, m_cfg.walkableClimb))
			return 0;
	}

	/*if (!m_keepInterResults)
	{*/
	delete[] m_triareas;
	m_triareas = 0;
	//}

	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (m_filterLowHangingObstacles)
		rcFilterLowHangingWalkableObstacles(nullptr, m_cfg.walkableClimb, *m_solid);
	if (m_filterLedgeSpans)
		rcFilterLedgeSpans(nullptr, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	if (m_filterWalkableLowHeightSpans)
		rcFilterWalkableLowHeightSpans(nullptr, m_cfg.walkableHeight, *m_solid);

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		Message("buildNavigation: Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(nullptr, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		Message("buildNavigation: Could not build compact data.");
		return 0;
	}

	/*if (!m_keepInterResults)
	{*/
	rcFreeHeightField(m_solid);
	m_solid = 0;
	//}

	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(nullptr, m_cfg.walkableRadius, *m_chf))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		Message("buildNavigation: Could not erode.");
		return 0;
	}

	// (Optional) Mark areas.
	const ConvexVolume* vols = SourceGeom->getConvexVolumes();
	for (int i = 0; i < SourceGeom->getConvexVolumeCount(); ++i)
		rcMarkConvexPolyArea(nullptr, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, vols[i].areaMod, *m_chf);

	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles

	/*if (m_partitionType == SAMPLE_PARTITION_WATERSHED)
	{*/
	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(nullptr, *m_chf))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
		Message("buildNavigation: Could not build distance field.");
		return 0;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(nullptr, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
		Message("buildNavigation: Could not build watershed regions.");
		return 0;
	}
	//}
	//else if (m_partitionType == SAMPLE_PARTITION_MONOTONE)
	//{
	//	// Partition the walkable surface into simple regions without holes.
	//	// Monotone partitioning does not need distancefield.
	//	if (!rcBuildRegionsMonotone(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
	//	{
	//		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
	//		return 0;
	//	}
	//}
	//else // SAMPLE_PARTITION_LAYERS
	//{
	//	// Partition the walkable surface into simple regions without holes.
	//	if (!rcBuildLayerRegions(m_ctx, *m_chf, m_cfg.borderSize, m_cfg.minRegionArea))
	//	{
	//		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
	//		return 0;
	//	}
	//}

	// Create contours.
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		Message("buildNavigation: Out of memory 'cset'.");
		return 0;
	}
	if (!rcBuildContours(nullptr, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		Message("buildNavigation: Could not create contours.");
		return 0;
	}

	if (m_cset->nconts == 0)
	{
		return 0;
	}

	// Build polygon navmesh from the contours.
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return 0;
	}
	if (!rcBuildPolyMesh(nullptr, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		Message("buildNavigation: Could not create contours.");
		return 0;
	}

	// Build detail mesh.
	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'dmesh'.");
		Message("buildNavigation: Out of memory 'dmesh'.");
		return 0;
	}

	if (!rcBuildPolyMeshDetail(nullptr, *m_pmesh, *m_chf,
		m_cfg.detailSampleDist, m_cfg.detailSampleMaxError,
		*m_dmesh))
	{
		m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could build polymesh detail.");
		Message("buildNavigation: Could build polymesh detail.");
		return 0;
	}

	/*if (!m_keepInterResults)
	{*/
	rcFreeCompactHeightfield(m_chf);
	m_chf = 0;
	rcFreeContourSet(m_cset);
	m_cset = 0;
	//}

	unsigned char* navData = 0;
	int navDataSize = 0;
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		if (m_pmesh->nverts >= 0xffff)
		{
			// The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
			m_ctx->log(RC_LOG_ERROR, "Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
			Message("Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
			return 0;
		}

		// Update poly flags from areas.
		for (int i = 0; i < m_pmesh->npolys; ++i)
		{
			m_pmesh->flags[i] = sampleAreaToFlags(m_pmesh->areas[i]);
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		params.offMeshConVerts = SourceGeom.Get()->getOffMeshConnectionVerts();
		params.offMeshConRad = SourceGeom.Get()->getOffMeshConnectionRads();
		params.offMeshConDir = SourceGeom.Get()->getOffMeshConnectionDirs();
		params.offMeshConAreas = SourceGeom.Get()->getOffMeshConnectionAreas();
		params.offMeshConFlags = SourceGeom.Get()->getOffMeshConnectionFlags();
		params.offMeshConUserID = SourceGeom.Get()->getOffMeshConnectionId();
		params.offMeshConCount = SourceGeom.Get()->getOffMeshConnectionCount();
		params.walkableHeight = AgentHeight;
		params.walkableRadius = AgentRadius;
		params.walkableClimb = AgentMaxClimb;
		params.tileX = tx;
		params.tileY = ty;
		params.tileLayer = 0;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			//m_ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return 0;
		}
	}

	TileMemUsage = navDataSize / 1024.0f;

	//m_ctx->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	//duLogBuildTimes(*m_ctx, m_ctx->getAccumulatedTime(RC_TIMER_TOTAL));
	m_ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);

	TileBuildTime = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;

	dataSize = navDataSize;

	return navData;
}

TRef<NavmeshMesh> BuilDrawMeshTile(const dtNavMesh& mesh, const dtNavMeshQuery* query,
	const dtMeshTile* tile, unsigned char flags) {

	auto outMesh = std::make_unique<NavmeshMesh>();

	dtPolyRef base = mesh.getPolyRefBase(tile);

	int tileNum = mesh.decodePolyIdTile(base);
	const unsigned int tileColor = duIntToCol(tileNum, 128);

	for (int i = 0; i < tile->header->polyCount; ++i)
	{
		const dtPoly* p = &tile->polys[i];
		if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
			continue;

		const dtPolyDetail* pd = &tile->detailMeshes[i];

		for (int j = 0; j < pd->triCount; ++j)
		{
			int* t = &(((int*)tile->detailTris)[pd->triBase + j]);

			outMesh->TempIndices.push_back(*t);
		}

		for (int j = 0; j < pd->vertCount / 3; j += 3)
		{
			D3DColorVertex v;
			v.position.x = tile->detailVerts[pd->vertBase + j + 0];
			v.position.y = tile->detailVerts[pd->vertBase + j + 1];
			v.position.z = tile->detailVerts[pd->vertBase + j + 2];

			v.color.x = 0;
			v.color.y = 0;
			v.color.z = 0;
			v.color.w = 0.4f;

			outMesh->TempVertices.push_back(v);
		}
	}

	if (!outMesh->Intialize()) {
		return nullptr;
	}

	return outMesh.release();
}

void NavmeshTool::BuildAllTiles()
{
	if (!SourceGeom.Get()) return;
	if (!NavMesh.Get()) return;

	const float* bmin = SourceGeom->getNavMeshBoundsMin();
	const float* bmax = SourceGeom->getNavMeshBoundsMax();
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, CellSize, &gw, &gh);
	const int ts = (int)TileSize;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	const float tcs = TileSize * CellSize;

	// Start the build process.
	//m_ctx->startTimer(RC_TIMER_TEMP);

	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			LastBuiltTileBmin[0] = bmin[0] + x * tcs;
			LastBuiltTileBmin[1] = bmin[1];
			LastBuiltTileBmin[2] = bmin[2] + y * tcs;

			LastBuiltTileBmax[0] = bmin[0] + (x + 1) * tcs;
			LastBuiltTileBmax[1] = bmax[1];
			LastBuiltTileBmax[2] = bmin[2] + (y + 1) * tcs;

			int dataSize = 0;
			unsigned char* data = BuildTileMesh(x, y, LastBuiltTileBmin, LastBuiltTileBmax, dataSize);
			if (data)
			{
				// Remove any previous data (navmesh owns and deletes the data).
				NavMesh->removeTile(NavMesh->getTileRefAt(x, y, 0), 0, 0);
				// Let the navmesh own the data.
				dtStatus status = NavMesh->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, 0);
				if (dtStatusFailed(status))
					dtFree(data);
			}
		}
	}

	// Start the build process.	
	//m_ctx->stopTimer(RC_TIMER_TEMP);

	//m_totalBuildTimeMs = m_ctx->getAccumulatedTime(RC_TIMER_TEMP) / 1000.0f;
}

void NavmeshTool::BuilDrawMesh(unsigned char flags)
{
	for (int i = 0; i < /*NavMesh->getMaxTiles()*/ 3; ++i)
	{
		const dtMeshTile* tile = NavMesh->getTile(i);
		if (!tile->header) continue;
		auto TileMesh = BuilDrawMeshTile(*NavMesh.Get(), 0, tile, flags);

		DrawMeshes.push_back(TileMesh.Get());
	}
}

void NavmeshTool::Calculate()
{
	int gw = 0, gh = 0;
	const float* bmin = SourceGeom->getNavMeshBoundsMin();
	const float* bmax = SourceGeom->getNavMeshBoundsMax();
	rcCalcGridSize(bmin, bmax, CellSize, &gw, &gh);
	const int ts = (int)TileSize;
	TileWidth = (gw + ts - 1) / ts;
	TileHeight = (gh + ts - 1) / ts;
	//snprintf(text, 64, "Tiles  %d x %d", tw, th);
	//imguiValue(text);

	// Max tiles and max polys affect how the tile IDs are caculated.
	// There are 22 bits available for identifying a tile and a polygon.
	int tileBits = Min((int)ilog2(nextPow2(TileWidth * TileHeight)), 14);
	if (tileBits > 14) tileBits = 14;
	int polyBits = 22 - tileBits;
	MaxTiles = 1 << tileBits;
	MaxPolysPerTile = 1 << polyBits;

	////snprintf(text, 64, "Max Tiles  %d", m_maxTiles);
	//imguiValue(text);
	//snprintf(text, 64, "Max Polys  %d", m_maxPolysPerTile);
	//imguiValue(text);
}

//NavmeshMesh
bool NavmeshMesh::GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<INT>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format)
{
	OutTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Format = DXGI_FORMAT_R32_SINT;

	OutVertices = std::move(TempVertices);
	OutIndices = std::move(TempIndices);

	return true;
}

void NavmeshMesh::SetShaders(ID3D11DeviceContext* Context)
{
	auto Shader = GD3DDriver.GetShaderPack(AppShader::LineShaderPack);
	if (Shader.Get()) {
		Shader->SetShader(Context);
	}
}

//NavmeshMesh2
bool NavmeshMesh2::GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<INT>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format)
{
	OutTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Format = DXGI_FORMAT_R32_UINT;

	OutVertices.reserve(TempVertices.size() / 3);
	OutIndices = std::move(TempIndices);

	for (size_t i = 0; i < TempVertices.size(); i += 3)
	{
		D3DColorVertex v;
		v.position.x = TempVertices[i + 0];
		v.position.y = TempVertices[i + 1];
		v.position.z = TempVertices[i + 2];
		v.color = Color;

		OutVertices.push_back(v);
	}

	return true;
}

void NavmeshMesh2::SetShaders(ID3D11DeviceContext* Context)
{
	auto Shader = GD3DDriver.GetShaderPack(AppShader::LineShaderPack);
	if (Shader.Get()) {
		Shader->SetShader(Context);
	}
}

//Globals
NavmeshTool	 GNavmeshTool;

rcAreaModification const SAMPLE_AREAMOD_GROUND(SAMPLE_POLYAREA_TYPE_GROUND, SAMPLE_POLYAREA_TYPE_MASK);
static rcAreaModification const SAMPLE_AREAMOD_WATER(SAMPLE_POLYAREA_TYPE_WATER, SAMPLE_POLYAREA_TYPE_MASK);
static rcAreaModification const SAMPLE_AREAMOD_ROAD(SAMPLE_POLYAREA_TYPE_ROAD, SAMPLE_POLYAREA_TYPE_MASK);
static rcAreaModification const SAMPLE_AREAMOD_GRASS(SAMPLE_POLYAREA_TYPE_GRASS, SAMPLE_POLYAREA_TYPE_MASK);
static rcAreaModification const SAMPLE_AREAMOD_DOOR(SAMPLE_POLYAREA_FLAG_DOOR, SAMPLE_POLYAREA_FLAG_DOOR);
static rcAreaModification const SAMPLE_AREAMOD_JUMP(SAMPLE_POLYAREA_FLAG_JUMP, SAMPLE_POLYAREA_FLAG_JUMP);


////////////////////////////////////////////////////////////////////////////////////////////////////

BuildContext::BuildContext() :
	m_messageCount(0),
	m_textPoolSize(0)
{
	memset(m_messages, 0, sizeof(char*) * MAX_MESSAGES);

	resetTimers();
}

// Virtual functions for custom implementations.
void BuildContext::doResetLog()
{
	m_messageCount = 0;
	m_textPoolSize = 0;
}

void BuildContext::doLog(const rcLogCategory category, const char* msg, const int len)
{
	if (!len) return;
	if (m_messageCount >= MAX_MESSAGES)
		return;
	char* dst = &m_textPool[m_textPoolSize];
	int n = TEXT_POOL_SIZE - m_textPoolSize;
	if (n < 2)
		return;
	char* cat = dst;
	char* text = dst + 1;
	const int maxtext = n - 1;
	// Store category
	*cat = (char)category;
	// Store message
	const int count = rcMin(len + 1, maxtext);
	memcpy(text, msg, count);
	text[count - 1] = '\0';
	m_textPoolSize += 1 + count;
	m_messages[m_messageCount++] = dst;
}

void BuildContext::doResetTimers()
{
	for (int i = 0; i < RC_MAX_TIMERS; ++i)
		m_accTime[i] = -1;
}

void BuildContext::doStartTimer(const rcTimerLabel label)
{
	m_startTime[label] = getPerfTime();
}

void BuildContext::doStopTimer(const rcTimerLabel label)
{
	const TimeVal endTime = getPerfTime();
	const int deltaTime = (int)(endTime - m_startTime[label]);
	if (m_accTime[label] == -1)
		m_accTime[label] = deltaTime;
	else
		m_accTime[label] += deltaTime;
}

int BuildContext::doGetAccumulatedTime(const rcTimerLabel label) const
{
	return getPerfTimeUsec(m_accTime[label]);
}

void BuildContext::dumpLog(const char* format, ...)
{
	// Print header.
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("\n");

	// Print messages
	const int TAB_STOPS[4] = { 28, 36, 44, 52 };
	for (int i = 0; i < m_messageCount; ++i)
	{
		const char* msg = m_messages[i] + 1;
		int n = 0;
		while (*msg)
		{
			if (*msg == '\t')
			{
				int count = 1;
				for (int j = 0; j < 4; ++j)
				{
					if (n < TAB_STOPS[j])
					{
						count = TAB_STOPS[j] - n;
						break;
					}
				}
				while (--count)
				{
					putchar(' ');
					n++;
				}
			}
			else
			{
				putchar(*msg);
				n++;
			}
			msg++;
		}
		putchar('\n');
	}
}

int BuildContext::getLogCount() const
{
	return m_messageCount;
}

const char* BuildContext::getLogText(const int i) const
{
	return m_messages[i] + 1;
}
