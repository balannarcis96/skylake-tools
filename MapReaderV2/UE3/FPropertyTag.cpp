#include "FPropertyTag.h"
#include "FVector.h"
#include "FColor.h"
#include "FGUID.h"
#include "FMatrix.h"
#include "FRotator.h"
#include "UObject.h"
#include "FGUID.h"

void _clearPropVec(std::vector<FPropertyTag*> * vec) {
	if (!vec || !vec->size()) {
		return;
	}

	for (size_t i = 0; i < vec->size(); i++)
	{
		auto * e = (*vec)[i];
		if (e) {
			delete e;
			e = nullptr;
		}
	}

	vec->clear();
	delete vec;
}

void _clearPropVec2(std::vector<std::vector<FPropertyTag*>> * vec) {
	if (!vec || !vec->size()) {
		return;
	}

	for (size_t i = 0; i < vec->size(); i++)
	{
		auto & e = (*vec)[i];
		for (size_t j = 0; j < e.size(); j++)
		{
			if (e[j]) {
				delete e[j];
			}
		}
		e.clear();
	}

	vec->clear();
	delete vec;
}

FPropertyTag::FPropertyTag(FIStream * stream, UObject * uObj, FPropertyTag * parent)
	: FReadable(stream)
{
	this->parent = parent;
	uObject = uObj;

	fName = FName(stream);
	if (IsNone()) {
		return;
	}

	fType = FName(stream);
	if (!fType()) {
		return;
	}

	dataSize = stream->ReadInt32();
	arrayIndex = stream->ReadInt32();

	if (fType.IsEqual(kPropTypeInt)  || fType.IsEqual(kPropTypeBool)) {
		value = new int32;
		*(int32*)value = stream->ReadInt32();
	}
	else if (fType.IsEqual(kPropTypeObj)) {
		value = UObject::ReadFromStream(stream);
	}
	else if (fType.IsEqual(kPropTypeByte)) {
		value = new uint8[dataSize];
		stream->Read((uint8*)value, dataSize);
	}
	else if (fType.IsEqual(kPropTypeFloat)) {
		value = new float;
		*(float*)value = stream->ReadFloat();
	}
	else if (fType.IsEqual(kPropTypeName)) {
		value = new FName(stream);
	}
	else if (fType.IsEqual(kPropTypeString)) {
		value = new FString(stream);
	}
	else if (fType.IsEqual(kPropTypeStruct)) {
		fStruct = FName(stream);

		if (fStruct.IsEqual(kPropTypeStructVector)) {
			value = new FVector3(stream);
		}
		else if (fStruct.IsEqual(kPropTypeStructVector4)) {
			value = new FVector4(stream);
		}
		else if (fStruct.IsEqual(kPropTypeStructColor)) {
			if (dataSize == 4) {
				value = new FColor(stream);
			}
			else {
				value = new FLinerColor(stream);
			}
		}
		else if (fStruct.IsEqual(kPropTypeStructLinearColor)) {
			if (dataSize == 4) {
				value = new FColor(stream);
			}
			else {
				value = new FLinerColor(stream);
			}
		}
		else if (fStruct.IsEqual(kPropTypeStructGuid)) {
			value = new FGUID(stream);
		}
		else if (fStruct.IsEqual(kPropTypeStructRotator)) {
			value = new FRotator(stream);
		}
		else if (fStruct.IsEqual(kPropTypeStructMatrix)) {
			value = new FMatrix(stream);
		}
		else {
			u_int64 safePos = stream->_pos;
			int32 err = FALSE;
			FPropertyTag * subProp = nullptr;

			if (structs.find(fStruct()) != structs.end()) {
				const char * sName = structs[fStruct()];
				if (strE(sName, "Property")) {
					subProp = new FPropertyTag(stream, uObj, this);
				}
				else {
					//unk property
				}
			}

			if (subProp) {
				arrayType = "Property";
				value = new std::vector<FPropertyTag>();
				std::vector<FPropertyTag*>* _val = (std::vector<FPropertyTag*>*)value;
				_val->push_back(subProp);

				while (!subProp->IsNone()) {
					subProp = new FPropertyTag(stream, uObj, this);
					if (!subProp) {
						err = TRUE;
						stream->_pos = safePos;
						_clearPropVec(_val);
						value = nullptr;
						break;
					}
					_val->push_back(subProp);
				}
			}

			if (err || !subProp) {
				arrayType = "Raw";
				value = new uint8[dataSize];
				stream->Read((uint8*)value, dataSize);
			}

		}
	}
	else if (fType.IsEqual(kPropTypeArray)) {
		if (!dataSize) {
			return;
		}

		elementCount = stream->ReadInt32();
		if (!elementCount) {
			return;
		}

		const char * type = propertyTypeForName(fName(), uObj->Class());
		arrayType = type ? type : "";

		if (!type || strE(type, "Raw")) {
			if (!type) {
				arrayType = "Raw";
			}

			value = new uint8[dataSize - 4];
			stream->Read((uint8*)value, dataSize - 4);
		}
		else if (strE(type, "Property")) {
			u_int64 safePos = stream->_pos;
			int32 i = 0;

			value = new std::vector<std::vector<FPropertyTag*>>();
			std::vector<std::vector<FPropertyTag*>> * _val = (std::vector<std::vector<FPropertyTag*>>*)value;
			_val->push_back(std::vector<FPropertyTag*>());

			for (; i < elementCount; )
			{
				FPropertyTag * addTag = new FPropertyTag(stream, uObj, this);
				if (!addTag) {
					i = -1;
					break;
				}

				if (!addTag->IsNone()) {
					(*_val)[_val->size() - 1].push_back(addTag);
				}
				else {
					(*_val)[_val->size() - 1].push_back(addTag);
					i++;
					if (i < elementCount)
						_val->push_back(std::vector<FPropertyTag*>());
				}
			}

			if (i == -1) {
				arrayType = "Raw";
				_clearPropVec2(_val);
				stream->_pos = safePos;
				value = new uint8[dataSize - 4];
				stream->Read((uint8*)value, dataSize - 4);
			}

		}
		else if (strE(type, "Object")) {
			value = new std::vector<UObject*>();
			std::vector<UObject*> * _val = (std::vector<UObject*>*)value;
			for (size_t i = 0; i < elementCount; i++)
			{
				_val->push_back(UObject::ReadFromStream(stream));
			}
		}
		else if (strE(type, "Name")) {
			value = new std::vector<FName>();
			std::vector<FName> * _val = (std::vector<FName>*)value;
			for (size_t i = 0; i < elementCount; i++)
			{
				_val->push_back(FName(stream));
			}
		}
		else {
			int32 elementSize = (dataSize - 4) / elementCount;
			if (elementSize == 1) {
				if (EndsWith(Name(), "Data")) {
					arrayType = "Raw";
				}

				value = new uint8[elementCount];
				stream->Read((uint8*)value, elementCount);
			}
			else if (elementSize == 2) {
				arrayType = "Int16Array";

				value = new int16[elementCount];
				stream->Read((uint8*)value, elementCount * 2);
			}
			else if (elementSize == 4) {
				arrayType = "Int32Array";

				value = new int32[elementCount];
				stream->Read((uint8*)value, elementCount * 4);
			}
			else if (elementSize == 8) {
				arrayType = "Int64Array";

				value = new _int64[elementCount];
				stream->Read((uint8*)value, elementCount * 8);
			}
			else if (elementSize == 16) {
				arrayType = "FGUIDArray";

				value = new FGUID[elementCount];
				for (size_t i = 0; i < elementCount; i++)
				{
					((FGUID*)value)[i] = FGUID(stream);
				}
			}
			else {
				u_int64 safePos = stream->_pos;
				int32 i = 0;

				value = new std::vector<std::vector<FPropertyTag*>>();
				std::vector<std::vector<FPropertyTag*>> * _val = (std::vector<std::vector<FPropertyTag*>>*)value;
				_val->push_back(std::vector<FPropertyTag*>());

				const char* propName = fName();
				if (!(strE(propName, "EdgeDirections") || strE(propName, "FaceNormalDirections"))) {
					if (EndsWith(propName, "Data")) {
						i = -1;
					}
					else {
						for (; i < elementCount; )
						{
							FPropertyTag * addTag = new FPropertyTag(stream, uObj, this);
							if (!addTag) {
								i = -1;
								break;
							}

							if (!addTag->IsNone()) {
								(*_val)[_val->size() - 1].push_back(addTag);
							}
							else {
								(*_val)[_val->size() - 1].push_back(addTag);
								i++;
								if (i < elementCount)
									_val->push_back(std::vector<FPropertyTag*>());
							}
						}

					}

					if (i == -1) {
						_clearPropVec2(_val);
					}
				}
				else {
					i = -1;
				}

				if (i == -1) {
					arrayType = "Raw";
					_clearPropVec2(_val);
					stream->_pos = safePos;
					value = new uint8[dataSize - 4];
					stream->Read((uint8*)value, dataSize - 4);
				}
				else {
					arrayType = "Property";
				}
			}

		}
	}
	else {
		if (dataSize) {
			arrayType = "RawUNK";
			u_int64 safePos = stream->_pos;
			int32 i = 0;

			FPropertyTag * addTag = nullptr;
			value = new std::vector<FPropertyTag*>();

			do {
				if (strE(structs[fName()], "Property")) {
					addTag = new FPropertyTag(stream, uObj, this);
				}

				if (addTag) {
					((std::vector<FPropertyTag*>*)value)->push_back(addTag);
				}
				else {
					i = -1;
					break;
				}

			} while (!addTag->IsNone());


			if (i == -1) {
				_clearPropVec((std::vector<FPropertyTag*>*)value);
				stream->_pos = safePos;
				value = new uint8[dataSize];
				stream->Read((uint8*)value, dataSize);
			}
			else {
				arrayType = "Property";
			}
		}
	}
}

