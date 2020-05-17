#include "App.h"

#pragma region KEY_STAGE

#define KEY_W 0
#define KEY_A 1
#define KEY_S 2
#define KEY_D 3
#define KEY_Q 4
#define KEY_E 5
#define KEY_ESC 6
#define KEYS_COUNT 7

struct _keyState {
	SHORT keys[KEYS_COUNT];
} keysState1, keysState2;

void GetMousePosition(POINT* p) {
	GetCursorPos(p);
	ScreenToClient(GApp.GetWindowHandle(), p);
}
void UpdateKeyState(_keyState* state) {
	state->keys[KEY_W] = GetKeyState(0x57) & 0x8000;	//W
	state->keys[KEY_A] = GetKeyState(0x41) & 0x8000;	//A
	state->keys[KEY_S] = GetKeyState(0x53) & 0x8000;	//S
	state->keys[KEY_D] = GetKeyState(0x44) & 0x8000;	//D
	state->keys[KEY_Q] = GetKeyState(0x51) & 0x8000;	//Q
	state->keys[KEY_E] = GetKeyState(0x45) & 0x8000;	//E
	state->keys[KEY_ESC] = GetKeyState(VK_ESCAPE) & 0x8000;	//ESC
}
bool KeyPressed(SHORT vk_key) {
	return keysState1.keys[vk_key] && !keysState2.keys[vk_key];
}
bool KeyDown(SHORT vk_key) {
	return keysState1.keys[vk_key] && keysState2.keys[vk_key];
}
bool KeyUp(SHORT vk_key) {
	return !keysState1.keys[vk_key] && !keysState2.keys[vk_key];
}

#pragma endregion

bool Scene::Initialize()
{
	if (!Camera.Initialize()) {
		return 1;
	}

	Close();

	if (!GGUIManager.RegisterUI<UIElementType::Scene>(this)) {
		return 1;
	}

	if (!Light.Initialize(GetD3DDevice())) {
		return 4;
	}

	if (!SceneGrid.Intialize()) {
		return 3;
	}

	if (!AABB.Intialize()) {
		return 4;
	}

	IsValid = TRUE;
	return 0;
}

void Scene::Update(float dt)
{
	Camera.Update(dt);

	Light.Data.Eye.x = Camera.Position.x;
	Light.Data.Eye.y = Camera.Position.y;
	Light.Data.Eye.z = Camera.Position.z;

	Light.NeedsUpdate = TRUE;
}

void Scene::Begin()
{
	auto* Context = GetD3DDeviceContext();

	Camera.SetCameraBuffer(Context);

	if (Light.NeedsUpdate) {
		Light.UpdateValue(Context);
	}

	Context->VSSetConstantBuffers(1, 1, Light.Buffer.GetSelfPtr());
}

void Scene::End()
{
	Camera.End(GetD3DDeviceContext());
}

void Scene::Render(float dt)
{
	Begin();

	auto* Context = GetD3DDeviceContext();

	SceneGrid.Render(Context);

	if (AABB.Visible) {
		AABB.Render(Context);
	}

	End();
}

