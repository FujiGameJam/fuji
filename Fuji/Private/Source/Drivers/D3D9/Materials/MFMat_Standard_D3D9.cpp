#include "Fuji.h"

#if MF_RENDERER == MF_DRIVER_D3D9 || defined(MF_RENDERPLUGIN_D3D9)

#if defined(MF_RENDERPLUGIN_D3D9)
	#define MFMat_Standard_RegisterMaterial MFMat_Standard_RegisterMaterial_D3D9
	#define MFMat_Standard_UnregisterMaterial MFMat_Standard_UnregisterMaterial_D3D9
	#define MFMat_Standard_Begin MFMat_Standard_Begin_D3D9
	#define MFMat_Standard_CreateInstance MFMat_Standard_CreateInstance_D3D9
	#define MFMat_Standard_DestroyInstance MFMat_Standard_DestroyInstance_D3D9
#endif

#include "MFHeap.h"
#include "MFTexture_Internal.h"
#include "MFMaterial_Internal.h"
#include "Display_Internal.h"
#include "MFView_Internal.h"
#include "../MFRenderer_D3D9.h"
#include "Materials/MFMat_Standard.h"

#include "../Shaders/MatStandard_s.h"
#include "../Shaders/MatStandard_a.h"

static MFMaterial *pSetMaterial = 0;
extern uint32 renderSource;
extern uint32 currentRenderFlags;

extern IDirect3DDevice9 *pd3dDevice;
IDirect3DVertexShader9 *pVS_s = NULL;
IDirect3DVertexShader9 *pVS_a = NULL;

extern const MFMatrix *pAnimMats;
extern int gNumAnimMats;

extern const uint16 *pCurrentBatch;
extern int gNumBonesInBatch;


int MFMat_Standard_RegisterMaterial(void *pPlatformData)
{
	MFCALLSTACK;

	pd3dDevice->CreateVertexShader(g_vs11_main_s, &pVS_s);
	pd3dDevice->CreateVertexShader(g_vs11_main_a, &pVS_a);

	return 0;
}

void MFMat_Standard_UnregisterMaterial()
{
	MFCALLSTACK;

	pVS_s->Release();
	pVS_a->Release();
}