FPropertyTag::~FPropertyTag()
{
	if (value) {
		delete value;
		value = nullptr;
	}
}

const char * FPropertyTag::Name() const
{
	return fName();
}

const char * FPropertyTag::Type() const
{
	return fType();
}

const char * FPropertyTag::StructName() const
{
	return fStruct();
}

const bool FPropertyTag::IsNone() const
{
	return strE(fName(), "None");
}




void FPropertyTag::InitStructs()
{
	structs["RawDistributionFloat"] = "Property";
	structs["RawDistributionVector"] = "Property";
	structs["ExpressionInput"] = "Property";
	structs["VectorMaterialInput"] = "Property";
	structs["ScalarMaterialInput"] = "Property";
	structs["LightingChannelContainer"] = "Property";
	structs["KAggregateGeom"] = "Property";
	structs["CloudMesh"] = "Property";
	structs["SkyBackdrop"] = "Property";
	structs["SkySystem"] = "Property";
	structs["PostProcessSettings"] = "Property";
	structs["DirectionalLightSetting"] = "Property";
	structs["SkyLightSetting"] = "Property";
	structs["S1EnvironmentMapSetting"] = "Property";
	structs["S1DefaultPostProcessing"] = "Property";
	structs["S1LightProbeSetting"] = "Property";
	structs["S1PostProcessing"] = "Property";
	structs["S1WindSetting"] = "Property";
	structs["ColorMaterialInput"] = "Property";
	structs["ClimateSetting"] = "Property";
}