void Scene::RenderUI()
{
	ImGui::Begin("Scene", &bDisplay);
	if (!IsValid) {
		ImGui::TextColored(ERROR_COLOR, "Scene failed to initialize!");
		ImGui::End();
		return;
	}

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Scene"))
		{
			UINT fps = (UINT)(1.0f / GApp.GetTimer().Elapsed);
			ImGui::TextColored(INFO_COLOR, "FPS:%d", fps);
			ImGui::Separator();

			ImGui::TextColored(INFO_COLOR, "Scene Light");
			if (ImGui::SliderFloat("Ambient", &Light.Data.Ambient, 0.001f, 5.0f)) { Light.NeedsUpdate = TRUE; }
			if (ImGui::SliderFloat("Diffuse", &Light.Data.Diffuse, 0.001f, 5.0f)) { Light.NeedsUpdate = TRUE; }
			if (ImGui::SliderFloat("Specular", &Light.Data.Specular, 0.001f, 5.0f)) { Light.NeedsUpdate = TRUE; }
			if (ImGui::SliderFloat("Shininess", &Light.Data.Shininess, 0.001f, 5.0f)) { Light.NeedsUpdate = TRUE; }

			if (ImGui::Button("Move Light origin To Current Position")) {
				auto Temp = Camera.Position * -1;

				Light.Data.Direction.x = Temp.x;
				Light.Data.Direction.y = Temp.y;
				Light.Data.Direction.z = Temp.z;

				D3DXVec4Normalize(&Light.Data.Direction, &Light.Data.Direction);
			}

			ImGui::ColorPicker3("Ambient Light Color", (float*)&Light.Data.AmbientLight);
			ImGui::ColorPicker3("Light Color", (float*)&Light.Data.Color);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Camera"))
		{
			ImGui::TextColored(INFO_COLOR, "Position:");
			ImGui::Text("x:%.2f y:%.2f z:%.2f", Camera.Position.x, Camera.Position.y, Camera.Position.z);
			ImGui::TextColored(INFO_COLOR, "Rotation:");
			ImGui::Text("x:%.2f y:%.2f z:%.2f", Camera.Rotation.x, Camera.Rotation.y, Camera.Rotation.z);
			ImGui::TextColored(INFO_COLOR, "LookAt:");
			ImGui::Text("x:%.2f y:%.2f z:%.2f", Camera.LookAt.x, Camera.LookAt.y, Camera.LookAt.z);
			ImGui::TextColored(INFO_COLOR, "Mouse:");

			float deltaX, deltaY;
			deltaX = Camera.Point2.x - Camera.Point1.x;
			deltaY = Camera.Point2.y - Camera.Point1.y;

			ImGui::Text("DeltaX:%.2f DeltaY:%.2f", deltaX, deltaY);
			ImGui::Separator();
			ImGui::SliderFloat("Speed", &Camera.Speed, 10.0f, 5000.0f);

			if (ImGui::SliderAngle("FOV", &Camera.Angle, 20.0f, 120.0f)) {
				Camera.UpdateFOV();
			}

			ImGui::SliderFloat("Sensivity", &Camera.Sensivity, 0.1f, 50.0f);

			if (ImGui::Button("Go To Origin")) {
				Camera.Position = { 100.0f, 100.0f, 100.0f };
				Camera.LookAt = { 0.0f, 0.0f, 0.0f };

				Camera.UpdateLookAt();
			}

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Misc"))
		{
			if (ImGui::ColorPicker3("AABB Color", (float*)&AABB.Color)) {
				AABB.UpdateColor();
			}

			ImGui::ColorPicker3("Clear Color", (float*)&ClearColor);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

//Camera
bool Camera::Initialize()
{
	auto Viewport = GD3DDriver.GetViewport();

	D3DXMatrixPerspectiveFovLH(&Projection, Angle, Viewport.Width / Viewport.Height, Nearest, Farthest);
	D3DXMatrixIdentity(&World);

	UpdateCMBuffer();

	D3D11_BUFFER_DESC bDesc;
	memset(&bDesc, 0, sizeof(D3D11_BUFFER_DESC));
	bDesc.Usage = D3D11_USAGE_DYNAMIC;
	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bDesc.ByteWidth = sizeof(CMBuffer);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT result = GetD3DDevice()->CreateBuffer(&bDesc, NULL, MatricesBuffer.GetSelfPtr());
	if (FAILED(result)) {
		Message("Failed to create camera buffer");
		return false;
	}

	return true;
}

void Camera::UpdateCMBuffer()
{
	D3DXMatrixTranspose(&CMBuffer.ProjMatrix, &Projection);
	D3DXMatrixTranspose(&CMBuffer.WorldMatrix, &World);
	D3DXMatrixTranspose(&CMBuffer.ViewMatrix, &View);
}

void Camera::UpdateLookAt()
{
	float yaw, pitch, roll;

	yaw = Rotation.y * 0.0174532925f;
	pitch = Rotation.x * 0.0174532925f;
	roll = Rotation.z * 0.0174532925f;

	LookAt = { 0.0f,0.0f ,1.0f };
	Up = { 0.0f, 1.0f ,0.0f };

	D3DXMatrixRotationYawPitchRoll(&RotationMatrix, yaw, pitch, roll);

	D3DXVec3TransformCoord(&LookAt, &LookAt, &RotationMatrix);
	D3DXVec3TransformCoord(&Up, &Up, &RotationMatrix);

	LookAt = Position + LookAt;

	D3DXMatrixLookAtLH(&View, &Position, &LookAt, &Up);
}

void Camera::OnMove()
{
}

void Camera::OnRotate()
{

}

void Camera::Update(float dt) {
	if (!GApp.HasFocus()) {
		return;
	}

	const POINT IdleMousePos = { 100, 100 };
	auto Viewport = GD3DDriver.GetViewport();

	//Update mouse state
	Point1.x = Point2.x;
	Point1.y = Point2.y;
	GetCursorPos(&Point2);

	//ScreenToClient(GApp.GetWindowHandle(), &Point2);

	for (size_t i = 0; i < KEYS_COUNT; i++) {
		keysState1.keys[i] = keysState2.keys[i];
	}
	UpdateKeyState(&keysState2);

	if (KeyPressed(KEY_ESC)) {
		if (!Enabled) {
			Enabled = TRUE;

			LastMousePos = Point2;

			Point1.x = Point2.x = IdleMousePos.x;
			Point1.y = Point2.y = IdleMousePos.y;
			SetCursorPos(IdleMousePos.x, IdleMousePos.y);
		}
		else {
			SetCursorPos(LastMousePos.x, LastMousePos.y);

			Enabled = FALSE;
		}

		ImGui::GetIO().MouseDrawCursor = Enabled == FALSE;
		ShowCursor(FALSE);
		return;
	}

	if (!Enabled) {
		return;
	}

	D3DXVECTOR3 moveVec = { 0.0f,0.0f,0.0f };

	bool Moved = false;
	if (KeyDown(KEY_W)) {
		MoveForward(dt);
		Moved = true;
	}
	if (KeyDown(KEY_S)) {
		MoveBackward(dt);
		Moved = true;
	}
	if (KeyDown(KEY_D)) {
		MoveRight(dt);
		Moved = true;
	}
	if (KeyDown(KEY_A)) {
		MoveLeft(dt);
		Moved = true;
	}
	if (KeyDown(KEY_Q)) {
		MoveUpward(dt);
		Moved = true;
	}
	if (KeyDown(KEY_E)) {
		MoveDownward(dt);
		Moved = true;
	}

	if (Moved) {
		UpdateLookAt();
	}

	SetCursorPos(IdleMousePos.x, IdleMousePos.y);
#define Y_ANGLE 75.0f

	float deltaX, deltaY;
	deltaX = Point2.x - IdleMousePos.x;
	deltaY = Point2.y - IdleMousePos.y;

	if (std::fabs(deltaX) >= 1.0f || std::fabs(deltaY) >= 1.0f) {
		/*	deltaX = deltaX < 0.0f ? -1.0f : 1.0f;
			deltaY = deltaY < 0.0f ? -1.0f : 1.0f;*/

		float rot_y = (deltaX * dt * 5) * Sensivity;
		float rot_x = (deltaY * dt * 5) * Sensivity;

		//if (Point3.y < -degreesToRadians(Y_ANGLE)) {
		//	Point3.y = Point3.y - (Point3.y - degreesToRadians(-Y_ANGLE));
		//}

		//if (Point3.y > degreesToRadians(Y_ANGLE)) {
		//	Point3.y = Point3.y - (Point3.y - degreesToRadians(Y_ANGLE));
		//}

		//Rotation.y = Clampf(Point3.y, degreesToRadians(-Y_ANGLE), degreesToRadians(Y_ANGLE));
		//Rotation.z = 0.0f;
		//Rotation.x = /*WrapAngle(*/Point3.x/*)*/;

		Rotation.y = WrapAngle(Rotation.y + rot_y);
		Rotation.z = 0.0f;
		Rotation.x = Clampf(Rotation.x + rot_x, -Y_ANGLE, Y_ANGLE);

		UpdateLookAt();
	}

}

void Camera::SetCameraBuffer(ID3D11DeviceContext* D3DContext)
{
	UpdateCMBuffer();

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	D3DContext->Map(MatricesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	Camera::_CMBuffer* mDataPtr = (Camera::_CMBuffer*)mappedResource.pData;

	mDataPtr->WorldMatrix = CMBuffer.WorldMatrix;
	mDataPtr->ViewMatrix = CMBuffer.ViewMatrix;
	mDataPtr->ProjMatrix = CMBuffer.ProjMatrix;

	D3DContext->Unmap(MatricesBuffer.Get(), 0);

	D3DContext->VSSetConstantBuffers(0, 1, MatricesBuffer.GetSelfPtr());
}

void Camera::End(ID3D11DeviceContext* D3DContext)
{
	D3DContext->VSSetConstantBuffers(0, 0, nullptr);
}

void Camera::UpdateFOV()
{
	auto Viewport = GD3DDriver.GetViewport();
	D3DXMatrixPerspectiveFovLH(&Projection, Angle, Viewport.Width / Viewport.Height, Nearest, Farthest);
}

//Scene grid
bool SceneGrid::GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<UINT16>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format)
{
	const INT GridCellWidth = (INT)std::floor(Width / Divisions);
	const INT GridCellHeight = (INT)std::floor(Height / Divisions);

	const INT VerticesX = Divisions;
	const INT VerticesY = Divisions;
	const INT NumVertices = (VerticesX) * (VerticesY);

	const D3DXVECTOR3 GridTopLeft = { Position.x - Width / 2, Position.y - Height / 2, 0.0f };

	OutVertices.reserve(NumVertices);
	OutIndices.reserve(NumVertices);

	INT Index = 0;
	for (INT y = 0; y < VerticesY; y++)
	{
		for (INT x = 0; x < VerticesX; x++)
		{
			//const INT Index = (y * VerticesX) + x;

			D3DColorVertex v;
			v.position.x = GridTopLeft.x + (x * GridCellWidth);
			v.position.z = GridTopLeft.y + (y * GridCellHeight);
			v.position.y = 0.0f;
			v.color = Color;

			OutVertices.push_back(v);
			OutIndices.push_back(Index++);
		}
	}

	//OutVertices.push_back({
	//				{
	//					GridTopLeft.x,
	//					GridTopLeft.y ,
	//					0.0f
	//				}, Color });
	//OutVertices.push_back({
	//				{
	//					GridTopLeft.x + 50,
	//					GridTopLeft.y + 50 ,
	//					10.0f
	//				}, Color });
	//OutVertices.push_back({
	//				{
	//					GridTopLeft.x + 50,
	//					GridTopLeft.y ,
	//					100.0f
	//				}, Color });

	//OutIndices.push_back(0);
	//OutIndices.push_back(1);
	//OutIndices.push_back(2);

	//for (INT y = 0; y < VerticesY - 1; y++)
	//{
	//	for (INT x = 0; x < VerticesX - 1; x++)
	//	{
	//		INT Row1 = y * (VerticesX + 1);
	//		INT Row2 = (y + 1) * (VerticesX + 1);

	//		// triangle 1
	//		OutIndices.push_back(Row1 + x);
	//		OutIndices.push_back(Row1 + x + 1);
	//		OutIndices.push_back(Row2 + x + 1);

	//		// triangle 2
	//		OutIndices.push_back(Row1 + x);
	//		OutIndices.push_back(Row2 + x + 1);
	//		OutIndices.push_back(Row2 + x);
	//	}
	//}

	OutTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	Format = DXGI_FORMAT_R16_UINT;

	return true;
}

void SceneGrid::SetShaders(ID3D11DeviceContext* Context)
{
	auto Shader = GD3DDriver.GetShaderPack(AppShader::LineShaderPack);
	if (Shader.Get()) {
		Shader->SetShader(Context);
	}
}

//Scene AABB
bool SceneAABB::GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<UINT16>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format)
{
	OutTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	Format = DXGI_FORMAT_R16_UINT;

	D3DColorVertex leftTop1;
	leftTop1.position.x = Box.Min.X;
	leftTop1.position.y = Box.Min.Y;
	leftTop1.position.z = Box.Min.Z;
	leftTop1.color = Color;
	OutVertices.push_back(leftTop1);

	D3DColorVertex rightTop1;
	rightTop1.position.x = Box.Max.X;
	rightTop1.position.y = Box.Min.Y;
	rightTop1.position.z = Box.Min.Z;
	rightTop1.color = Color;
	OutVertices.push_back(rightTop1);

	D3DColorVertex leftBottom1;
	leftBottom1.position.x = Box.Min.X;
	leftBottom1.position.y = Box.Max.Y;
	leftBottom1.position.z = Box.Min.Z;
	leftBottom1.color = Color;
	OutVertices.push_back(leftBottom1);

	D3DColorVertex rightBottom1;
	rightBottom1.position.x = Box.Max.X;
	rightBottom1.position.y = Box.Max.Y;
	rightBottom1.position.z = Box.Min.Z;
	rightBottom1.color = Color;
	OutVertices.push_back(rightBottom1);

	D3DColorVertex leftTop2;
	leftTop2.position.x = Box.Min.X;
	leftTop2.position.y = Box.Min.Y;
	leftTop2.position.z = Box.Max.Z;
	leftTop2.color = Color;
	OutVertices.push_back(leftTop2);

	D3DColorVertex rightTop2;
	rightTop2.position.x = Box.Max.X;
	rightTop2.position.y = Box.Min.Y;
	rightTop2.position.z = Box.Max.Z;
	rightTop2.color = Color;
	OutVertices.push_back(rightTop2);

	D3DColorVertex leftBottom2;
	leftBottom2.position.x = Box.Min.X;
	leftBottom2.position.y = Box.Max.Y;
	leftBottom2.position.z = Box.Max.Z;
	leftBottom2.color = Color;
	OutVertices.push_back(leftBottom2);

	D3DColorVertex rightBottom2;
	rightBottom2.position.x = Box.Max.X;
	rightBottom2.position.y = Box.Max.Y;
	rightBottom2.position.z = Box.Max.Z;
	rightBottom2.color = Color;
	OutVertices.push_back(rightBottom2);

	//face 1
	OutIndices.push_back(0);
	OutIndices.push_back(1);
	OutIndices.push_back(1);
	OutIndices.push_back(3);
	OutIndices.push_back(3);
	OutIndices.push_back(2);
	OutIndices.push_back(2);
	OutIndices.push_back(0);

	//face 2
	OutIndices.push_back(4);
	OutIndices.push_back(5);
	OutIndices.push_back(5);
	OutIndices.push_back(7);
	OutIndices.push_back(7);
	OutIndices.push_back(6);
	OutIndices.push_back(6);
	OutIndices.push_back(4);

	//connections
	OutIndices.push_back(0);
	OutIndices.push_back(4);

	OutIndices.push_back(1);
	OutIndices.push_back(5);

	OutIndices.push_back(2);
	OutIndices.push_back(6);

	OutIndices.push_back(3);
	OutIndices.push_back(7);

	return true;
}

void SceneAABB::SetShaders(ID3D11DeviceContext* Context)
{
	auto Shader = GD3DDriver.GetShaderPack(AppShader::LineShaderPack);
	if (Shader.Get()) {
		Shader->SetShader(Context);
	}
}

//Globals
Scene	GScene;
