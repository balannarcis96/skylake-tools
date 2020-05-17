#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#define uint8 unsigned char

#define uint16 unsigned short

#define uint32 unsigned long

#define u_int64 unsigned long long

#define int8 char

#define int16 short

#define int32 long

#define _int64 long long

#define STRING const char*

#define ulong unsigned long

#define uint unsigned int

#define wchar uint16

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define kClass								"Class"
#define kComponent							"Component"
#define kClassPackage						"Package"
#define kClassMaterial						"Material"
#define kClassMaterialInstanceConstant		"MaterialInstanceConstant"
#define kClassTexture2D						"Texture2D"
#define kClassSkeletalMesh					"SkeletalMesh"
#define kClassStaticMesh					"StaticMesh"

#define kPropNameNone  "None"
#define kPropTypeInt  "IntProperty"
#define kPropTypeFloat  "FloatProperty"
#define kPropTypeObj  "ObjectProperty"
#define kPropTypeName  "NameProperty"
#define kPropTypeString  "StrProperty"
#define kPropTypeStruct  "StructProperty"
#define kPropTypeArray  "ArrayProperty"
#define kPropTypeBool  "BoolProperty"
#define kPropTypeByte  "ByteProperty"

#define kPropTypeStructVector  "Vector"
#define kPropTypeStructVector4  "Vector4"
#define kPropTypeStructColor  "Color"
#define kPropTypeStructLinearColor  "LinearColor"
#define kPropTypeStructGuid  "Guid"
#define kPropTypeStructRotator  "Rotator"
#define kPropTypeStructMatrix  "Matrix"

#define PACKAGE_MAGIC  0x9E2A83C1

#define TERA_COOKER_V  76
#define TERA_ENGINE_V  4206

#define BULKDATA_StoreInSeparateFile  0x01			// bulk stored in different file
#define BULKDATA_CompressedZlib  0x02				// lzo
#define BULKDATA_CompressedLzo  0x10				// zlib
#define BULKDATA_Unused  0x20						// empty bulk block
#define BULKDATA_SeparateData 0x40					// offset differs

#define COMPRESSION_NONE	 0
#define COMPRESSION_LZO		 1
#define COMPRESSION_ZLIB	 2

#define MAX_NODE_VERTICES 255   // Max vertices in a Bsp node.
#define MAX_ZONES  64			// Max zones per level.

#define TERRAIN_ZSCALE  (1.0f / 128.0f)

struct Vector2 {
	float v[2];
};

struct Vector3 {
	float v[3];

	Vector3() {}
	Vector3(float x, float y, float z) { v[0] = x; v[1] = y; v[2] = z; }

	const Vector3 operator+(const Vector3& other) const {
		return Vector3(other.v[0] + v[0], other.v[1] + v[1], other.v[2] + v[2]);
	}

	const float operator[](int i) const {
		return v[i];
	}
};

struct Vector4 {
	float v[4];
};

int NormalizeAxis(int Angle);
bool StartsWith(const char *a, const char *b);
int EndsWith(const char * str, const char * suffix);


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


short Height_T(int x, int y, int maxX, int maxY, short * height);

#define _USE_MATH_DEFINES
#include <math.h>
#define degreesToRadians(angleDegrees) ((angleDegrees) * (M_PI / 180.0f))
#define radiansToDegrees(angleRadians) ((angleRadians) * (180.0f / M_PI))

#include <string.h>
#define strE(a,b) (strcmp(a,b) == 0)

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

#define __PI 3.14159265359

#define TO_RADIANS(x) (((x) * __PI) / 180)
#define TO_DEGREES(x) (((x) * 180) / __PI)

#include <Windows.h>
int32 DirectoryExists(const char * dirName_in);
int32 FileExists(const char * dirName_in);

#endif
