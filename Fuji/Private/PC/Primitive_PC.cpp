#include "Fuji.h"
#include "Display_Internal.h"
#include "MFView.h"
#include "MFVector.h"
#include "MFMatrix.h"
#include "Primitive.h"
#include "MFTexture.h"
#include "Renderer.h"
#include "MFMaterial.h"

#include <d3d9.h>

struct LitVertex
{
	enum
	{
		FVF = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1
	};

	struct LitPos
	{
		float x, y, z;
	} pos;

	struct LitNormal
	{
		float x, y, z;
	} normal;

	unsigned int colour;

	float u,v;
};

LitVertex primBuffer[1024];
LitVertex current;

uint32 primType;
uint32 beginCount;
uint32 currentVert;

extern IDirect3DDevice9 *pd3dDevice;

/*** functions ***/

void Primitive_InitModule()
{
	CALLSTACK;

}

void Primitive_DeinitModule()
{
	CALLSTACK;

}

void DrawMFPrimitiveStats()
{

}

void MFPrimitive(uint32 type, uint32 hint)
{
	CALLSTACK;

	primType = type & PT_PrimMask;

	if(type & PT_Untextured)
	{
		MFMaterial_SetMaterial(MFMaterial_GetStockMaterial(MFMat_White));
	}

	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&MFMatrix::identity);
	pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&MFView_GetViewToScreenMatrix());

	if(MFView_IsOrtho())
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&MFMatrix::identity);
	else
		pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&MFView_GetWorldToViewMatrix());

	Renderer_Begin();
}

void MFBegin(uint32 vertexCount)
{
	CALLSTACK;

	beginCount = vertexCount;
	currentVert = 0;

	current.u = current.v = 0.0f;
	current.colour = 0xFFFFFFFF;
	current.normal.x = current.normal.z = 0.0f;
	current.normal.y = 1.0f;
}

void MFSetMatrix(const MFMatrix &mat)
{
	CALLSTACK;

	pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&mat);
}

void MFSetColour(const MFVector &colour)
{
	MFSetColour(colour.x, colour.y, colour.z, colour.w);
}

void MFSetColour(float r, float g, float b, float a)
{
	current.colour = ((uint32)(r*255.0f))<<16 | ((uint32)(g*255.0f))<<8 | (uint32)(b*255.0f) | ((uint32)(a*255.0f))<<24;
}

void MFSetColour(uint32 col)
{
	current.colour = col;
}

void MFSetTexCoord1(float u, float v)
{
	current.u = u;
	current.v = v;
}

void MFSetNormal(const MFVector &normal)
{
	MFSetNormal(normal.x, normal.y, normal.z);
}

void MFSetNormal(float x, float y, float z)
{
	current.normal.x = x;
	current.normal.y = y;
	current.normal.z = z;
}

void MFSetPosition(const MFVector &pos)
{
	MFSetPosition(pos.x, pos.y, pos.z);
}

void MFSetPosition(float x, float y, float z)
{
	CALLSTACK;

	current.pos.x = x;
	current.pos.y = y;
	current.pos.z = z;

	primBuffer[currentVert] = current;
	++currentVert;
}

void MFEnd()
{
	CALLSTACK;

	DBGASSERT(currentVert == beginCount, "Incorrect number of vertices.");

	pd3dDevice->SetFVF(LitVertex::FVF);

	switch(primType)
	{
		case PT_PointList:
			pd3dDevice->DrawPrimitiveUP(D3DPT_POINTLIST, beginCount, primBuffer, sizeof(LitVertex));
			break;
		case PT_LineList:
			pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, beginCount/2, primBuffer, sizeof(LitVertex));
			break;
		case PT_LineStrip:
			pd3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, beginCount-1, primBuffer, sizeof(LitVertex));
			break;
		case PT_TriList:
			pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, beginCount/3, primBuffer, sizeof(LitVertex));
			break;
		case PT_TriStrip:
			pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, beginCount-2, primBuffer, sizeof(LitVertex));
			break;
		case PT_TriFan:
			pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, beginCount-2, primBuffer, sizeof(LitVertex));
			break;
	}
}
