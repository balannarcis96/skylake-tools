#include "FRotator.h"
#include "FIStream.h"

FRotator::FRotator(FIStream * stream)
	:FReadable(stream)
{
	pitch = stream->ReadInt32();
	yaw = stream->ReadInt32();
	roll = stream->ReadInt32();
}

FRotator::FRotator(int p, int y, int r)
{
	pitch = p;
	yaw = y;
	roll = r;
}

const Vector3 FRotator::euler()
{
	int components[3] = { roll ,pitch, yaw };
	normalize(components);
	float coords[3] = { 0.0f,0.0f,0.0f };

	coords[0] = components[0] * (180.0f / 32768.0f);
	coords[1] = components[1] * (180.0f / 32768.0f);
	coords[2] = components[2] * (180.0f / 32768.0f);

	return Vector3{ coords[0], coords[1], coords[2] };
}

void FRotator::normalize(int components[3])
{
	components[0] = NormalizeAxis(pitch);
	components[1] = NormalizeAxis(yaw);
	components[2] = NormalizeAxis(roll);
}

void FRotator::denormalize(int components[3])
{
	components[0] = pitch & 0xFFFF;
	components[1] = yaw & 0xFFFF;
	components[2] = roll & 0xFFFF;
}
