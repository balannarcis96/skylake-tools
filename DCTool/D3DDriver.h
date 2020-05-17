#pragma once	

constexpr int MaxShaderInputVariablesCount = 16;

enum class AppShader {
	None = 0,
	MainShaderPack,
	LineShaderPack,
	MAX
};

struct D3DColorVertex {
	D3DXVECTOR3 position;
	D3DXVECTOR4 color;

	D3DColorVertex() {}
	D3DColorVertex(const D3DColorVertex& other) {
		memcpy_s(this, sizeof(D3DColorVertex), &other, sizeof(D3DColorVertex));
	}
	void operator=(const D3DColorVertex& other) {
		memcpy_s(this, sizeof(D3DColorVertex), &other, sizeof(D3DColorVertex));
	}
};

struct RegisteredShader {
	std::string		PShaderFileName;
	std::string		VShaderFileName;
	int				Version = 0;
	bool			IsValid = true;
	std::string		Name;
	std::vector<RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>> InputElementDesc;
};

struct D3DShaderPack {
	TRef<ID3D11PixelShader>					pixelShader = nullptr;
	TRef<ID3D11VertexShader>				vertexShader = nullptr;
	TRef<ID3D11InputLayout>					inputLayout = nullptr;

	TRef<RegisteredShader>					source = nullptr;

	void SetShader(ID3D11DeviceContext* Context)const noexcept {
		Context->IASetInputLayout(inputLayout.Get());

		Context->PSSetShader(pixelShader.Get(), nullptr, 0);
		Context->VSSetShader(vertexShader.Get(), nullptr, 0);
	}

	static TRef<D3DShaderPack>	Create(RegisteredShader* rShader);

	~D3DShaderPack() {
		if (inputLayout.Get()) {
			inputLayout->Release();
			inputLayout = nullptr;
		}

		if (pixelShader.Get()) {
			pixelShader->Release();
			pixelShader = nullptr;
		}

		if (vertexShader.Get()) {
			vertexShader->Release();
			vertexShader = nullptr;
		}
	}
};

const float CLEAR_COLOR[4]{ 39.0f / 255.0f, 58.0f / 255.0f,93.0f / 255.0f,1.0f };

struct D3DDriver {
	TRef<ID3D11Device>						device = nullptr;
	TRef<ID3D11DeviceContext>				context = nullptr;
	TRef<IDXGISwapChain>					swapChain = nullptr;
	TRef<ID3D11RenderTargetView>			renderTargetView = nullptr;
	TRef<ID3D11DepthStencilState>			depthStencil = nullptr;
	TRef<ID3D11Texture2D>					depthStencilTexture = nullptr;
	TRef<ID3D11DepthStencilView>			depthStencilView = nullptr;
	TRef<ID3D11RasterizerState>				rasterizerState = nullptr;
	TRef<ID3D11BlendState>					alphaBlend = nullptr;
	TRef<ID3D11BlendState>					disabledAlphaBlend = nullptr;

	int Initialize(int width, int height);
	void Shutdown() {}

	TRef<D3DShaderPack>	GetShaderPack(AppShader Shader) {
		if (activeShaders[(int)Shader].Get()) {
			return activeShaders[(int)Shader].Get();
		}

		if (!LoadShaderPack(Shader)) {
			return nullptr;
		}

		return activeShaders[(int)Shader];
	}
	TRef<D3DShaderPack> GetShaderPackNoLoad(AppShader Shader) {
		if (activeShaders[(int)Shader].Get()) {
			return activeShaders[(int)Shader].Get();
		}

		return nullptr;
	}

	void D3DBegin(D3DXVECTOR4& ClearColor)
	{
		SetDefaultRT();

		context->OMSetBlendState(alphaBlend.Get(), 0, 0xffffffff);
		context->ClearRenderTargetView(renderTargetView.Get(), ClearColor);
		context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
	}
	void D3DEnd()
	{
		swapChain->Present(1, 0);
	}

	const D3D11_VIEWPORT& GetViewport() const noexcept { return vp; }

	bool RegisterShaderPack(AppShader Shader, const char* pixelShaderName, const char* vertexShaderName, const char* packName, std::vector<RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>>& params);

	std::string BuildShaderPath(const char* ShaderName)
	{
		std::string Path = DefaultResourcePath;

		Path += "/shaders/";
		Path += ShaderName;

		return std::move(Path);
	}
	std::string BuildDCPath(const char* DCName)
	{
		std::string Path = DefaultResourcePath;

		Path += "/DC/";
		Path += DCName;

		return std::move(Path);
	}
private:

	D3D11_VIEWPORT							vp;

	bool LoadShaderPack(const AppShader Shader) {
		if (!registeredShaders[(int)Shader].Get() || !registeredShaders[(int)Shader]->IsValid) {
			Message("No registered shader type [%d]", (int)Shader);
			return false;
		}

		auto RShader = registeredShaders[(int)Shader];

		TRef<D3DShaderPack> NewShader = D3DShaderPack::Create(RShader.Get());
		if (!NewShader.Get()) {
			Message("Failed to load shader pack [%s]", registeredShaders[(int)Shader]->Name.c_str());
			return false;
		}

		activeShaders[(int)Shader] = NewShader;

		return true;
	}

	TRef<D3DShaderPack>						activeShaders[(int)AppShader::MAX];
	TRef<RegisteredShader>					registeredShaders[(int)AppShader::MAX];

	void SetDefaultRT();
	void SetDefViewport();
};

extern ID3D11Device* GetD3DDevice();
extern ID3D11DeviceContext* GetD3DDeviceContext();

//Globals
extern D3DDriver GD3DDriver;

const D3DXVECTOR3 UNIT_X = { 1.0f,0.0f,0.0f };
const D3DXVECTOR3 UNIT_Y = { 0.0f,1.0f,0.0f };
const D3DXVECTOR3 UNIT_Z = { 0.0f,0.0f,1.0f };

float AngleBetweenNormals(D3DXVECTOR4 v1, D3DXVECTOR4 v2);

inline D3DXVECTOR3 operator+(D3DXVECTOR3& left, D3DXVECTOR4& right) {
	return {
		left.x = right.x,
		left.y = right.y,
		left.z = right.z
	};
}
