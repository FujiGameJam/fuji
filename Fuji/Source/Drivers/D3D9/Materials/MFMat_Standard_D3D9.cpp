#include "Fuji.h"

#if MF_RENDERER == MF_DRIVER_D3D9 || defined(MF_RENDERPLUGIN_D3D9)

#if defined(MF_RENDERPLUGIN_D3D9)
	#define MFMat_Standard_RegisterMaterial MFMat_Standard_RegisterMaterial_D3D9
	#define MFMat_Standard_UnregisterMaterial MFMat_Standard_UnregisterMaterial_D3D9
	#define MFMat_Standard_Begin MFMat_Standard_Begin_D3D9
	#define MFMat_Standard_CreateInstancePlatformSpecific MFMat_Standard_CreateInstancePlatformSpecific_D3D9
	#define MFMat_Standard_DestroyInstancePlatformSpecific MFMat_Standard_DestroyInstancePlatformSpecific_D3D9
#endif

#include "MFHeap.h"
#include "MFRenderer_Internal.h"
#include "MFRenderState_Internal.h"
#include "MFTexture_Internal.h"
#include "MFMaterial_Internal.h"
#include "MFDisplay_Internal.h"
#include "MFView_Internal.h"
#include "../MFRenderer_D3D9.h"
#include "Materials/MFMat_Standard_Internal.h"

#include "../Shaders/Registers.h"
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

static const DWORD sFilterModes[] =
{
    D3DTEXF_NONE,
    D3DTEXF_POINT,
    D3DTEXF_LINEAR,
    D3DTEXF_ANISOTROPIC
};

static const DWORD sAddressModes[] =
{
    D3DTADDRESS_WRAP,
    D3DTADDRESS_MIRROR,
    D3DTADDRESS_CLAMP,
    D3DTADDRESS_BORDER,
    D3DTADDRESS_MIRRORONCE
};

static const DWORD sBlendOp[] =
{
    D3DBLENDOP_ADD,
    D3DBLENDOP_SUBTRACT,
    D3DBLENDOP_REVSUBTRACT,
    D3DBLENDOP_MIN,
    D3DBLENDOP_MAX
};

static const DWORD sBlendArg[] =
{
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
	D3DBLEND_SRCALPHASAT,
	D3DBLEND_BLENDFACTOR,
	D3DBLEND_INVBLENDFACTOR,
	D3DBLEND_SRCCOLOR2,
	D3DBLEND_INVSRCCOLOR2,
	D3DBLEND_ZERO,
	D3DBLEND_ZERO,
};

static const DWORD sCullMode[] =
{
	D3DCULL_NONE,
	D3DCULL_CCW,
	D3DCULL_CW,
};

static const DWORD sCompareFunc[] =
{
	D3DCMP_NEVER,
	D3DCMP_LESS,
	D3DCMP_EQUAL,
	D3DCMP_LESSEQUAL,
	D3DCMP_GREATER,
	D3DCMP_NOTEQUAL,
	D3DCMP_GREATEREQUAL,
	D3DCMP_ALWAYS,
};

int MFMat_Standard_RegisterMaterial(MFMaterialType *pType)
{
	MFCALLSTACK;

	pd3dDevice->CreateVertexShader(reinterpret_cast<const DWORD*>(g_vs_main_s), &pVS_s);
	pd3dDevice->CreateVertexShader(reinterpret_cast<const DWORD*>(g_vs_main_a), &pVS_a);

	return 0;
}

void MFMat_Standard_UnregisterMaterial()
{
	MFCALLSTACK;

	pVS_s->Release();
	pVS_a->Release();
}

