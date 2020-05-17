#include "Actor.h"
#include "FPropertyTag.h"
#include "FVector.h"
#include "MeshComponent.h"
#include "StaticMesh.h"
#include "D3D.h"
#include "FRotator.h"

#include "FGlobalMath.h"
FGlobalMath GMath;

D3DXMATRIX FRotatorToD3DXMATRIX(const FRotator& rotator) {
	D3DXVECTOR3 zero = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	float M[4][4];

	memset(M, 0, sizeof(4 * 4 * sizeof(float)));

	const FLOAT	SR = GMath.SinTab(rotator.roll);
	const FLOAT	SP = GMath.SinTab(rotator.pitch);
	const FLOAT	SY = GMath.SinTab(rotator.yaw);
	const FLOAT	CR = GMath.CosTab(rotator.roll);
	const FLOAT	CP = GMath.CosTab(rotator.pitch);
	const FLOAT	CY = GMath.CosTab(rotator.yaw);

	M[0][0] = CP * CY;
	M[0][1] = CP * SY;
	M[0][2] = SP;
	M[0][3] = 0.f;

	M[1][0] = SR * SP * CY - CR * SY;
	M[1][1] = SR * SP * SY + CR * CY;
	M[1][2] = -SR * CP;
	M[1][3] = 0.f;

	M[2][0] = -(CR * SP * CY + SR * SY);
	M[2][1] = CY * SR - CR * SP * SY;
	M[2][2] = CR * CP;
	M[2][3] = 0.f;

	M[3][0] = zero.x;
	M[3][1] = zero.y;
	M[3][2] = zero.z;
	M[3][3] = 1.f;

	return D3DXMATRIX((const float*)M);
}

const D3DXMATRIX Actor::LocalToWorld() {
	D3DXMATRIX Result;
	D3DXMatrixIdentity(&Result);

	D3DXMATRIX ActorRot;
	D3DXMATRIX ActorScale;
	D3DXMATRIX ActorTrans;

	D3DXMATRIX ComponentRot;
	D3DXMATRIX ComponentScale;
	D3DXMATRIX ComponentTrans;

	//ACTOR
	const Vector3 aPos = absolutePosition();
	D3DXMatrixTranslation(&ActorTrans, vPosition.v[0], vPosition.v[1], vPosition.v[2]);

	//const auto aRotE = absoluteRotation();
	//FRotator aRot = absoluteRotator();
	FRotator aRot = FRotator(0, 0, 0);
	FPropertyTag* prop = GetPropByName("Rotation");
	if (prop && prop->value)
	{
		aRot = *(FRotator*)prop->value;
	}

	ActorRot = FRotatorToD3DXMATRIX(aRot);

	const Vector3 aDS3D = absoluteDrawScale3D();
	const float aDS = absoluteDrawScale();
	const float DrawScale = fDrawScale;
	D3DXMatrixScaling(&ActorScale, vDrawScale3D.v[0] * aDS, vDrawScale3D.v[1] * aDS, vDrawScale3D.v[2] * aDS);

	//COMPONENT
	const Vector3 cPos = component->translation();

	const FRotator cRto = component->rotator();
	ComponentRot = FRotatorToD3DXMATRIX(cRto);
	//const auto cRtoE = component->rotation();
	const auto cDS3D = component->scale3D();
	const auto cDS = component->scale();


	//aRot.pitch = cRto.pitch;
	//aRot.roll = cRto.roll;
	//aRot.yaw = cRto.yaw;

	// 0. Rotate Component
	D3DXMatrixMultiply(&Result, &Result, &ComponentRot);
	// 1. Scale
	D3DXMatrixMultiply(&Result, &Result, &ActorScale);
	// 2. Rotate
	D3DXMatrixMultiply(&Result, &Result, &ActorRot);
	// 3. Translate
	D3DXMatrixMultiply(&Result, &Result, &ActorTrans);

#if 1
	/*
		FTranslationMatrix	LToW		( -PrePivot					);
		FScaleMatrix		ActorScale	( DrawScale3D * DrawScale	);
		FRotationMatrix		ActorRot		( Rotation					);
		FTranslationMatrix	ActorTrans	( Location					);
		LToW *= ActorScale;
		LToW *= ActorRot;
		LToW *= ActorTrans;
		return LToW;
	*/




#else
	const FLOAT
		SR = GMath.SinTab(Rotation.roll),
		SP = GMath.SinTab(Rotation.pitch),
		SY = GMath.SinTab(Rotation.yaw),
		CR = GMath.CosTab(Rotation.roll),
		CP = GMath.CosTab(Rotation.pitch),
		CY = GMath.CosTab(Rotation.yaw);

	const FLOAT
		LX = Location.x,
		LY = Location.y,
		LZ = Location.z,
		PX = 0,	   // no prepivot 
		PY = 0,	   // no prepivot 
		PZ = 0;	   // no prepivot 

	const FLOAT	DX = DrawScale3D.x * DrawScale,
		DY = DrawScale3D.y * DrawScale,
		DZ = DrawScale3D.z * DrawScale;

	Result.m[0][0] = CP * CY * DX;
	Result.m[0][1] = CP * DX * SY;
	Result.m[0][2] = DX * SP;
	Result.m[0][3] = 0.f;

	Result.m[1][0] = DY * (CY * SP * SR - CR * SY);
	Result.m[1][1] = DY * (CR * CY + SP * SR * SY);
	Result.m[1][2] = -CP * DY * SR;
	Result.m[1][3] = 0.f;

	Result.m[2][0] = -DZ * (CR * CY * SP + SR * SY);
	Result.m[2][1] = DZ * (CY * SR - CR * SP * SY);
	Result.m[2][2] = CP * CR * DZ;
	Result.m[2][3] = 0.f;

	Result.m[3][0] = LX - CP * CY * DX * PX + CR * CY * DZ * PZ * SP - CY * DY * PY * SP * SR + CR * DY * PY * SY + DZ * PZ * SR * SY;
	Result.m[3][1] = LY - (CR * CY * DY * PY + CY * DZ * PZ * SR + CP * DX * PX * SY - CR * DZ * PZ * SP * SY + DY * PY * SP * SR * SY);
	Result.m[3][2] = LZ - (CP * CR * DZ * PZ + DX * PX * SP - CP * DY * PY * SR);
	Result.m[3][3] = 1.f;

#endif
	return Result;
}

