#pragma once

/// Mask of the ceil part of the area id (3 lower bits)
/// the 0 value (RC_NULL_AREA) is left unused
static const unsigned char SAMPLE_POLYAREA_TYPE_MASK = 0x07;
/// Value for the kind of ceil "ground"
static const unsigned char SAMPLE_POLYAREA_TYPE_GROUND = 0x1;
/// Value for the kind of ceil "water"
static const unsigned char SAMPLE_POLYAREA_TYPE_WATER = 0x2;
/// Value for the kind of ceil "road"
static const unsigned char SAMPLE_POLYAREA_TYPE_ROAD = 0x3;
/// Value for the kind of ceil "grass"
static const unsigned char SAMPLE_POLYAREA_TYPE_GRASS = 0x4;
/// Flag for door area. Can be combined with area types and jump flag.
static const unsigned char SAMPLE_POLYAREA_FLAG_DOOR = 0x08;
/// Flag for jump area. Can be combined with area types and door flag.
static const unsigned char SAMPLE_POLYAREA_FLAG_JUMP = 0x10;

extern rcAreaModification const SAMPLE_AREAMOD_GROUND;

enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
	SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
	SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
};

struct NavmeshMesh : SceneMesh<D3DColorVertex, INT> {
	bool GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<INT>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format);
	void SetShaders(ID3D11DeviceContext* Context);

	std::vector<D3DColorVertex> TempVertices;
	std::vector<INT> TempIndices;
};

struct NavmeshMesh2 : SceneMesh<D3DColorVertex, INT> {
	bool GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<INT>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format);
	void SetShaders(ID3D11DeviceContext* Context);

	bool Intialize() override {
		if (!SceneMesh<D3DColorVertex, INT>::Intialize()) {
			return false;
		}

		AABB.Box.Min = { FLT_MAX,FLT_MAX ,FLT_MAX };
		AABB.Box.Max = { FLT_MIN,FLT_MIN ,FLT_MIN };

		for (size_t i = 0; i < TempVertices.size(); i+= 3)
		{
			AABB.Box.Min.X = fmin(AABB.Box.Min.X, TempVertices[i + 0]);
			AABB.Box.Min.Y = fmin(AABB.Box.Min.Y, TempVertices[i + 1]);
			AABB.Box.Min.Z = fmin(AABB.Box.Min.Z, TempVertices[i + 2]);

			AABB.Box.Max.X = fmax(AABB.Box.Max.X, TempVertices[i + 0]);
			AABB.Box.Max.Y = fmax(AABB.Box.Max.Y, TempVertices[i + 1]);
			AABB.Box.Max.Z = fmax(AABB.Box.Max.Z, TempVertices[i + 2]);
		}

		return AABB.Intialize();
	}

	void Render(ID3D11DeviceContext* Context, UINT Offset = 0) {

		AABB.Render(Context);

		SceneMesh<D3DColorVertex, INT>::Render(Context, Offset);
	}

	SceneAABB					AABB;

	D3DXVECTOR4					Color{ 0.0f,0.0f ,0.0f, 1.0f };
	std::vector<float>			TempVertices;
	std::vector<INT>			TempIndices;
};

/// Recast build context.
class BuildContext : public rcContext
{
	TimeVal m_startTime[RC_MAX_TIMERS];
	TimeVal m_accTime[RC_MAX_TIMERS];

	static const int MAX_MESSAGES = 1000;
	const char* m_messages[MAX_MESSAGES];
	int m_messageCount;
	static const int TEXT_POOL_SIZE = 8000;
	char m_textPool[TEXT_POOL_SIZE];
	int m_textPoolSize;

public:
	BuildContext();

	/// Dumps the log to stdout.
	void dumpLog(const char* format, ...);
	/// Returns number of log messages.
	int getLogCount() const;
	/// Returns log message text.
	const char* getLogText(const int i) const;

protected:
	/// Virtual functions for custom implementations.
	///@{
	virtual void doResetLog();
	virtual void doLog(const rcLogCategory category, const char* msg, const int len);
	virtual void doResetTimers();
	virtual void doStartTimer(const rcTimerLabel label);
	virtual void doStopTimer(const rcTimerLabel label);
	virtual int doGetAccumulatedTime(const rcTimerLabel label) const;
	///@}
};

struct NavmeshTool : UIElement<UIElementType::NavmeshTool> {

	NavmeshTool() {
		SetDefaults();
	}

	~NavmeshTool() {
		Clear();
	}