int MFMat_Standard_Begin(MFMaterial *pMaterial, MFRendererState &state)
{
	MFCALLSTACK;

	MFMat_Standard_Data *pData = (MFMat_Standard_Data*)pMaterial->pInstanceData;

	if(&state != NULL)
	{
		if(state.pMatrixStatesSet[MFSCM_WorldViewProjection] != state.pMatrixStates[MFSCM_WorldViewProjection])
		{
			MFMatrix *pWVP = state.getDerivedMatrix(MFSCM_WorldViewProjection);
			state.pMatrixStates[MFSCM_WorldViewProjection] = pWVP;

			MFMatrix mat;
			mat.Transpose(*pWVP);
			pd3dDevice->SetVertexShaderConstantF(r_wvp, (float*)&mat, 4);
		}

		if(state.pMatrixStatesSet[MFSCM_UV0] != state.pMatrixStates[MFSCM_UV0])
		{
			MFMatrix *pUV0 = state.pMatrixStates[MFSCM_UV0];
			state.pMatrixStatesSet[MFSCM_UV0] = pUV0;

			MFMatrix mat;
			mat.Transpose(*pUV0);
			pd3dDevice->SetVertexShaderConstantF(r_tex, (float*)&mat, 2);
		}

		if(state.pVectorStatesSet[MFSCV_MaterialDiffuseColour] != state.pVectorStates[MFSCV_MaterialDiffuseColour])
		{
			MFVector *pDiffuseColour = state.pVectorStates[MFSCV_MaterialDiffuseColour];
			state.pVectorStatesSet[MFSCV_MaterialDiffuseColour] = pDiffuseColour;

			pd3dDevice->SetVertexShaderConstantF(r_modelColour, (float*)pDiffuseColour, 1);
		}

		bool bDetailPresent = state.isSet(MFSB_CT_Bool, MFSCB_DetailMapSet);
		bool bDiffusePresent = state.isSet(MFSB_CT_Bool, MFSCB_DiffuseSet);

		if(bDetailPresent)
		{
			// set detail map
			MFTexture *pDetail = state.pTextures[MFSCT_DetailMap];
			if(state.pTexturesSet[MFSCT_DetailMap] != pDetail)
			{
				state.pTexturesSet[MFSCT_DetailMap] = pDetail;
				pd3dDevice->SetTexture(1, (IDirect3DTexture9*)pDetail->pInternalData);
			}

			// set detail map sampler
			MFSamplerState *pDetailSamp = (MFSamplerState*)state.pRenderStates[MFSCRS_DetailMapSamplerState];
			if(state.pRenderStatesSet[MFSCRS_DetailMapSamplerState] != pDetailSamp)
			{
				state.pRenderStatesSet[MFSCRS_DetailMapSamplerState] = pDetailSamp;

				pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, sFilterModes[pDetailSamp->stateDesc.minFilter]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, sFilterModes[pDetailSamp->stateDesc.magFilter]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, sFilterModes[pDetailSamp->stateDesc.mipFilter]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, sAddressModes[pDetailSamp->stateDesc.addressU]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, sAddressModes[pDetailSamp->stateDesc.addressV]);
			}
		}
		else
		{
			if(state.pTexturesSet[MFSCT_DetailMap] != NULL)
			{
				state.pTexturesSet[MFSCT_DetailMap] = NULL;
				pd3dDevice->SetTexture(1, NULL);
			}
		}

		if(bDiffusePresent)
		{
			// set diffuse map
			MFTexture *pDiffuse = state.pTextures[MFSCT_Diffuse];
			if(state.pTexturesSet[MFSCT_Diffuse] != pDiffuse)
			{
				state.pTexturesSet[MFSCT_Diffuse] = pDiffuse;
				pd3dDevice->SetTexture(0, (IDirect3DTexture9*)pDiffuse->pInternalData);
			}

			// set diffuse map sampler
			MFSamplerState *pDiffuseSamp = (MFSamplerState*)state.pRenderStates[MFSCRS_DiffuseSamplerState];
			if(state.pRenderStatesSet[MFSCRS_DiffuseSamplerState] != pDiffuseSamp)
			{
				state.pRenderStatesSet[MFSCRS_DiffuseSamplerState] = pDiffuseSamp;

				pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, sFilterModes[pDiffuseSamp->stateDesc.minFilter]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, sFilterModes[pDiffuseSamp->stateDesc.magFilter]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, sFilterModes[pDiffuseSamp->stateDesc.mipFilter]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, sAddressModes[pDiffuseSamp->stateDesc.addressU]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, sAddressModes[pDiffuseSamp->stateDesc.addressV]);
			}
		}
		else
		{
			if(state.pTexturesSet[MFSCT_Diffuse] != NULL)
			{
				state.pTexturesSet[MFSCT_Diffuse] = NULL;
				pd3dDevice->SetTexture(0, NULL);
			}
		}

		// configure the texture combiner (can we cache this?)
		if(state.boolChanged(MFSCB_DetailMapSet) || state.boolChanged(MFSCB_DiffuseSet) || state.boolChanged(MFSCB_User0))
		{
			const uint32 mask = MFBIT(MFSCB_DetailMapSet) | MFBIT(MFSCB_DiffuseSet) | MFBIT(MFSCB_User0);
			state.boolsSet = (state.boolsSet & ~mask) | (state.bools & mask);

			if(bDetailPresent)
			{
				pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
				pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			}
			else
			{
				pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

				if(state.getBool(MFSCB_User0))
				{
					// we need to scale the colour intensity by the vertex alpha since it wont happen during the blend.
					pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
					pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

//					pd3dDevice->SetTextureStageState(1, D3DTSS_CONSTANT, 0);

					pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
					pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEMP);
					pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				}
				else
				{
					pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
					pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				}
			}
		}

		// blend state
		MFBlendState *pBlendState = (MFBlendState*)state.pRenderStates[MFSCRS_BlendState];
		if(state.pRenderStatesSet[MFSCRS_BlendState] != pBlendState)
		{
			state.pRenderStatesSet[MFSCRS_BlendState] = pBlendState;

			MFBlendStateDesc::RenderTargetBlendDesc &target = pBlendState->stateDesc.renderTarget[0];
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, target.bEnable ? TRUE : FALSE);
			if(target.bEnable)
			{
				pd3dDevice->SetRenderState(D3DRS_BLENDOP, sBlendOp[target.blendOp]);
				pd3dDevice->SetRenderState(D3DRS_SRCBLEND, sBlendArg[target.srcBlend]);
				pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, sBlendArg[target.srcBlendAlpha]);
				pd3dDevice->SetRenderState(D3DRS_DESTBLEND, sBlendArg[target.destBlend]);
				pd3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, sBlendArg[target.destBlendAlpha]);
			}
		}

		// rasteriser state
		MFRasteriserState *pRasteriserState = (MFRasteriserState*)state.pRenderStates[MFSCRS_RasteriserState];
		if(state.pRenderStatesSet[MFSCRS_RasteriserState] != pRasteriserState)
		{
			state.pRenderStatesSet[MFSCRS_RasteriserState] = pRasteriserState;

			pd3dDevice->SetRenderState(D3DRS_CULLMODE, sCullMode[pRasteriserState->stateDesc.cullMode]);
		}

		// depth/stencil state
		MFDepthStencilState *pDSState = (MFDepthStencilState*)state.pRenderStates[MFSCRS_DepthStencilState];
		if(state.pRenderStatesSet[MFSCRS_DepthStencilState] != pDSState)
		{
			state.pRenderStatesSet[MFSCRS_DepthStencilState] = pDSState;

			bool bDepthTest = pDSState->stateDesc.bDepthEnable;
			pd3dDevice->SetRenderState(D3DRS_ZENABLE, bDepthTest ? D3DZB_TRUE : D3DZB_FALSE);

			if(bDepthTest)
			{
				pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, pDSState->stateDesc.depthWriteMask == MFDepthWriteMask_Zero ? FALSE : TRUE);
				pd3dDevice->SetRenderState(D3DRS_ZFUNC, sCompareFunc[pDSState->stateDesc.depthFunc]);
			}
		}

		// setup alpha test
		if(state.boolChanged(MFSCB_AlphaTest))
		{
			state.boolsSet ^= MFBIT(MFSCB_AlphaTest);
			if(state.getBool(MFSCB_AlphaTest))
			{
				pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
				pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			}
			else
			{
				pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			}
		}

		// set alpha ref
		if(state.pVectorStatesSet[MFSCV_RenderState] != state.pVectorStates[MFSCV_RenderState] && state.getBool(MFSCB_AlphaTest))
		{
			MFVector *pRS = state.pVectorStates[MFSCV_RenderState];
			state.pVectorStatesSet[MFSCV_RenderState] = pRS;

			pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)MFClamp(0.f, pRS->x * 255.f, 255.f));
		}

		// set clour/alpha scales
		if(state.pVectorStatesSet[MFSCV_User0] != state.pVectorStates[MFSCV_User0])
		{
			MFVector *pMask = state.pVectorStates[MFSCV_User0];
			state.pVectorStatesSet[MFSCV_User0] = pMask;

			pd3dDevice->SetVertexShaderConstantF(r_colourMask, (float*)pMask, 1);
		}

		// set animation matrices
		if(state.getBool(MFSCB_Animated))
		{
//			for(int b=0; b<gNumBonesInBatch; b++)
//				MFRendererPC_SetAnimationMatrix(b, pAnimMats[pCurrentBatch[b]]);

			MFRendererPC_SetVertexShader(pVS_a);
		}
		else
			MFRendererPC_SetVertexShader(pVS_s);
	}
	else
	{
		if(pSetMaterial != pMaterial)
		{
			bool premultipliedAlpha = false;

			// set some render states
			if(pData->detailMapIndex)
			{
				// HACK: for compound multitexturing
				MFMat_Standard_Data::Texture &diffuse = pData->textures[pData->diffuseMapIndex];
				MFMat_Standard_Data::Texture &detail = pData->textures[pData->detailMapIndex];

				IDirect3DTexture9 *pDiffuse = (IDirect3DTexture9*)diffuse.pTexture->pInternalData;
				IDirect3DTexture9 *pDetail = (IDirect3DTexture9*)detail.pTexture->pInternalData;

				pd3dDevice->SetTexture(0, pDetail);
				pd3dDevice->SetTexture(1, pDiffuse);

				// set the mip filter to nearest!!!
				pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, sFilterModes[detail.minFilter]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, sFilterModes[detail.magFilter]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, sFilterModes[detail.mipFilter]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, sFilterModes[diffuse.minFilter]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, sFilterModes[diffuse.magFilter]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, sFilterModes[diffuse.mipFilter]);

				pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, sAddressModes[detail.addressU]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, sAddressModes[detail.addressV]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, sAddressModes[diffuse.addressU]);
				pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, sAddressModes[diffuse.addressV]);

				pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
				pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

				MFRendererPC_SetTextureMatrix(pData->textureMatrix);
			}
			else if(pData->textures[pData->diffuseMapIndex].pTexture)
			{
				MFMat_Standard_Data::Texture &diffuse = pData->textures[pData->diffuseMapIndex];

				IDirect3DTexture9 *pTexture = (IDirect3DTexture9*)diffuse.pTexture->pInternalData;

				pd3dDevice->SetTexture(0, pTexture);
				pd3dDevice->SetTexture(1, NULL);

				pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, sFilterModes[diffuse.minFilter]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, sFilterModes[diffuse.magFilter]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, sFilterModes[diffuse.mipFilter]);

				pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, sAddressModes[diffuse.addressU]);
				pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, sAddressModes[diffuse.addressV]);

				pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

				MFRendererPC_SetTextureMatrix(pData->textureMatrix);

				premultipliedAlpha = !!(pData->textures[pData->diffuseMapIndex].pTexture->pTemplateData->flags & TEX_PreMultipliedAlpha);

				if(premultipliedAlpha)
				{
					// we need to scale the colour intensity by the vertex alpha since it wont happen during the blend.
					pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
					pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	//				pd3dDevice->SetTextureStageState(1, D3DTSS_CONSTANT, 0);

					pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
					pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEMP);
					pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
				}
				else
				{
					pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
					pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				}
			}
			else
			{
				pd3dDevice->SetTexture(0, NULL);
			}

			switch(pData->materialType&MF_BlendMask)
			{
				case 0:
					pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
					break;
				case MF_AlphaBlend:
					pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					pd3dDevice->SetRenderState(D3DRS_SRCBLEND, premultipliedAlpha ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
					pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
					break;
				case MF_Additive:
					pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
					pd3dDevice->SetRenderState(D3DRS_SRCBLEND, premultipliedAlpha ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
					pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					break;
				case MF_Subtractive:
					pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
					pd3dDevice->SetRenderState(D3DRS_SRCBLEND, premultipliedAlpha ? D3DBLEND_ONE : D3DBLEND_SRCALPHA);
					pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
					break;
			}

			if(pData->materialType & MF_Mask)
			{
				pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
				pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0xFF);
			}
			else
			{
				pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			}

			switch((pData->materialType & MF_CullMode) >> 6)
			{
				case 0:
					pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
					break;
				case 1:
					pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
					break;
				case 2:
					pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
					break;
				case 3:
					// 'default' ?
					pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
					break;
			}

			if(!(pData->materialType&MF_NoZRead) || !(pData->materialType&MF_NoZWrite))
			{
				pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
				pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, (pData->materialType&MF_NoZWrite) ? FALSE : TRUE);
				pd3dDevice->SetRenderState(D3DRS_ZFUNC, (pData->materialType&MF_NoZRead) ? D3DCMP_ALWAYS : D3DCMP_LESSEQUAL);
			}
			else
			{
				pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
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
	}

	return 0;
}

void MFMat_Standard_CreateInstancePlatformSpecific(MFMaterial *pMaterial)
{
}

void MFMat_Standard_DestroyInstancePlatformSpecific(MFMaterial *pMaterial)
{
}

#endif // MF_RENDERER