void Actor::ReadData(FIStream& data) {
	UObject::ReadData(data);

	FPropertyTag* prop = GetPropByName("Location");
	if (!prop)
	{
		vPosition = { 0.0f,0.0f ,0.0f };
	}
	else
	{
		FVector3* fVP = (FVector3*)prop->value;
		vPosition = { fVP->x,fVP->y,fVP->z };
	}

	prop = GetPropByName("DrawScale3D");
	if (!prop)
	{
		vDrawScale3D = { 1.0f,1.0f ,1.0f };
	}
	else
	{
		FVector3* fVP = (FVector3*)prop->value;
		vDrawScale3D = { fVP->x,fVP->y,fVP->z };
	}

	prop = GetPropByName("Rotation");
	if (!prop)
	{
		vRotation = { 0.0f,0.0f ,0.0f };
	}
	else
	{
		Vector3 fVP = ((FRotator*)prop->value)->euler();
		vRotation = { fVP.v[0] ,fVP.v[1],fVP.v[2] };
	}

	prop = GetPropByName("DrawScale");
	if (!prop)
	{
		fDrawScale = 1.0f;
	}
	else
	{
		fDrawScale = *(float*)prop->value;
	}
}

const char* Actor::GetDisplayName() const {
	return Name();
}

const FRotator* Actor::Rotator() const {
	FPropertyTag* prop = GetPropByName("Rotation");
	if (!prop || !prop->value)
	{
		return nullptr;
	}

	return (FRotator*)prop->value;
}

const float Actor::absoluteDrawScale() const {
	if (!component)
	{
		return 1.0f;
	}

	float cScale = component->scale();
	return fDrawScale * cScale;
}

