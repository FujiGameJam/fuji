#if !defined(_MFMAT_EFFECT_H)
#define _MFMAT_EFFECT_H

#if defined(MF_WINDOWS)
#include <d3dx9.h>
#endif

struct MFMat_Effect_Data
{
#if defined(MF_WINDOWS)
	ID3DXEffect *pEffect;
#else
	void *pEffect;
#endif
};

int       MFMat_Effect_RegisterMaterial(void *pPlatformData);
void      MFMat_Effect_UnregisterMaterial();
void      MFMat_Effect_CreateInstance(MFMaterial *pMaterial);
void      MFMat_Effect_DestroyInstance(MFMaterial *pMaterial);
void      MFMat_Effect_Update(MFMaterial *pMaterial);
void      MFMat_Effect_BuildStateBlock(MFMaterial *pMaterial);
int       MFMat_Effect_Begin(MFMaterial *pMaterial);
void      MFMat_Effect_SetParameter(MFMaterial *pMaterial, int parameterIndex, int argIndex, size_t value);
uintp     MFMat_Effect_GetParameter(MFMaterial *pMaterial, int parameterIndex, int argIndex, void *pValue);
int       MFMat_Effect_GetNumParams();
MFMaterialParameterInfo* MFMat_Effect_GetParameterInfo(int parameterIndex);

#endif // _MFMAT_EFFECT_H
