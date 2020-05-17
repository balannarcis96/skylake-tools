#ifndef ACTOR_H
#define ACTOR_H

#include "UObject.h"
#include "FRotator.h"
#include "FColor.h"
#include "UComponent.h"
#include "FPropertyTag.h"
#include <D3D11.h>

struct StaticMesh;

struct Actor :UObject {
	Vector3 vPosition;
	Vector3 vDrawScale3D;
	float   fDrawScale;
	Vector3 vRotation;
	ActorComponent* component;

	const char* GetDisplayName() const;
	const FRotator* Rotator() const;
	const float absoluteDrawScale() const;
	const Vector3 absolutePosition() const;
	const Vector3 absoluteDrawScale3D() const;
	const Vector3 absoluteRotation() const;
	const Vector3 absoluteSCNRotation() const;
	const FRotator absoluteRotator() const;

	const D3DXMATRIX LocalToWorld();
	void ReadData(FIStream& data);

	virtual void Render() {}
};

struct StaticMeshActor : Actor {
	StaticMesh* mesh;

	ID3D11Buffer* vBuffer;
	D3DVertex* vertices;
	uint32 vertexCount;

	void ReadData(FIStream& data);
	void Render();

	void GetMinAndMax(D3DXVECTOR3& min, D3DXVECTOR3& max) {
		for (size_t i = 0; i < vertexCount; i++)
		{
			min.x = std::fmin(min.x, vertices[i].position.x);
			min.y = std::fmin(min.y, vertices[i].position.y);
			min.z = std::fmin(min.z, vertices[i].position.z);
			max.x = std::fmax(max.x, vertices[i].position.x);
			max.y = std::fmax(max.y, vertices[i].position.y);
			max.z = std::fmax(max.z, vertices[i].position.z);
		}
	}

	StaticMeshActor();
	~StaticMeshActor();
};

struct SkeletalMeshActor : Actor {
	void ReadData(FIStream& data);
};

struct InterpActor : Actor {
	void ReadData(FIStream& data);
};

struct LightActor : Actor {
	void ReadData(FIStream& data);
};

struct PointLight : LightActor {
	void ReadData(FIStream& data);
};

struct SpotLight : LightActor {
	void ReadData(FIStream& data);
};

#endif
