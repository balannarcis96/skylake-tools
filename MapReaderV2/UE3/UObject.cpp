#include "UObject.h"
#include "UPackage.h"
#include "FIStream.h"
#include "UClass.h"
#include "FPropertyTag.h" 

#include "StaticMesh.h"
#include "Level.h"
#include "Model.h"
#include "Actor.h"
#include "Texture2D.h"
#include "UClass.h"
#include "Terrain.h"
#include "TerrainComponent.h"
#include "MeshComponent.h"

UObject * UObject::ReadFromStream(FIStream * stream)
{
	int32 objIndex = stream->ReadInt32();

	UObject * uObject = stream->_package->uObjectForIndex(objIndex);
	return uObject;
}

UObject * UObject::UObjectForClass(const char * uClass)
{
	if (!uClass) {
		return nullptr;
	}

	if (strE(uClass, "StaticMesh")) {
		return new StaticMesh();
	}
	else if (strE(uClass, "Level")) {
		return new Level();
	}
	else if (strE(uClass, "StaticMeshActor")) {
		return new StaticMeshActor();
	}
	else if (strE(uClass, "LightActor")) {
		return new LightActor();
	}
	else if (strE(uClass, "Texture2D")) {
		return new Texture2D();
	}
	else if (strE(uClass, "Terrain")) {
		return new Terrain();
	}
	else if (strE(uClass, "TerrainComponent")) {
		return new TerrainComponent();
	}
	else if (strE(uClass, "ShadowMapTexture2D")) {
		return new Texture2D();
	}
	else if (strE(uClass, "LightMapTexture2D")) {
		return new Texture2D();
	}
	else if (strE(uClass, "TerrainWeightMapTexture")) {
		return new Texture2D();
	}
	else if (strE(uClass, "StaticMeshComponent")) {
		return new StaticMeshComponent();
	}
	else if (strE(uClass, "Actor")) {
		return new Actor();
	}
	else if (strE(uClass, "ActorComponent")) {
		return new ActorComponent();
	}
	else if (strE(uClass, "PrimitiveComponent")) {
		return new PrimitiveComponent();
	}
	else if (strE(uClass, "DistributionFloat")) {
		return new DistributionFloat();
	}
	else if (strE(uClass, "DistributionFloatConstant")) {
		return new DistributionFloatConstant();
	}
	else if (strE(uClass, "DistributionFloatConstantCurve")) {
		return new DistributionFloatConstantCurve();
	}
	else if (strE(uClass, "DistributionFloatUniform")) {
		return new DistributionFloatUniform();
	}
	else if (strE(uClass, "DistributionVector")) {
		return new DistributionVector();
	}
	else if (strE(uClass, "DistributionVectorConstantCurve")) {
		return new DistributionVectorConstantCurve();
	}
	else if (strE(uClass, "DistributionVectorUniform")) {
		return new DistributionVectorUniform();
	}
	else if (strE(uClass, "LightComponent")) {
		return new LightComponent();
	}
	else if (strE(uClass, "SpotLightComponent")) {
		return new SpotLightComponent();
	}
	else if (strE(uClass, "Model")) {
		//todo build the model
	}

	return new UObject();
}

UObject::UObject()
{
	stateFrame = nullptr;
	native = nullptr;
}

UObject::UObject(UPackage * package)
	:FReadable(package)
{
	stateFrame = nullptr;
	native = nullptr;
}

UObject::UObject(FIStream * stream)
	:FReadable(stream)
{
	stateFrame = nullptr;
	native = nullptr;
}

UObject::~UObject()
{
	if (stateFrame) {
		delete stateFrame;
		stateFrame = nullptr;
	}
}

void UObject::ReadObject(bool fallBackToSafe) {
	if ((!exportObject || !exportObject->originalOffset) && !(exportObject->objectFlags & RF_ClassDefaultObject) && !native) {
		return;
	}

	FIStream& s = package->stream;
	u_int64 safePos = s._pos;
	s._pos = exportObject->originalOffset;

	if (exportObject->objectFlags & RF_HasStack) {
		stateFrame = new FStateFrame(&package->stream);
	}

	FObjectExport* parent = (FObjectExport*)Parent();
	if (!IsNative() && !(exportObject->objectFlags & RF_ClassDefaultObject) && !(!parent && parent->objectFlags & RF_ClassDefaultObject)) {
		if (strstr(Class(), "Component") != NULL) { // Class() is subclass of UComponent
			expressionIndex = s.ReadInt32();
		}
		else {
			if (StartsWith(Class(), "Distribution")) {
				expressionIndex = s.ReadInt32();

			}
			else if (EndsWith(Class(), "Component")) {
				expressionIndex = s.ReadInt32();
			}
		}
	}

	netIndex = s.ReadInt32();

	// read FProperties here
	FPropertyTag * fProperty = new FPropertyTag(&package->stream, this, nullptr);
	if (!fProperty) {
		return;
	}

	properties.push_back(fProperty);

	while (!fProperty->IsNone()) {
		fProperty = new FPropertyTag(&package->stream, this, nullptr);
		if (!fProperty)
			break;
		properties.push_back(fProperty);
	}

	rawDataOffset = s._pos;
	dataSize = exportObject->serialSize - (rawDataOffset - exportObject->originalOffset);

	if (exportObject->objectFlags & RF_ClassDefaultObject) {
		if (fallBackToSafe) {
			s._pos = safePos;
		}
		return;
	}

	ReadData(s);

	if (fallBackToSafe) {
		s._pos = safePos;
	}
}

void UObject::ReadData(FIStream & data)
{
}

const char * UObject::Path() {
	if (_path != "") {
		return _path.c_str();
	}

	_path = std::string(Name());

	FObject * parent = fObject->Parent();
	FObject* last = nullptr;
	while (parent) {
		_path.insert(0, "/");
		_path.insert(0, parent->Name());

		last = parent;
		parent = parent->Parent();
	}

	FObjectExport * expo = (FObjectExport*)last;
	if (expo) {
		_path.insert(0, "/");
		_path.insert(0, package->Name());
	}

	return _path.c_str();
}

FObject * UObject::Parent() const
{
	if (importObject) {
		return importObject;
	}
	else if (exportObject) {
		return exportObject;
	}

	throw std::exception("no fObject found for UObject");
}

const bool UObject::IsNative() const
{
	return false;
}

const bool UObject::IsZero() const
{
	return false;
}

FPropertyTag * UObject::GetPropByName(const char * name) const {
	for (size_t i = 0; i < properties.size(); i++)
	{
		if (strcmp(properties[i]->Name(), name) == 0) {
			return properties[i];
		}
	}

	return nullptr;
}

const char * UObject::Name() const
{
	if (importObject) {
		return importObject->Name();
	}
	else if (exportObject) {
		return exportObject->Name();
	}

	return nullptr;
}

const char * UObject::Class() const
{
	if (importObject) {
		return importObject->Class();
	}
	else if (exportObject) {
		return exportObject->Class();
	}
}