void FPropertyTag::InitProperties()
{
	properties["GFxMovieInfo"]["References"] = "Object";

	properties["StaticMeshComponent"]["Materials"] = "Object";
	properties["StaticMeshComponent"]["IrrelevantLights"] = "Raw";

	properties["PhysicsAssetInstance"]["Constraints"] = "Object";
	properties["PhysicsAssetInstance"]["Bodies"] = "Object";

	properties["PhysicsAsset"]["ConstraintSetup"] = "Object";
	properties["PhysicsAsset"]["BodySetup"] = "Object";
	properties["PhysicsAsset"]["BoundsBodies"] = "Int";

	properties["InterpCurveEdSetup"]["Curves"] = "Property";

	properties["ParticleSystem"]["Emitters"] = "Property";
	properties["ParticleSystem"]["LODDistances"] = "Int";

	properties["PostProcessChain"]["Effects"] = "Object";

	properties["Material"]["EditorComments"] = "Int";
	properties["Material"]["ReferencedTextures"] = "Object";
	properties["Material"]["Expressions"] = "Object";

	properties["MaterialInstanceConstant"]["ScalarParameterValues"] = "Property";
	properties["MaterialInstanceConstant"]["TextureParameterValues"] = "Property";
	properties["MaterialInstanceConstant"]["VectorParameterValues"] = "Property";
	properties["MaterialInstanceConstant"]["ReferencedTextures"] = "Object";

	properties["SkeletalMesh"]["LODInfo"] = "Property";
	properties["SkeletalMesh"]["Sockets"] = "Object";
	properties["SkeletalMesh"]["bEnableShadowCasting"] = "Bool";
	properties["SkeletalMesh"]["LODMaterialMap"] = "Int";

	properties["AnimSet"]["TrackBoneNames"] = "Name";
	properties["AnimSet"]["Sequences"] = "Object";

	properties["Terrain"]["Layers"] = "ArrayProperty";
}