const Vector3 Actor::absolutePosition() const {
	if (!component)
	{
		return{ 0.0f,0.0f ,0.0f };
	}

	Vector3 cPos = component->translation();
	return{ vPosition.v[0] + cPos.v[0], vPosition.v[1] + cPos.v[1] ,vPosition.v[2] + cPos.v[2] };
}

const Vector3 Actor::absoluteDrawScale3D() const {
	if (!component)
	{
		return{ 1.0f,1.0f,1.0f };
	}

	Vector3 cScale3D = component->scale3D();
	return{ vDrawScale3D.v[0] * cScale3D.v[0], vDrawScale3D.v[1] * cScale3D.v[1] ,vDrawScale3D.v[2] * cScale3D.v[2] };
}

const Vector3 Actor::absoluteRotation() const {
	if (!component)
	{
		return{ 0.0f,0.0f ,0.0f };
	}

	Vector3 cRot = component->rotation();
	return{ vRotation.v[0] + cRot.v[0], vRotation.v[1] + cRot.v[1] ,vRotation.v[2] + cRot.v[2] };
}

const Vector3 Actor::absoluteSCNRotation() const {
	Vector3 res = {
		(float)-degreesToRadians(vRotation.v[1]),
		(float)-degreesToRadians(vRotation.v[2]),
		(float)degreesToRadians(vRotation.v[0])
	};

	if (!component)
	{
		return{ res.v[0],res.v[1],res.v[2] };
	}

	Vector3 cRot = component->rotation();
	return{
		res.v[0] + (float)-degreesToRadians(cRot.v[1]),
		res.v[1] + (float)-degreesToRadians(cRot.v[2]),
		res.v[2] + (float)degreesToRadians(cRot.v[0]),
	};
}

const FRotator Actor::absoluteRotator() const {
	if (!component)
	{
		return FRotator{ 0, 0 ,0 };
	}

	const FRotator cRot = component->rotator();

	FPropertyTag* prop = GetPropByName("Rotation");
	if (!prop || !prop->value)
	{
		return cRot;
	}

	FRotator aRot = (*(FRotator*)prop->value);
	return cRot + aRot;
}

