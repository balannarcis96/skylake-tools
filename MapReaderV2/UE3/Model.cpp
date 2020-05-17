#include "Model.h"

FBspNode::FBspNode()
{
}

FBspNode::FBspNode(FIStream * stream)
	:FReadable(stream)
{
	plane = FVector4(stream);
	iVertPool = stream->ReadInt32();
	iSurf = stream->ReadInt32();
	iVertexIndex = stream->ReadInt32();
	componentIndex = stream->ReadInt16();
	componentNodeIndex = stream->ReadInt16();
	componentElementIndex = stream->ReadInt16();
}
