#ifndef UCOMPONENT_H
#define UCOMPONENT_H

#include "FColor.h"
#include "FRotator.h"
#include "UClass.h"
#include "FString.h"
#include "UObject.h"
#include "FArray.h"
#include "FBoxSphereBounds.h"
#include  "FMesh.h"

struct UComponent : UObject
{
	UClass * templateOwnerClass;
	FName templateName;

	UComponent();

	void ReadData(FIStream & data);
};

struct ActorComponent : UComponent {
	const Vector3 rotation() const;
	const Vector3 scale3D() const;
	const Vector3 translation() const;
	const Vector3 location() const;
	const float scale() const;
	const FRotator rotator() const;
};

struct PrimitiveComponent : ActorComponent {

	PrimitiveComponent()
	:ActorComponent()
	{}


	PrimitiveComponent(FIStream* stream){}

};

struct DistributionFloat : UComponent {

};

struct DistributionFloatConstant : DistributionFloat {

};

struct DistributionFloatConstantCurve : DistributionFloat {

};

struct DistributionFloatUniform : DistributionFloat {

};

struct DistributionVector : UComponent
{};

struct DistributionVectorConstant : DistributionVector
{};

struct DistributionVectorConstantCurve : DistributionVector
{};

struct DistributionVectorUniform : DistributionVector
{};

struct LightComponent : ActorComponent {
	FColor *lightColor;
	float brughtness;
	FArray<FConvexVolume>  inclusionConvexVolumes;
	FArray<FConvexVolume>  exclusionConvexVolumes;

protected:
	void ReadData(FIStream& data);
};

struct PointLightComponent : LightComponent {
	float radius;
	float falloffExponent;
	Vector3 lightTranslation;
protected:
	void ReadData(FIStream& data);
};

struct SpotLightComponent : LightComponent {
	float innerConeAngle;
	float outerConeAngle;
	float lightShaftConeAngle;

protected:
	void ReadData(FIStream& data);
};

#endif