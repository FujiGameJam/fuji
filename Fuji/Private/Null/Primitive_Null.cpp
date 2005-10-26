#include "Fuji.h"
#include "Primitive.h"

uint32 beginCount;
uint32 currentVert;

void Primitive_InitModule()
{
}

void Primitive_DeinitModule()
{
}

void DrawMFPrimitiveStats()
{
}

void MFPrimitive(uint32 type, uint32 hint)
{
}

void MFBegin(uint32 vertexCount)
{
	beginCount = vertexCount;
}

void MFSetMatrix(const MFMatrix &mat)
{
}

void MFSetColour(const MFVector &colour)
{
}

void MFSetColour(float r, float g, float b, float a)
{
}

void MFSetColour(uint32 col)
{
}

void MFSetTexCoord1(float u, float v)
{
}

void MFSetNormal(const MFVector &normal)
{
}

void MFSetNormal(float x, float y, float z)
{
}

void MFSetPosition(const MFVector &pos)
{
	++currentVert;
}

void MFSetPosition(float x, float y, float z)
{
	++currentVert;
}

void MFEnd()
{
	DBGASSERT(currentVert == beginCount, "Incorrect number of vertices.");
}