void FPropertyTag::InitClasses()
{
	classes["Engine"].push_back("ActorFactoryStaticMesh");
	classes["Engine"].push_back("AeroInnerVolume");
	classes["Engine"].push_back("AeroSet");
	classes["Engine"].push_back("AeroVolume");
	classes["Engine"].push_back("AmbientSoundNonLoop");
	classes["Engine"].push_back("AmbientSoundSimple");
	classes["Engine"].push_back("AnimNodeBlend");
	classes["Engine"].push_back("AnimNodeBlendPerBone");
	classes["Engine"].push_back("AnimNodeSequence");
	classes["Engine"].push_back("AnimNodeSlot");
	classes["Engine"].push_back("AnimSequence");
	classes["Engine"].push_back("AnimSet");
	classes["Engine"].push_back("AnimTree");
	classes["Engine"].push_back("AudioComponent");
	classes["Engine"].push_back("BlockingVolume");
	classes["Engine"].push_back("BookMark");
	classes["Engine"].push_back("Brush");
	classes["Engine"].push_back("BrushComponent");
	classes["Engine"].push_back("CameraActor");
	classes["Engine"].push_back("CameraAnim");
	classes["Engine"].push_back("CylinderComponent");
	classes["Engine"].push_back("DecalActor");
	classes["Engine"].push_back("DecalComponent");
	classes["Engine"].push_back("DirectionalLight");
	classes["Engine"].push_back("DirectionalLightComponent");
	classes["Engine"].push_back("DirectionalLightToggleable");
	classes["Engine"].push_back("DistributionFloatConstant");
	classes["Engine"].push_back("DistributionFloatConstantCurve");
	classes["Engine"].push_back("DistributionFloatUniform");
	classes["Engine"].push_back("DistributionVectorConstant");
	classes["Engine"].push_back("DistributionVectorConstantCurve");
	classes["Engine"].push_back("DistributionVectorUniform");
	classes["Engine"].push_back("DistributionVectorUniformCurve");
	classes["Engine"].push_back("DrawFrustumComponent");
	classes["Engine"].push_back("DrawLightConeComponent");
	classes["Engine"].push_back("DrawLightRadiusComponent");
	classes["Engine"].push_back("DrawSoundRadiusComponent");
	classes["Engine"].push_back("DynamicLightEnvironmentComponent");
	classes["Engine"].push_back("Emitter");
	classes["Engine"].push_back("HeightFog");
	classes["Engine"].push_back("HeightFogComponent");
	classes["Engine"].push_back("InterpActor");
	classes["Engine"].push_back("InterpCurveEdSetup");
	classes["Engine"].push_back("InterpData");
	classes["Engine"].push_back("InterpFilter_Custom");
	classes["Engine"].push_back("InterpGroup");
	classes["Engine"].push_back("InterpGroupDirector");
	classes["Engine"].push_back("InterpTrackAnimControl");
	classes["Engine"].push_back("InterpTrackColorProp");
	classes["Engine"].push_back("InterpTrackColorScale");
	classes["Engine"].push_back("InterpTrackDirector");
	classes["Engine"].push_back("InterpTrackEvent");
	classes["Engine"].push_back("InterpTrackFade");
	classes["Engine"].push_back("InterpTrackFloatMaterialParam");
	classes["Engine"].push_back("InterpTrackFloatProp");
	classes["Engine"].push_back("InterpTrackMove");
	classes["Engine"].push_back("InterpTrackSlomo");
	classes["Engine"].push_back("InterpTrackSound");
	classes["Engine"].push_back("InterpTrackToggle");
	classes["Engine"].push_back("InterpTrackVectorMaterialParam");
	classes["Engine"].push_back("InterpTrackVectorProp");
	classes["Engine"].push_back("InterpTrackVisibility");
	classes["Engine"].push_back("KActor");
	classes["Engine"].push_back("LensFlare");
	classes["Engine"].push_back("LensFlareComponent");
	classes["Engine"].push_back("LensFlareSource");
	classes["Engine"].push_back("Level");
	classes["Engine"].push_back("LevelStreamingDistance");
	classes["Engine"].push_back("LevelStreamingKismet");
	classes["Engine"].push_back("LevelStreamingVolume");
	classes["Engine"].push_back("LightFunction");
	classes["Engine"].push_back("LightMapTexture2D");
	classes["Engine"].push_back("LightVolume");
	classes["Engine"].push_back("Material");
	classes["Engine"].push_back("MaterialExpressionAbs");
	classes["Engine"].push_back("MaterialExpressionAdd");
	classes["Engine"].push_back("MaterialExpressionAppendVector");
	classes["Engine"].push_back("MaterialExpressionBumpOffset");
	classes["Engine"].push_back("MaterialExpressionCameraVector");
	classes["Engine"].push_back("MaterialExpressionCeil");
	classes["Engine"].push_back("MaterialExpressionClamp");
	classes["Engine"].push_back("MaterialExpressionComment");
	classes["Engine"].push_back("MaterialExpressionComponentMask");
	classes["Engine"].push_back("MaterialExpressionConstant");
	classes["Engine"].push_back("MaterialExpressionConstant2Vector");
	classes["Engine"].push_back("MaterialExpressionConstant3Vector");
	classes["Engine"].push_back("MaterialExpressionConstant4Vector");
	classes["Engine"].push_back("MaterialExpressionConstantBiasScale");
	classes["Engine"].push_back("MaterialExpressionConstantClamp");
	classes["Engine"].push_back("MaterialExpressionCosine");
	classes["Engine"].push_back("MaterialExpressionDepthBiasedAlpha");
	classes["Engine"].push_back("MaterialExpressionDesaturation");
	classes["Engine"].push_back("MaterialExpressionDestColor");
	classes["Engine"].push_back("MaterialExpressionDestDepth");
	classes["Engine"].push_back("MaterialExpressionDivide");
	classes["Engine"].push_back("MaterialExpressionDotProduct");
	classes["Engine"].push_back("MaterialExpressionDynamicParameter");
	classes["Engine"].push_back("MaterialExpressionFlipBookSample");
	classes["Engine"].push_back("MaterialExpressionFloor");
	classes["Engine"].push_back("MaterialExpressionFrac");
	classes["Engine"].push_back("MaterialExpressionFresnel");
	classes["Engine"].push_back("MaterialExpressionIf");
	classes["Engine"].push_back("MaterialExpressionLensFlareIntensity");
	classes["Engine"].push_back("MaterialExpressionLensFlareOcclusion");
	classes["Engine"].push_back("MaterialExpressionLensFlareRadialDistance");
	classes["Engine"].push_back("MaterialExpressionLinearInterpolate");
	classes["Engine"].push_back("MaterialExpressionMeshEmitterVertexColor");
	classes["Engine"].push_back("MaterialExpressionMeshSubUV");
	classes["Engine"].push_back("MaterialExpressionMultiply");
	classes["Engine"].push_back("MaterialExpressionOneMinus");
	classes["Engine"].push_back("MaterialExpressionPanner");
	classes["Engine"].push_back("MaterialExpressionParameter");
	classes["Engine"].push_back("MaterialExpressionParticleSubUV");
	classes["Engine"].push_back("MaterialExpressionPixelDepth");
	classes["Engine"].push_back("MaterialExpressionPower");
	classes["Engine"].push_back("MaterialExpressionReflectionVector");
	classes["Engine"].push_back("MaterialExpressionRotator");
	classes["Engine"].push_back("MaterialExpressionScalarParameter");
	classes["Engine"].push_back("MaterialExpressionSceneDepth");
	classes["Engine"].push_back("MaterialExpressionScreenPosition");
	classes["Engine"].push_back("MaterialExpressionSine");
	classes["Engine"].push_back("MaterialExpressionSquareRoot");
	classes["Engine"].push_back("MaterialExpressionStaticComponentMaskParameter");
	classes["Engine"].push_back("MaterialExpressionStaticSwitchParameter");
	classes["Engine"].push_back("MaterialExpressionSubtract");
	classes["Engine"].push_back("MaterialExpressionTextureCoordinate");
	classes["Engine"].push_back("MaterialExpressionTextureSample");
	classes["Engine"].push_back("MaterialExpressionTextureSampleParameter2D");
	classes["Engine"].push_back("MaterialExpressionTextureSampleParameterCube");
	classes["Engine"].push_back("MaterialExpressionTextureSampleParameterSubUV");
	classes["Engine"].push_back("MaterialExpressionTime");
	classes["Engine"].push_back("MaterialExpressionTransform");
	classes["Engine"].push_back("MaterialExpressionTransformPosition");
	classes["Engine"].push_back("MaterialExpressionVectorParameter");
	classes["Engine"].push_back("MaterialExpressionVertexColor");
	classes["Engine"].push_back("MaterialExpressionWorldPosition");
	classes["Engine"].push_back("MaterialInstanceActor");
	classes["Engine"].push_back("MaterialInstanceConstant");
	classes["Engine"].push_back("MaterialInstanceTimeVarying");
	classes["Engine"].push_back("Model");
	classes["Engine"].push_back("ModelComponent");
	classes["Engine"].push_back("Note");
	classes["Engine"].push_back("ObjectRedirector");
	classes["Engine"].push_back("Package");
	classes["Engine"].push_back("ParticleLODLevel");
	classes["Engine"].push_back("ParticleModuleAcceleration");
	classes["Engine"].push_back("ParticleModuleAccelerationOverLifetime");
	classes["Engine"].push_back("ParticleModuleBeamSource");
	classes["Engine"].push_back("ParticleModuleBeamTarget");
	classes["Engine"].push_back("ParticleModuleColor");
	classes["Engine"].push_back("ParticleModuleColorOverLife");
	classes["Engine"].push_back("ParticleModuleColorScaleOverLife");
	classes["Engine"].push_back("ParticleModuleLifetime");
	classes["Engine"].push_back("ParticleModuleLocation");
	classes["Engine"].push_back("ParticleModuleLocationDirect");
	classes["Engine"].push_back("ParticleModuleLocationEmitter");
	classes["Engine"].push_back("ParticleModuleLocationEmitterDirect");
	classes["Engine"].push_back("ParticleModuleLocationPrimitiveCylinder");
	classes["Engine"].push_back("ParticleModuleLocationPrimitiveSphere");
	classes["Engine"].push_back("ParticleModuleMeshRotation");
	classes["Engine"].push_back("ParticleModuleMeshRotationRate");
	classes["Engine"].push_back("ParticleModuleMeshRotationRateMultiplyLife");
	classes["Engine"].push_back("ParticleModuleOrbit");
	classes["Engine"].push_back("ParticleModuleOrientationAxisLock");
	classes["Engine"].push_back("ParticleModuleParameterDynamic");
	classes["Engine"].push_back("ParticleModuleRequired");
	classes["Engine"].push_back("ParticleModuleRotation");
	classes["Engine"].push_back("ParticleModuleRotationRate");
	classes["Engine"].push_back("ParticleModuleRotationRateMultiplyLife");
	classes["Engine"].push_back("ParticleModuleS1Climate");
	classes["Engine"].push_back("ParticleModuleSize");
	classes["Engine"].push_back("ParticleModuleSizeMultiplyLife");
	classes["Engine"].push_back("ParticleModuleSizeMultiplyVelocity");
	classes["Engine"].push_back("ParticleModuleSizeScale");
	classes["Engine"].push_back("ParticleModuleSpawn");
	classes["Engine"].push_back("ParticleModuleSubUV");
	classes["Engine"].push_back("ParticleModuleTypeDataBeam2");
	classes["Engine"].push_back("ParticleModuleTypeDataDecal");
	classes["Engine"].push_back("ParticleModuleTypeDataMesh");
	classes["Engine"].push_back("ParticleModuleTypeDataTrail2");
	classes["Engine"].push_back("ParticleModuleVelocity");
	classes["Engine"].push_back("ParticleModuleVelocityInheritParent");
	classes["Engine"].push_back("ParticleModuleVelocityOverLifetime");
	classes["Engine"].push_back("ParticleSpriteEmitter");
	classes["Engine"].push_back("ParticleSystem");
	classes["Engine"].push_back("ParticleSystemComponent");
	classes["Engine"].push_back("PathNode");
	classes["Engine"].push_back("PhysicalMaterial");
	classes["Engine"].push_back("PhysicsAsset");
	classes["Engine"].push_back("PhysicsAssetInstance");
	classes["Engine"].push_back("PlayerStart");
	classes["Engine"].push_back("PointLight");
	classes["Engine"].push_back("PointLightComponent");
	classes["Engine"].push_back("PointLightMovable");
	classes["Engine"].push_back("Polys");
	classes["Engine"].push_back("Prefab");
	classes["Engine"].push_back("PrefabInstance");
	classes["Engine"].push_back("RB_BodyInstance");
	classes["Engine"].push_back("RB_BodySetup");
	classes["Engine"].push_back("ReachSpec");
	classes["Engine"].push_back("ReverbVolume");
	classes["Engine"].push_back("S1AnimNodeSequence");
	classes["Engine"].push_back("S1ChannelVolume");
	classes["Engine"].push_back("S1IsolatedSoundVolume");
	classes["Engine"].push_back("S1LevelStreamingBaseLevel");
	classes["Engine"].push_back("S1LevelStreamingSound");
	classes["Engine"].push_back("S1LevelStreamingSuperLow");
	classes["Engine"].push_back("S1LevelStreamingVOID");
	classes["Engine"].push_back("S1MapComment");
	classes["Engine"].push_back("S1MatineeActorPlayer");
	classes["Engine"].push_back("S1MatineeActorVillager");
	classes["Engine"].push_back("S1MusicVolume");
	classes["Engine"].push_back("S1ParentSkeletalMeshComponent");
	classes["Engine"].push_back("S1PhysicalMaterialProperty");
	classes["Engine"].push_back("S1SeqAct_CameraShake");
	classes["Engine"].push_back("S1SeqAct_ChangeWeapon");
	classes["Engine"].push_back("S1SeqAct_MovieScript");
	classes["Engine"].push_back("S1SeqAct_ToggleCinematic");
	classes["Engine"].push_back("S1SkeletalMeshActorMAT");
	classes["Engine"].push_back("S1SmartActor");
	classes["Engine"].push_back("S1SmartGeneralStaticMeshComponent");
	classes["Engine"].push_back("S1SoundNodeSequence");
	classes["Engine"].push_back("S1UnloadS1LevelVolume");
	classes["Engine"].push_back("S1WaterVolume");
	classes["Engine"].push_back("SeqAct_ActivateRemoteEvent");
	classes["Engine"].push_back("SeqAct_Interp");
	classes["Engine"].push_back("SeqAct_LevelStreaming");
	classes["Engine"].push_back("SeqAct_Log");
	classes["Engine"].push_back("SeqAct_PlaySound");
	classes["Engine"].push_back("SeqEvent_Console");
	classes["Engine"].push_back("SeqEvent_LevelLoaded");
	classes["Engine"].push_back("Sequence");
	classes["Engine"].push_back("SequenceFrame");
	classes["Engine"].push_back("SequenceFrameWrapped");
	classes["Engine"].push_back("SeqVar_Float");
	classes["Engine"].push_back("SeqVar_Object");
	classes["Engine"].push_back("ShadowMap1D");
	classes["Engine"].push_back("ShadowMap2D");
	classes["Engine"].push_back("ShadowMapTexture2D");
	classes["Engine"].push_back("SkeletalMesh");
	classes["Engine"].push_back("SkeletalMeshActor");
	classes["Engine"].push_back("SkeletalMeshActorMAT");
	classes["Engine"].push_back("SkeletalMeshComponent");
	classes["Engine"].push_back("SkeletalMeshSocket");
	classes["Engine"].push_back("SkyLight");
	classes["Engine"].push_back("SkyLightComponent");
	classes["Engine"].push_back("SoundCue");
	classes["Engine"].push_back("SoundNodeAmbient");
	classes["Engine"].push_back("SoundNodeAmbientNonLoop");
	classes["Engine"].push_back("SoundNodeDelay");
	classes["Engine"].push_back("SoundNodeLooping");
	classes["Engine"].push_back("SoundNodeWave");
	classes["Engine"].push_back("SpeedTree");
	classes["Engine"].push_back("SpeedTreeActor");
	classes["Engine"].push_back("SpeedTreeActorFactory");
	classes["Engine"].push_back("SpeedTreeComponent");
	classes["Engine"].push_back("SpotLight");
	classes["Engine"].push_back("SpotLightComponent");
	classes["Engine"].push_back("SpotLightMovable");
	classes["Engine"].push_back("SpotLightToggleable");
	classes["Engine"].push_back("SpriteComponent");
	classes["Engine"].push_back("StaticMesh");
	classes["Engine"].push_back("StaticMeshActor");
	classes["Engine"].push_back("StaticMeshComponent");
	classes["Engine"].push_back("Terrain");
	classes["Engine"].push_back("TerrainComponent");
	classes["Engine"].push_back("TerrainLayerSetup");
	classes["Engine"].push_back("TerrainMaterial");
	classes["Engine"].push_back("TerrainWeightMapTexture");
	classes["Engine"].push_back("Texture2D");
	classes["Engine"].push_back("TextureCube");
	classes["Engine"].push_back("TextureFlipBook");
	classes["Engine"].push_back("Trigger");
	classes["Engine"].push_back("World");
	classes["Engine"].push_back("WorldInfo");

	classes["Core"].push_back("Class");
	classes["Core"].push_back("MetaData");
	classes["Core"].push_back("TextBuffer");
	classes["Core"].push_back("ArrayProperty");
	classes["Core"].push_back("ByteProperty");
	classes["Core"].push_back("BoolProperty");
	classes["Core"].push_back("ClassProperty");
	classes["Core"].push_back("ComponentProperty");
	classes["Core"].push_back("FloatProperty");
	classes["Core"].push_back("InterfaceProperty");
	classes["Core"].push_back("IntProperty");
	classes["Core"].push_back("NameProperty");
	classes["Core"].push_back("ObjectProperty");
	classes["Core"].push_back("ScriptProperty");
	classes["Core"].push_back("StrProperty");
	classes["Core"].push_back("StructProperty");
	classes["Core"].push_back("Const");
	classes["Core"].push_back("Enum");
	classes["Core"].push_back("Function");
	classes["Core"].push_back("Package");
	classes["Core"].push_back("ObjectRedirector");
	classes["Core"].push_back("Object");
	classes["Core"].push_back("Component");
	classes["Core"].push_back("Interface");
	classes["Core"].push_back("Subsystem");
	classes["Core"].push_back("Commandlet");
	classes["Core"].push_back("DistributionFloat");
	classes["Core"].push_back("DistributionVector");
	classes["Core"].push_back("HelpComandlet");
}

const char * FPropertyTag::propertyTypeForName(const char * name, const char * _class)
{
	if (properties.find(_class) != properties.end() &&
		properties[_class].find(name) != properties[_class].end())
	{
		return properties[_class][name];
	}
	else
	{
		return nullptr;
	}
}


std::map<const char *, const char*> FPropertyTag::structs;
std::map<const char *, std::vector<std::string>> FPropertyTag::classes;
std::map<const char *, std::map< const char*, const char*>> FPropertyTag::properties;