	bool Initialize();
	void Render(ID3D11DeviceContext* Context, float dt);
	void RenderUI() override;
	void SetDefaults() {
		CellSize = 1.0f;
		CellHeight = 1.0f;
		AgentHeight = 80.0f;
		AgentRadius = 20.0f;
		AgentMaxClimb = 20.0f;
		AgentMaxSlope = 46.0f;
		RegionMinSize = 8.0f;
		RegionMergedSize = 20.0f;
		PolyMaxEdgeLength = 50.0f;
		PolyMaxEdgeError = 1.3f;
		VertsPerPoly = 3.0f;
		SampleDistance = 1.0f;
		MaxSampleError = 1.0f;
		TileSize = 192;
		Partitioning = 0;
		FilterFlags = 0;
	}

	TRef<InputGeom> BuildSceneInputGeom(INT lod = 0);

	bool BuildNavmesh(TRef<InputGeom> InputGeom);
	void BuilDrawMesh(unsigned char flags = 0xff);
	void Clear() {
		if (SourceGeom.Get()) {
			delete SourceGeom.Get();
			SourceGeom = nullptr;
		}

		MaxTiles = 0;
		MaxPolysPerTile = 0;
		TileWidth = 0;
		TileHeight = 0;

		if (NavMesh.Get()) {
			dtFreeNavMesh(NavMesh.Get());
			NavMesh = nullptr;
		}

		if (NavMeshQuery.Get()) {
			dtFreeNavMeshQuery(NavMeshQuery.Get());
			NavMeshQuery = nullptr;
		}


		for (auto* mesh : DrawMeshes)
		{
			if (mesh) {
				delete mesh;
			}
		}
		DrawMeshes.clear();

		if (PreparedMesh.Get()) {
			delete PreparedMesh.Get();
			PreparedMesh = nullptr;
		}

		cleanup();
	}

private:
	unsigned char* BuildTileMesh(const int TileX, const int TileY, const float* BoundMin, const float* BoundMax, int& DataSize);
	void BuildAllTiles();

	void Calculate();
	void cleanup() {
		delete[] m_triareas;
		m_triareas = 0;
		rcFreeHeightField(m_solid);
		m_solid = 0;
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
		rcFreePolyMesh(m_pmesh);
		m_pmesh = 0;
		rcFreePolyMeshDetail(m_dmesh);
		m_dmesh = 0;
	}

	//Build time
	unsigned char* m_triareas = nullptr;
	rcHeightfield* m_solid = nullptr;
	rcCompactHeightfield* m_chf = nullptr;
	rcContourSet* m_cset = nullptr;
	rcPolyMesh* m_pmesh = nullptr;
	rcPolyMeshDetail* m_dmesh = nullptr;
	rcConfig					m_cfg;
	bool						m_filterLowHangingObstacles = true;
	bool						m_filterLedgeSpans = true;
	bool						m_filterWalkableLowHeightSpans = true;

	UINT						TileCol = 0;
	float						LastBuiltTileBmin[3]{ 0.0f, 0.0f, 0.0f };
	float						LastBuiltTileBmax[3]{ 0.0f, 0.0f, 0.0f };
	float						TileBuildTime = 0.0f;
	float						TileMemUsage = 0.0f;
	INT							TileTriCount = 0.0f;

	//Calcualted
	INT							MaxTiles = 0;
	INT							MaxPolysPerTile = 0;
	INT							TileWidth = 0;
	INT							TileHeight = 0;

	//Properties
	float						CellSize = 0.0f;
	float						CellHeight = 0.0f;
	float						AgentHeight = 0.0f;
	float						AgentRadius = 0.0f;
	float						AgentMaxClimb = 0.0f;
	float						AgentMaxSlope = 0.0f;
	INT							RegionMinSize = 0;
	INT							RegionMergedSize = 0;
	INT							PolyMaxEdgeLength = 0;
	float						PolyMaxEdgeError = 0.0f;
	INT							VertsPerPoly = 0;
	INT							SampleDistance = 0;
	INT							MaxSampleError = 0;
	INT							TileSize = 0;
	INT							Partitioning = 0;
	ULONG						FilterFlags = 0;

	TRef<InputGeom>				SourceGeom = nullptr;
	TRef<dtNavMesh>				NavMesh = nullptr;
	TRef<dtNavMeshQuery>		NavMeshQuery = nullptr;

	std::vector<NavmeshMesh*>	DrawMeshes;

	TRef<NavmeshMesh2>			PreparedMesh = nullptr;

	TRef<BuildContext>			m_ctx = nullptr;
};