int MFMat_Standard_Begin(MFMaterial *pMaterial)
{
	MFCALLSTACK;

	MFMat_Standard_Data *pData = (MFMat_Standard_Data*)pMaterial->pInstanceData;

	if(pSetMaterial != pMaterial)
	{
		bool premultipliedAlpha = false;

		// set some render states
		if(pData->detailMapIndex)
		{
			// HACK: for compound multitexturing
			IDirect3DTexture9 *pDiffuse = (IDirect3DTexture9*)pData->pTextures[pData->diffuseMapIndex]->pInternalData;
			IDirect3DTexture9 *pDetail = (IDirect3DTexture9*)pData->pTextures[pData->detailMapIndex]->pInternalData;

			MFRendererPC_SetTexture(0, pDetail);
			MFRendererPC_SetTexture(1, pDiffuse);

			MFRendererPC_SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			MFRendererPC_SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			MFRendererPC_SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
			MFRendererPC_SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			MFRendererPC_SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			MFRendererPC_SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

			MFRendererPC_SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			MFRendererPC_SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
			MFRendererPC_SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			MFRendererPC_SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
			MFRendererPC_SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			MFRendererPC_SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

			MFRendererPC_SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			MFRendererPC_SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
			MFRendererPC_SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
			MFRendererPC_SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
			MFRendererPC_SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			MFRendererPC_SetTextureMatrix(pData->textureMatrix);
		}
		else if(pData->pTextures[pData->diffuseMapIndex])
		{
			IDirect3DTexture9 *pTexture = (IDirect3DTexture9*)pData->pTextures[pData->diffuseMapIndex]->pInternalData;

			MFRendererPC_SetTexture(0, pTexture);
			MFRendererPC_SetTexture(1, NULL);

			MFRendererPC_SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			MFRendererPC_SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			MFRendererPC_SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

			MFRendererPC_SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			MFRendererPC_SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			MFRendererPC_SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			MFRendererPC_SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

			MFRendererPC_SetTextureMatrix(pData->textureMatrix);

			premultipliedAlpha = !!(pData->pTextures[pData->diffuseMapIndex]->pTemplateData->flags & TEX_PreMultipliedAlpha);

			if(premultipliedAlpha)
			{
				// we need to scale the colour intensity by the vertex alpha since it wont happen during the blend.
				MFRendererPC_SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
				MFRendererPC_SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

				MFRendererPC_SetTextureStageState(1, D3DTSS_CONSTANT, 0);

				MFRendererPC_SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
				MFRendererPC_SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CONSTANT);
				MFRendererPC_SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
			}
			else
			{
				MFRendererPC_SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				MFRendererPC_SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}
		}
		else
		{
			MFRendererPC_SetTexture(0, NULL);
		}

		switch(pData->materialType&MF_BlendMask)
		{
			case 0:
				MFRendererPC_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				break;
			case MF_AlphaBlend:
				MFRendererPC_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				MFRendererPC_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				MFRendererPC_SetRenderState(D3DRS_SRCBLEND, premultipliedAlpha ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
				MFRendererPC_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				break;
			case MF_Additive:
				MFRendererPC_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				MFRendererPC_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				MFRendererPC_SetRenderState(D3DRS_SRCBLEND, premultipliedAlpha ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
				MFRendererPC_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				break;
			case MF_Subtractive:
				MFRendererPC_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				MFRendererPC_SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
				MFRendererPC_SetRenderState(D3DRS_SRCBLEND, premultipliedAlpha ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
				MFRendererPC_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				break;
		}

		if(pData->materialType & MF_Mask)
		{
			MFRendererPC_SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			MFRendererPC_SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			MFRendererPC_SetRenderState(D3DRS_ALPHAREF, 0xFF);
		}
		else
		{
			MFRendererPC_SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		}

		switch(pData->materialType&MF_CullMode)
		{
			case 0<<6:
				MFRendererPC_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				break;
			case 1<<6:
				MFRendererPC_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
				break;
			case 2<<6:
				MFRendererPC_SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
				break;
			case 3<<6:
				// 'default' ?
				MFRendererPC_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
				break;
		}

		if(!(pData->materialType&MF_NoZRead) || !(pData->materialType&MF_NoZWrite))
		{
			MFRendererPC_SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
			MFRendererPC_SetRenderState(D3DRS_ZWRITEENABLE, (pData->materialType&MF_NoZWrite) ? FALSE : TRUE);
			MFRendererPC_SetRenderState(D3DRS_ZFUNC, (pData->materialType&MF_NoZRead) ? D3DCMP_ALWAYS : D3DCMP_LESSEQUAL);
		}
		else
		{
			MFRendererPC_SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		}
	}

	MFRendererPC_SetColourMask(1, 0, 1, 0);

	// set animation matrices...
	if(pAnimMats && pCurrentBatch)
	{
		for(int b=0; b<gNumBonesInBatch; b++)
			MFRendererPC_SetAnimationMatrix(b, pAnimMats[pCurrentBatch[b]]);
	}

	if(MFRendererPC_GetNumWeights() > 0)
		MFRendererPC_SetVertexShader(pVS_a);
	else
		MFRendererPC_SetVertexShader(pVS_s);

	return 0;
}

void MFMat_Standard_CreateInstance(MFMaterial *pMaterial)
{
	MFCALLSTACK;

	pMaterial->pInstanceData = MFHeap_Alloc(sizeof(MFMat_Standard_Data));
	MFMat_Standard_Data *pData = (MFMat_Standard_Data*)pMaterial->pInstanceData;

	MFZeroMemory(pData, sizeof(MFMat_Standard_Data));

	pData->ambient = MFVector::one;
	pData->diffuse = MFVector::one;

	pData->materialType = MF_AlphaBlend | 1<<6 /* back face culling */;
	pData->opaque = true;

	pData->textureMatrix = MFMatrix::identity;
	pData->uFrames = 1;
	pData->vFrames = 1;

	pData->alphaRef = 1.0f;
}

void MFMat_Standard_DestroyInstance(MFMaterial *pMaterial)
{
	MFCALLSTACK;

	MFMat_Standard_Data *pData = (MFMat_Standard_Data*)pMaterial->pInstanceData;

	for(uint32 a=0; a<pData->textureCount; a++)
	{
		MFTexture_Destroy(pData->pTextures[a]);
	}

	MFHeap_Free(pMaterial->pInstanceData);
}

#endif // MF_RENDERER
