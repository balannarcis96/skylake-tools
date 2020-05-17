#pragma once

struct Camera {
	struct _CMBuffer
	{
		D3DXMATRIX		WorldMatrix;
		D3DXMATRIX		ViewMatrix;
		D3DXMATRIX		ProjMatrix;
	};

	bool Initialize();
	void Update(float dt);
	void SetCameraBuffer(ID3D11DeviceContext* D3DContext);
	void End(ID3D11DeviceContext* D3DContext);
	void UpdateFOV();

	inline void MoveForward(float dt) {
		float tSpeed = Speed * dt;
		float Radians = Rotation.y * 0.0174532925f;

		Position.x += sinf(Radians) * tSpeed;
		Position.z += cosf(Radians) * tSpeed;
	}
	inline void MoveBackward(float dt) {
		float tSpeed = Speed * dt;
		float Radians = Rotation.y * 0.0174532925f;

		Position.x -= sinf(Radians) * tSpeed;
		Position.z -= cosf(Radians) * tSpeed;
	}
	inline void MoveLeft(float dt) {
		float tSpeed = Speed * dt;
		float Radians = Rotation.y * 0.0174532925f + (__PI / 2);

		Position.x -= sinf(Radians) * tSpeed;
		Position.z -= cosf(Radians) * tSpeed;
	}
	inline void MoveRight(float dt) {
		float tSpeed = Speed * dt;
		float Radians = Rotation.y * 0.0174532925f + (__PI / 2);

		Position.x += sinf(Radians) * tSpeed;
		Position.z += cosf(Radians) * tSpeed;
	}
	inline void MoveUpward(float dt) {
		float tSpeed = Speed * dt;

		Position.y += tSpeed;
	}
	inline void MoveDownward(float dt) {
		float tSpeed = Speed * dt;

		Position.y -= tSpeed;
	}

private:
	friend struct Scene;

	void UpdateCMBuffer();
	void UpdateLookAt();
	void OnMove();
	void OnRotate();
	void RotateCamera(D3DXVECTOR3 axis, float degrees) {
		degrees = TO_RADIANS(degrees);

		D3DXMatrixRotationAxis(&RotationMatrix, &axis, degrees);

		D3DXVECTOR4 look_at_target =
		{
			LookAt.x - Position.x,
			LookAt.y - Position.y,
			LookAt.z - Position.z,
			1.0f
		};

		D3DXVECTOR4 look_at_up =
		{
			Up.x - Position.x,
			Up.y - Position.y,
			Up.z - Position.z,
			1.0f
		};

		D3DXVec3Transform(&look_at_target, (D3DXVECTOR3*)&look_at_target, &RotationMatrix);
		D3DXVec3Transform(&look_at_up, (D3DXVECTOR3*)&look_at_up, &RotationMatrix);

		LookAt =
		{
			Position.x + look_at_target.x,
			Position.y + look_at_target.y,
			Position.z + look_at_target.z
		};

		Up =
		{
			Position.x + look_at_up.x,
			Position.y + look_at_up.y,
			Position.z + look_at_up.z
		};

		D3DXMatrixLookAtLH(&View, (D3DXVECTOR3*)&Position, (D3DXVECTOR3*)&LookAt, (D3DXVECTOR3*)&Up);
	}
	void Target(D3DXVECTOR3 new_target) {
		D3DXVECTOR4 old_look_at_target = {
			LookAt.x - Position.x,
			LookAt.y - Position.y,
			LookAt.z - Position.z,
			1.0f
		};

		D3DXVECTOR4 new_look_at_target = {
			new_target.x - Position.x,
			new_target.y - Position.y,
			new_target.z - Position.z,
			1.0f
		};

		float angle = TO_DEGREES(AngleBetweenNormals(old_look_at_target, new_look_at_target));
		if (angle != 0.0f && angle != 360.0f && angle != 180.0f) {
			D3DXVECTOR3 axis;
			D3DXVec3Cross(&axis, (D3DXVECTOR3*)&old_look_at_target, (D3DXVECTOR3*)&new_look_at_target);
			RotateCamera(axis, angle);
		}

		LookAt = {
			new_look_at_target.x,
			new_look_at_target.y,
			new_look_at_target.z
		};

		D3DXMatrixLookAtLH(&View, (D3DXVECTOR3*)&Position, (D3DXVECTOR3*)&LookAt, (D3DXVECTOR3*)&Up);
	}

