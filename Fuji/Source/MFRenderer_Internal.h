#pragma once
#if !defined(_MFRENDERER_INTERNAL_H)
#define _MFRENDERER_INTERNAL_H

#include "MFRenderer.h"
#include "MFRenderState.h"
#include "MFArray.h"

// init/deinit
MFInitStatus MFRenderer_InitModule();
void MFRenderer_DeinitModule();

void MFRenderer_InitModulePlatformSpecific();
void MFRenderer_DeinitModulePlatformSpecific();

bool MFRenderer_BeginFrame();
void MFRenderer_EndFrame();

int MFRenderer_CreateDisplay();
void MFRenderer_DestroyDisplay();
void MFRenderer_ResetDisplay();
bool MFRenderer_SetDisplayMode(int width, int height, bool bFullscreen);

typedef int (*MFRenderSortFunction)(const void*, const void*);

enum MFStateBlockType
{
	MFSBT_Global = 0,
	MFSBT_View,
	MFSBT_Entity,
	MFSBT_Material,
	MFSBT_Geometry,
	MFSBT_MaterialOverride,
	MFSBT_Layer,
	MFSBT_Override,

	MFSBT_Max,
	MFSBT_ForceInt = 0x7FFFFFFF,
};

MFALIGN_BEGIN(16)
struct MFRenderElement // 42 bytes atm... compress state block handles?
{
	// sorting (cost):
	//   dx9  - shaders, shader constants, render targets, decl, sampler state, vert/index buffer, textures, render states, drawprim
	//        - pShader, pView, pEntity, pMaterial ???
	//   dx11 - textures...
	//        - pMaterial, pShader, pView, pEntity ???

//	MFRenderElementData *pData;
	uint8 type;
	uint8 primarySortKey;
	uint16 zSort;

	MFMaterial *pMaterial;
//	MFRenderTechnique *pRenderTechnique;

	MFStateBlock *pViewState;
	MFStateBlock *pGeometryState;
	MFStateBlock *pEntityState;
	MFStateBlock *pMaterialState;
	MFStateBlock *pMaterialOverrideState;

	uint64 vertexBufferOffset : 20;
	uint64 indexBufferOffset : 20;
	uint64 vertexCount : 20;
	uint64 primType : 3;
	uint64 renderIndexed : 1;

//	MFRenderInstance *pInstances;
	uint8 numInstances;

	uint8 animBatch;

	uint16 unused[3];

//	MFRenderElementDebug *pDbg; // name/model/event/etc...
}
MFALIGN_END(16);

struct MFRenderLayer
{
	MFVector clearColour;
	float clearZ;
	int clearStencil;
	MFRenderClearFlags clearFlags;

	// render target config

	MFRenderLayerSortMode sortMode;
	MFStateBlock *pLayer;

	MFArray<MFRenderElement> elements;

	int primaryKey;

	const char *pName;
};

struct MFRenderer
{
	// layers
	MFRenderLayer *pLayers;
	int numLayers;

	MFStateBlock *pGlobal;
	MFStateBlock *pOverride;
};

struct MFRendererState
{
	__forceinline bool isSet(int type, int constant)
	{
		return !!(rsSet[type] & MFBIT(constant));
	}

	__forceinline uint32 getBool(int constant)
	{
		return rsSet[MFSB_CT_Bool] & bools & MFBIT(constant);
	}

	__forceinline uint32 boolChanged(int constant)
	{
		return (boolsSet ^ (rsSet[MFSB_CT_Bool] & bools)) & MFBIT(constant);
	}

	MFMatrix *getDerivedMatrix(MFStateConstant_Matrix matrix);

	MFMatrix derivedMatrices[MFSCM_NumDerived];
	uint32 derivedMatrixDirty;

	uint32 bools, boolsSet;

	MFStateBlock *pStateBlocks[MFSBT_Max];

	MFMatrix *pMatrixStates[MFSCM_Max];
	MFMatrix *pMatrixStatesSet[MFSCM_Max];
	MFVector *pVectorStates[MFSCV_Max];
	MFVector *pVectorStatesSet[MFSCV_Max];
	MFTexture *pTextures[MFSCT_Max];
	MFTexture *pTexturesSet[MFSCT_Max];
	void *pRenderStates[MFSCRS_Max];
	void *pRenderStatesSet[MFSCRS_Max];

	uint32 rsSet[MFSB_CT_TypeCount];
	uint32 rsMask[MFSB_CT_TypeCount];
};

// internal functions
void MFRendererInternal_SortElements(MFRenderLayer &layer);


#endif