void StaticMeshActor::ReadData(FIStream& data) {
	Actor::ReadData(data);

	FPropertyTag* prop = GetPropByName("StaticMeshComponent");
	if (prop)
	{
		component = (StaticMeshComponent*)prop->value;
	}

	if (component && prop)
	{
		component->ReadObject(false);

		StaticMeshComponent* smComponent = (StaticMeshComponent*)component;
		if (smComponent)
		{
			mesh = (StaticMesh*)((StaticMeshComponent*)component)->mesh;
			if (mesh)
			{
				if (strstr(mesh->Name(), "Bush") ||
					strstr(mesh->Name(), "Plant") ||
					strstr(mesh->Name(), "Flower"))
				{
					mesh = nullptr;
				}
				else if (strstr(mesh->Name(), "RoadLamp"))
				{
					mesh = (StaticMesh*)((StaticMeshComponent*)component)->mesh;
				}
			}
		}
		else
		{
			mesh = nullptr;
		}
	}
	else
	{
		mesh = nullptr;
	}

	if (!mesh || mesh->lodInfoCount == 0)
	{
		vBuffer = nullptr;
		vertices = nullptr;
		return;
	}

	//SET MATRICES HERE

	if (strstr(mesh->Name(), "Wall"))
	{
		mesh = mesh;
	}

	D3DXMATRIX LToW = LocalToWorld();

	//
	if (!mesh)
	{
		return;
	}

	if (mesh->lodInfoCount) {
		vertexCount = mesh->lodInfo[0].numVertices;
		if (vertexCount)
		{
			vertices = new D3DVertex[vertexCount];
			int32 size = vertexCount * sizeof(D3DVertex);
			memcpy_s(vertices, size, mesh->lodInfo[0].vertices, size);
		}
		else
		{
			vertices = nullptr;
		}
	}
	else
	{
		vertices = nullptr;
	}

	//transform new vertices here

	/*auto aRotator = absoluteRotator();
	auto eRotation = aRotator.euler();*/

	//D3DXMATRIX rotMatrixX;
	//D3DXMATRIX rotMatrixY;
	//D3DXMATRIX rotMatrixZ;
	//D3DXMatrixRotationX(&rotMatrixX, eRotation[0]);
	//D3DXMatrixRotationY(&rotMatrixY, eRotation[1]);
	//D3DXMatrixRotationZ(&rotMatrixZ, eRotation[2]);
	////
	//D3DXMATRIX rotMatrix;

	//D3DXMatrixRotationX(&rotMatrix, degreesToRadians(-90));

	D3DXVECTOR4 out_t;
	for (size_t i = 0; i < vertexCount; i++)
	{
		//LocalToWorld
		D3DXVec3Transform(&out_t, &vertices[i].position, &LToW);

		//D3DXVec3Transform(&out_t, (D3DXVECTOR3*)&out_t, &rotMatrix);

		vertices[i].position = {
			out_t.x, //x
			out_t.z, //y
			-out_t.y //z
		};

		//min.x = fmin(vertices[i].position.x, min.x);
		//min.y = fmin(vertices[i].position.y, min.y);
		//min.z = fmin(vertices[i].position.z, min.z);

		//max.x = fmax(vertices[i].position.x, max.x);
		//max.y = fmax(vertices[i].position.y, max.y);
		//max.z = fmax(vertices[i].position.z, max.z);
	}
	////

	//_D3DDriver& driver = GetD3DDriver();

	auto* device = GetD3DDevice();

	D3D11_BUFFER_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(D3DVertex) * vertexCount;
	bDesc.StructureByteStride = 0;
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA bData;
	bData.pSysMem = vertices;
	bData.SysMemPitch = 0;
	bData.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&bDesc, &bData, &vBuffer);
	if (FAILED(result))
	{
		throw std::exception("Failed to create vertex buffer");
	}

	if (mesh)
	{
		package->actorsMeshees.push_back(this);
	}
}

void StaticMeshActor::Render() {
	if (!mesh || mesh->lodInfoCount == 0)
	{
		return;
	}

	UINT offset = 0;
	UINT stride = sizeof(D3DVertex);

	auto* deviceContext = GetD3DDeviceContext();

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);

	mesh->Render(false);
}

StaticMeshActor::StaticMeshActor() {
	vBuffer = nullptr;
	vertices = nullptr;
}

StaticMeshActor::~StaticMeshActor() {
	if (vBuffer)
	{
		vBuffer->Release();
		vBuffer = nullptr;
	}

	if (vertices)
	{
		delete[] vertices;
		vertices = nullptr;
	}
}

void InterpActor::ReadData(FIStream& data) {
	Actor::ReadData(data);

	FPropertyTag* prop = GetPropByName("SkeletalMeshComponent");
	if (prop)
	{
		component = (ActorComponent*)package->uObjectForIndex(*(int32*)prop->value);
	}
	else
	{
		prop = GetPropByName("StaticMeshComponent");
		if (prop)
		{
			component = (ActorComponent*)package->uObjectForIndex(*(int32*)prop->value);
		}
	}

	if (component)
	{
		component->ReadData(data);
	}
}

void SkeletalMeshActor::ReadData(FIStream& data) {
	Actor::ReadData(data);

	FPropertyTag* prop = GetPropByName("SkeletalMeshComponent");
	if (prop)
	{
		component = (SkeletalMeshComponent*)package->uObjectForIndex(*(int32*)prop->value);
	}

	if (component)
	{
		component->ReadData(data);
	}
}

void LightActor::ReadData(FIStream& data) {
	Actor::ReadData(data);

	FPropertyTag* prop = GetPropByName("LightComponent");
	if (prop)
	{
		component = (LightComponent*)package->uObjectForIndex(*(int32*)prop->value);
	}

	if (component)
	{
		component->ReadData(data);
	}
}

void PointLight::ReadData(FIStream& data) {
	LightActor::ReadData(data);
}

void SpotLight::ReadData(FIStream& data) {
	LightActor::ReadData(data);
}
