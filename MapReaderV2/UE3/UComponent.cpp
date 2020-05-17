#include "UComponent.h"
#include "FPropertyTag.h"
#include "FVector.h"

UComponent::UComponent()
{
	templateOwnerClass = nullptr;
}

void UComponent::ReadData(FIStream & data)
{
	UObject::ReadData(data);

	data._pos = rawDataOffset;
}

const Vector3 ActorComponent::rotation() const
{
	FPropertyTag* prop = GetPropByName("Rotation");
	if (!prop || !prop->value) {
		return{ 0.0f,0.0f ,0.0f };
	}

	return ((FRotator*)prop->value)->euler();
}

const Vector3 ActorComponent::scale3D() const
{
	FPropertyTag* prop = GetPropByName("DrawScale3D");
	if (!prop || !prop->value) {
		return{ 1.0f,1.0f ,1.0f };
	}

	FVector3 * fV = ((FVector3*)prop->value);
	return{ fV->x , fV->y, fV->z };
}
const Vector3 ActorComponent::location() const
{
	FPropertyTag* prop = GetPropByName("Location");
	if (!prop || !prop->value) {
		return{ 0.0f,0.0f ,0.0f };
	}

	FVector3 * fV = ((FVector3*)prop->value);
	return{ fV->x , fV->y, fV->z };
}

const Vector3 ActorComponent::translation() const
{
	FPropertyTag* prop = GetPropByName("Translation");
	if (!prop || !prop->value) {
		return{ 0.0f,0.0f ,0.0f };
	}

	FVector3 * fV = ((FVector3*)prop->value);
	return{ fV->x , fV->y, fV->z };
}

const float ActorComponent::scale() const
{
	FPropertyTag* prop = GetPropByName("DrawScale");
	if (!prop || !prop->value) {
		return 1.0f;
	}

	return *(float*)prop->value;
}

const const FRotator ActorComponent::rotator() const
{
	FPropertyTag* prop = GetPropByName("Rotation");
	if (!prop || !prop->value) {
		return FRotator();
	}

	return *(FRotator*)prop->value;
}

void LightComponent::ReadData(FIStream & data)
{
	ActorComponent::ReadData(data);

	FPropertyTag* tag = GetPropByName("LightColor");
	lightColor = tag->value ? (FColor*)tag->value : nullptr;

	tag = GetPropByName("Brightness");
	brughtness = tag->value ? *(double*)tag->value : 1.0f;

	inclusionConvexVolumes = FArray<FConvexVolume>(&data);
	exclusionConvexVolumes = FArray<FConvexVolume>(&data);
}

void PointLightComponent::ReadData(FIStream & data)
{
	LightComponent::ReadData(data);

	FPropertyTag* tag = GetPropByName("Radius");
	radius = tag->value ? *(double*)tag->value : 100.0f;

	tag = GetPropByName("FalloffExponent");
	falloffExponent = tag->value ? *(double*)tag->value : 3.0f;


	tag = GetPropByName("Translation");
	if (tag && tag->value) {
		FVector3 *fV3 = (FVector3 *)tag->value;
		lightTranslation = { fV3->x, fV3->y, fV3->z };
	}
	else {
		lightTranslation = { 0.0f,0.0f,0.0f };
	}
}

void SpotLightComponent::ReadData(FIStream & data)
{
	LightComponent::ReadData(data);

	FPropertyTag* tag = GetPropByName("InnerConeAngle");
	innerConeAngle = tag->value ? *(double*)tag->value : 0.01f;
	if (innerConeAngle < 0.01f && innerConeAngle > -0.01f) {
		innerConeAngle = 0.01f;
	}

	tag = GetPropByName("OuterConeAngle");
	outerConeAngle = tag->value ? *(double*)tag->value : 0.01f;

	tag = GetPropByName("LightShaftConeAngle");
	lightShaftConeAngle = tag->value ? *(double*)tag->value : 0.01f;
}
