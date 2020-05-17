#pragma once

struct NOVTABLE SceneMeshBase {
	virtual void Render(ID3D11DeviceContext*, UINT Offset = 0) = 0;
};

template<typename VertexType, typename IndexType, bool Dyanmic = false>
struct NOVTABLE SceneMesh : SceneMeshBase {

	virtual bool Intialize();

	virtual bool GenerateMesh(std::vector<VertexType>& OutVertices, std::vector<IndexType>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format) = 0;
	virtual void SetShaders(ID3D11DeviceContext* Context) = 0;

	bool IsDynamic() const noexcept { if constexpr (Dyanmic) { return true; } else { return false; } }
	void Touch() {
		NeedsUpdate = TRUE;
	}

	virtual bool Build();
	virtual void Render(ID3D11DeviceContext* Context, UINT Offset = 0);

	//If SceneMesh<Dynamic = false> returns nullptr
	VertexType* GetVertices() noexcept {
		if constexpr (Dyanmic) {
			return Vertices.data();
		}
		else {
			return nullptr;
		}
	}

protected:

	BOOL								NeedsUpdate = TRUE;
	const UINT							Stride = sizeof(VertexType);
	TRef<ID3D11Buffer>					VertexBuffer = nullptr;
	TRef<ID3D11Buffer>					IndexBuffer = nullptr;
	D3D11_PRIMITIVE_TOPOLOGY			Topology;
	DXGI_FORMAT							DxgiFormat;

private:
	void UpdateVBuffer(ID3D11DeviceContext* Context);

	std::vector<VertexType>				Vertices;
	std::vector<IndexType>				Indices;
};

template<typename VertexType, typename IndexType, bool Dyanmic>
inline bool SceneMesh<VertexType, IndexType, Dyanmic>::Intialize()
{
	if (!GenerateMesh(Vertices, Indices, Topology, DxgiFormat)) {
		return false;
	}

	if (!Build()) {
		return false;
	}

	return true;
}

template<typename VertexType, typename IndexType, bool Dyanmic>
inline bool SceneMesh<VertexType, IndexType, Dyanmic>::Build()
{
	D3D11_USAGE Usage = IsDynamic() ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	D3D11_CPU_ACCESS_FLAG CPUAccessFlags = IsDynamic() ? D3D11_CPU_ACCESS_WRITE : (D3D11_CPU_ACCESS_FLAG)0;

	D3D11_BUFFER_DESC bDesc;
	bDesc.Usage = Usage;
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(VertexType) * Vertices.size();
	bDesc.CPUAccessFlags = CPUAccessFlags;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA bData;
	bData.pSysMem = Vertices.data();
	bData.SysMemPitch = 0;
	bData.SysMemSlicePitch = 0;

	auto result = GetD3DDevice()->CreateBuffer(&bDesc, &bData, VertexBuffer.GetSelfPtr());
	if (FAILED(result)) {
		return false;
	}

	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(IndexType) * Indices.size();

	ZeroMemory(&bData, sizeof(bData));
	bData.pSysMem = Indices.data();

	result = GetD3DDevice()->CreateBuffer(&bDesc, &bData, IndexBuffer.GetSelfPtr());
	if (FAILED(result)) {
		return false;
	}

	return true;
}

template<typename VertexType, typename IndexType, bool Dyanmic>
inline void SceneMesh<VertexType, IndexType, Dyanmic>::Render(ID3D11DeviceContext* Context, UINT Offset)
{
	SetShaders(Context);

	if (IsDynamic()) {
		UpdateVBuffer(Context);
	}

	Context->IASetPrimitiveTopology(Topology);
	Context->IASetVertexBuffers(0, 1, VertexBuffer.GetSelfPtr(), &Stride, &Offset);
	Context->IASetIndexBuffer(IndexBuffer.Get(), DxgiFormat, 0);

	Context->DrawIndexed(Indices.size(), 0, 0);
}

template<typename VertexType, typename IndexType, bool Dyanmic>
inline void SceneMesh<VertexType, IndexType, Dyanmic>::UpdateVBuffer(ID3D11DeviceContext* Context)
{
	if constexpr (Dyanmic) {
		if (NeedsUpdate) {
			D3D11_MAPPED_SUBRESOURCE mappedResource;

			Context->Map(VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

			memcpy_s(mappedResource.pData, sizeof(VertexType) * Vertices.size(), Vertices.data(), sizeof(VertexType) * Vertices.size());

			Context->Unmap(VertexBuffer.Get(), 0);

			NeedsUpdate = FALSE;
		}
	}
	else {
		return;
	}
}

template<typename T>
struct NOVTABLE ShaderResource {
	static_assert((sizeof(T) % 16) == 0, "Size of T must be multiple of 16");

	inline void UpdateValue(ID3D11DeviceContext* Context) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		Context->Map(Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		T* mDataPtr = (T*)mappedResource.pData;

		*mDataPtr = this->Data;

		Context->Unmap(Buffer.Get(), 0);

		NeedsUpdate = FALSE;
	}
	virtual bool Initialize(ID3D11Device* Device) {
		D3D11_BUFFER_DESC bDesc;
		bDesc.Usage = D3D11_USAGE_DYNAMIC;
		bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bDesc.ByteWidth = sizeof(T);
		bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bDesc.MiscFlags = 0;
		bDesc.StructureByteStride = 0;

		HRESULT CreateResult = Device->CreateBuffer(&bDesc, NULL, Buffer.GetSelfPtr());
		if (FAILED(CreateResult)) {
			return false;
		}

		return true;
	}

	TRef<ID3D11Buffer>	Buffer = nullptr;
	T					Data;
	BOOL				NeedsUpdate = TRUE;
};

struct SceneLightData { //dont touch , order and size matter
	D3DXVECTOR4		Color;
	D3DXVECTOR4		Direction;
	float			Ambient = 0.0f;
	float			Diffuse = 0.0f;
	float			Specular = 0.0f;
	float			Shininess = 0.0f;
	D3DXVECTOR4		AmbientLight;
	D3DXVECTOR4		Eye;

	SceneLightData() {}
	SceneLightData(const SceneLightData& other) {
		Ambient = other.Ambient;
		Diffuse = other.Diffuse;
		Specular = other.Specular;
		Shininess = other.Shininess;
		Direction = other.Direction;
		Color = other.Color;
		AmbientLight = other.AmbientLight;
		Eye = other.Eye;
	}
	void operator=(const SceneLightData& other) {
		Ambient = other.Ambient;
		Diffuse = other.Diffuse;
		Specular = other.Specular;
		Shininess = other.Shininess;
		Direction = other.Direction;
		Color = other.Color;
		AmbientLight = other.AmbientLight;
		Eye = other.Eye;
	};
};
