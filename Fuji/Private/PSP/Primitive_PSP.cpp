#include "Common.h"
#include "Display_Internal.h"
#include "View.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "Primitive.h"
#include "Texture.h"
#include "Renderer.h"
#include "MFMaterial.h"

uint32 primType;
uint32 beginCount;
uint32 currentVert;

void Primitive_InitModule()
{
	CALLSTACK;
}

void Primitive_DeinitModule()
{
	CALLSTACK;
}

void MFPrimitive(uint32 type, uint32 hint)
{
	CALLSTACK;

	primType = type & PT_PrimMask;

	if(type & PT_Untextured)
	{
		MFMaterial_SetMaterial(MFMaterial_GetStockMaterial(Mat_White));
	}

	Renderer_Begin();
}

void MFBegin(uint32 vertexCount)
{
	CALLSTACK;

	beginCount = vertexCount;
	currentVert = 0;
}

void MFSetMatrix(const Matrix &mat)
{
	CALLSTACK;
}

void MFSetColour(const Vector4 &colour)
{
	MFSetColour(colour.x, colour.y, colour.z, colour.w);
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

void MFSetNormal(const Vector3 &normal)
{
	MFSetNormal(normal.x, normal.y, normal.z);
}

void MFSetNormal(float x, float y, float z)
{
}

void MFSetPosition(const Vector3 &pos)
{
	MFSetPosition(pos.x, pos.y, pos.z);
}

void MFSetPosition(float x, float y, float z)
{
	CALLSTACK;

	++currentVert;
}

void MFEnd()
{
	CALLSTACK;

	DBGASSERT(currentVert == beginCount, "Incorrect number of vertices.");
}
