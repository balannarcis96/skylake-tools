#ifndef MODEL_H
#define MODEL_H

#include "UObject.h"
#include "FReadable.h"
#include "FVector.h"
#include "FBoxSphereBounds.h"
#include "FArray.h"

enum EBspNodeFlags
{
	// Flags.
	NF_NotCsg = 0x01, // Node is not a Csg splitter, i.e. is a transparent poly.
	NF_ShootThrough = 0x02, // Can shoot through (for projectile solid ops).
	NF_NotVisBlocking = 0x04, // Node does not block visibility, i.e. is an invisible collision hull.
	NF_PolyOccluded = 0x08, // Node's poly was occluded on the previously-drawn frame.
	NF_BoxOccluded = 0x10, // Node's bounding box was occluded.
	NF_BrightCorners = 0x10, // Temporary.
	NF_IsNew = 0x20, // Editor: Node was newly-added.
	NF_IsFront = 0x40, // Filter operation bounding-sphere precomputed and guaranteed to be front.
	NF_IsBack = 0x80, // Guaranteed back.

					  // Combinations of flags.
	NF_NeverMove = 0, // Bsp cleanup must not move nodes with these tags.
};

struct FBspNode : FReadable {
	FVector4 plane;
	int32 iVertPool;
	int32 iSurf;
	int32 iVertexIndex;
	int16 componentIndex;
	int16 componentNodeIndex;
	int16 componentElementIndex;

	FBspNode();
	FBspNode(FIStream * stream);
};

template<class T>
struct Model : UObject {
	FBoxSphereBounds bounds;
	TransFArray<FVector3> vectors;
	TransFArray<FVector3> points;
	TransFArray<FBspNode> nodes;
	TransFArray<T> verts;


	Model();

protected:
	void ReadData(FIStream& data) const;
};

template<class T>
inline Model<T>::Model()
{
}

template<class T>
inline void Model<T>::ReadData(FIStream & stream) const
{
	stream._pos = rawDataOffset;
	bounds = FBoxSphereBounds(&stream);
	vectors = TransFArray<FVector3>(&stream);
	points = TransFArray<FVector3>(&stream);
	nodes = TransFArray<FBspNode>(&stream);
}

#endif