	BOOL					Enabled = FALSE;

	float					Farthest = 100000.0f;
	float					Nearest = 0.1f;
	float					Angle = __PI / 4;
	float					Speed = 1500.0f;
	float					Sensivity = 4.0f;
	D3DXVECTOR3				Up = { 0.0f, 1.0f , 0.0f }; //y up?
	D3DXVECTOR3				Position = { 1800.0f, 900.0f, -80.0f };
	D3DXVECTOR3				LookAt = { 1800.0f, 968.0f, -80.0f };
	D3DXVECTOR3				Rotation = { 26.0f, -89.0f, 0.0f };

	D3DXMATRIX				RotationX;
	D3DXMATRIX				RotationY;
	D3DXMATRIX				RotationMatrix;

	D3DXMATRIX				View;
	D3DXMATRIX				LockAtMatrix;
	D3DXMATRIX				Projection;
	D3DXMATRIX				World;

	_CMBuffer				CMBuffer;
	TRef<ID3D11Buffer>		MatricesBuffer = nullptr;

	//Used for mouse position calculation
	POINT					Point1, Point2, LastMousePos;
	D3DXVECTOR2				Point3;
};

struct SceneGrid :SceneMesh<D3DColorVertex, UINT16> {
	INT				Width = 1000;
	INT				Height = 1000;
	INT				Divisions = 100;
	D3DXVECTOR3		Position = { 0.0f,0.0f ,0.0f };
	D3DXVECTOR4		Color = { 0.0f, 0.0, 0.0f, 1.0f };

	SceneGrid() {}

	// Inherited via SceneMesh
	virtual bool GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<UINT16>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format) override;
	virtual void SetShaders(ID3D11DeviceContext* Context) override;
};

struct SceneLight : ShaderResource<SceneLightData> {
	SceneLight() {
		SetDefault();
	}

	void SetDefault() {
		Data.Ambient = 0.1f;
		Data.Diffuse = 0.3f;
		Data.Specular = 0.3f;
		Data.Shininess = 0.8f;
		Data.AmbientLight = { 1.0f, 1.0f, 1.0f, 1.0f };
		Data.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Data.Direction = { 1.0f, -1.0f, 1.0f, 1.0f };
	}
};

struct SceneAABB :SceneMesh<D3DColorVertex, UINT16, true> {
	SceneAABB() {
		SetDefault();
	}

	void SetHeight(float Height) {
		auto* Vertices = GetVertices();

		//Vertices[0].position.y = Height;
		//Vertices[1].position.y = Height;
		//Vertices[4].position.y = Height;
		//Vertices[5].position.y = Height;

		Vertices[2].position.y = Height;
		Vertices[3].position.y = Height;
		Vertices[6].position.y = Height;
		Vertices[7].position.y = Height;

		Touch();
	}
	void UpdateColor() {
		auto* Vertices = GetVertices();

		for (size_t i = 0; i < 8; i++)
		{
			Vertices[i].color = Color;
		}

		Touch();
	}
	void SetDefault() {
		Box = SBox({ 0.0f,0.0f,0.0f }, { 10.0f, 10.0f, 10.0f });
		Color = { 0.0f,0.0f ,0.0f ,1.0f };
	}

	virtual bool GenerateMesh(std::vector<D3DColorVertex>& OutVertices, std::vector<UINT16>& OutIndices, D3D11_PRIMITIVE_TOPOLOGY& OutTopology, DXGI_FORMAT& Format) override;
	virtual void SetShaders(ID3D11DeviceContext* Context) override;

	bool		Visible = false;
	SBox		Box;
	D3DXVECTOR4	Color;
};

struct Scene : UIElement<UIElementType::Scene> {
	bool Initialize();
	void Shutdown() {}

	void Update(float dt);
	void Render(float dt);
	void RenderUI() override;

	Camera& GetCamera()noexcept {
		return Camera;
	}

	void SetAABBVisible(bool Visible) {
		AABB.Visible = Visible;
		if (!Visible) {
			AABB.SetDefault();
		}
	}
	D3DXVECTOR4					ClearColor = CLEAR_COLOR;
	SceneAABB					AABB;

private:
	void Begin();
	void End();

	BOOL						IsValid = FALSE;
	Camera						Camera;
	SceneGrid					SceneGrid;
	SceneLight					Light;
};

extern Scene	GScene;