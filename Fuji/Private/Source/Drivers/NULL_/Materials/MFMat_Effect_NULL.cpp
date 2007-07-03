#include "Fuji.h"

#if MF_RENDERER == NULL

#include "MFMaterial_Internal.h"

int MFMat_Effect_RegisterMaterial(void *pPlatformData)
{
	return 0;
}

void MFMat_Effect_UnregisterMaterial()
{
}

int MFMat_Effect_Begin(MFMaterial *pMaterial)
{
	return 0;
}

void MFMat_Effect_CreateInstance(MFMaterial *pMaterial)
{
	pMaterial->pInstanceData = NULL;
}

void MFMat_Effect_DestroyInstance(MFMaterial *pMaterial)
{
}

void MFMat_Effect_SetParameter(MFMaterial *pMaterial, int parameterIndex, int argIndex, const void *pValue)
{
}

uint32 MFMat_Effect_GetParameter(MFMaterial *pMaterial, int parameterIndex, int argIndex, void *pValue)
{
	return 0;
}

int MFMat_Effect_GetNumParams()
{
	return 0;
}

MFMaterialParameterInfo* MFMat_Effect_GetParameterInfo(int parameterIndex)
{
	return NULL;
}

#endif
