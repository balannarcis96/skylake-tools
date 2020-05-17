#include "App.h"

#include <D3DX11.h>
#include <fstream>

void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename)
{
	if (!errorMessage) {
		return;
	}

	char* compileErrors;
	unsigned long bufferSize, i;
	std::ofstream fout;

	compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();
	fout.open("shader-error.txt");
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}
	fout.close();

	errorMessage->Release();
	errorMessage = 0;
	MessageBoxA(hwnd, "Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

int D3DDriver::Initialize(int width, int height)
{
	auto hWnd = GApp.GetWindowHandle();

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	HRESULT result;

	DXGI_SWAP_CHAIN_DESC swDesc;
	ZeroMemory(&swDesc, sizeof(swDesc));
	swDesc.BufferCount = 1;
	swDesc.BufferDesc.Width = clientRect.right;
	swDesc.BufferDesc.Height = clientRect.bottom;
	swDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swDesc.BufferDesc.RefreshRate.Numerator = 0;
	swDesc.BufferDesc.RefreshRate.Denominator = 1;
	swDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swDesc.OutputWindow = hWnd;
	swDesc.SampleDesc.Count = 1;
	swDesc.SampleDesc.Quality = 0;
	swDesc.Windowed = TRUE;
	swDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	result = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swDesc,
		swapChain.GetSelfPtr(),
		device.GetSelfPtr(),
		NULL,
		context.GetSelfPtr());
	if (FAILED(result)) {
		return 1;
	}

	ID3D11Texture2D* backBuffer = nullptr;
	swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (!backBuffer) {
		return 2;
	}

	result = device->CreateRenderTargetView(backBuffer, NULL, renderTargetView.GetSelfPtr());
	if (FAILED(result)) {
		return 3;
	}

	backBuffer->Release();

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = device->CreateDepthStencilState(&dsDesc, depthStencil.GetSelfPtr());
	if (FAILED(result)) {
		return 4;
	}

	D3D11_TEXTURE2D_DESC tDesc;
	tDesc.Width = clientRect.right;
	tDesc.Height = clientRect.bottom;
	tDesc.MipLevels = 1;
	tDesc.ArraySize = 1;
	tDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 0;
	tDesc.Usage = D3D11_USAGE_DEFAULT;
	tDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tDesc.CPUAccessFlags = 0;
	tDesc.MiscFlags = 0;

	result = device->CreateTexture2D(&tDesc, NULL, depthStencilTexture.GetSelfPtr());
	if (FAILED(result)) {
		return 5;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(depthStencilTexture.Get(), &dsvDesc, depthStencilView.GetSelfPtr());
	if (FAILED(result)) {
		return 6;
	}

	SetDefaultRT();

	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	//rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = device->CreateRasterizerState(&rasterDesc, rasterizerState.GetSelfPtr());
	if (FAILED(result)) {
		return 7;
	}

	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;
	vp.Width = clientRect.right;
	vp.Height = clientRect.bottom;
	SetDefViewport();

	D3D11_BLEND_DESC blDesc;
	ZeroMemory(&blDesc, sizeof(blDesc));
	blDesc.RenderTarget[0].BlendEnable = TRUE;
	blDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	result = device->CreateBlendState(&blDesc, alphaBlend.GetSelfPtr());
	if (FAILED(result)) {
		return 8;
	}

	blDesc.RenderTarget[0].BlendEnable = FALSE;
	result = device->CreateBlendState(&blDesc, disabledAlphaBlend.GetSelfPtr());
	if (FAILED(result)) {
		return 8;
	}

	return 0;
}

bool D3DDriver::RegisterShaderPack(AppShader Shader, const char* pixelShaderName, const char* vertexShaderName, const char* packName, std::vector<RawDataWrapper<D3D11_INPUT_ELEMENT_DESC>>& params)
{
	if (registeredShaders[(int)Shader].Get())
	{
		Message("ShaderResource already registered type[%d] name[%s]", (int)Shader, registeredShaders[(int)Shader]->Name.c_str());
		return false;
	}

	auto NewPack = std::make_unique<RegisteredShader>();

	NewPack->PShaderFileName = GConfigTool.BuildShaderPath(pixelShaderName);
	NewPack->VShaderFileName = GConfigTool.BuildShaderPath(vertexShaderName);
	NewPack->Name = packName;
	NewPack->InputElementDesc = std::move(params);
	registeredShaders[(int)Shader] = NewPack.release();

	return true;
}

void D3DDriver::SetDefaultRT()
{
	context->OMSetDepthStencilState(depthStencil.Get(), 1);
	context->OMSetRenderTargets(1, renderTargetView.GetSelfPtr(), depthStencilView.Get());
}

void D3DDriver::SetDefViewport()
{
	context->RSSetState(rasterizerState.Get());
	context->RSSetViewports(1, &vp);
}

TRef<D3DShaderPack> D3DShaderPack::Create(RegisteredShader* rShader)
{
	auto ShaderPack = std::make_unique<D3DShaderPack>();

	HRESULT result;
	ID3D10Blob* errorBlob;
	ID3D10Blob* shaderBlob;
	const char* pShaderFile = rShader->PShaderFileName.c_str();
	const char* vShaderFile = rShader->VShaderFileName.c_str();

	result = D3DX11CompileFromFileA(pShaderFile, NULL, NULL, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &shaderBlob, &errorBlob, NULL);
	if (FAILED(result)) {
		OutputShaderErrorMessage(errorBlob, GetActiveWindow(), pShaderFile);
		return nullptr;
	}

	result = GD3DDriver.device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, ShaderPack->pixelShader.GetSelfPtr());
	if (FAILED(result)) {
		return nullptr;
	}

	shaderBlob->Release();

	result = D3DX11CompileFromFileA(vShaderFile, NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &shaderBlob, &errorBlob, NULL);
	if (FAILED(result)) {
		OutputShaderErrorMessage(errorBlob, GetActiveWindow(), vShaderFile);
		return nullptr;
	}

	result = GD3DDriver.device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, ShaderPack->vertexShader.GetSelfPtr());
	if (FAILED(result)) {
		shaderBlob->Release();
		return nullptr;
	}

	UINT numElements = (UINT)rShader->InputElementDesc.size();

	result = GD3DDriver.device->CreateInputLayout((const D3D11_INPUT_ELEMENT_DESC*)rShader->InputElementDesc.data(), numElements, shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), ShaderPack->inputLayout.GetSelfPtr());
	if (FAILED(result)) {
		shaderBlob->Release();
		return nullptr;
	}

	shaderBlob->Release();

	ShaderPack->source = rShader;

	return ShaderPack.release();
}

//Globals
D3DDriver GD3DDriver;

ID3D11Device* GetD3DDevice() { return GD3DDriver.device.Get(); }

ID3D11DeviceContext* GetD3DDeviceContext() { return GD3DDriver.context.Get(); }

float AngleBetweenNormals(D3DXVECTOR4 v1, D3DXVECTOR4 v2)
{
	D3DXVec3Normalize((D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v1);
	D3DXVec3Normalize((D3DXVECTOR3*)&v2, (D3DXVECTOR3*)&v2);

	float dot = D3DXVec3Dot((D3DXVECTOR3*)&v1, (D3DXVECTOR3*)&v2);
	v1 = {
		v1.x * v2.x,
		v1.y * v2.y,
		v1.z * v2.z,
		1.0f
	};

	dot /= (D3DXVec3Length((D3DXVECTOR3*)&v1));

	return dot;
}
