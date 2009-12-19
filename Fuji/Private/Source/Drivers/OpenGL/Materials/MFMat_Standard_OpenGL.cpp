#include "Fuji.h"

#if MF_RENDERER == MF_DRIVER_OPENGL || defined(MF_RENDERPLUGIN_OPENGL)

#if defined(MF_RENDERPLUGIN_OPENGL)
	#define MFMat_Standard_RegisterMaterial MFMat_Standard_RegisterMaterial_OpenGL
	#define MFMat_Standard_UnregisterMaterial MFMat_Standard_UnregisterMaterial_OpenGL
	#define MFMat_Standard_Begin MFMat_Standard_Begin_OpenGL
	#define MFMat_Standard_CreateInstance MFMat_Standard_CreateInstance_OpenGL
	#define MFMat_Standard_DestroyInstance MFMat_Standard_DestroyInstance_OpenGL
#endif

#include "MFHeap.h"
#include "MFTexture_Internal.h"
#include "MFMaterial_Internal.h"
#include "Display_Internal.h"
#include "MFView_Internal.h"
#include "../../Source/Materials/MFMat_Standard.h"

#include "../MFOpenGL.h"

static MFMaterial *pSetMaterial = 0;

int MFMat_Standard_RegisterMaterial(void *pPlatformData)
{
	return 0;
}

void MFMat_Standard_UnregisterMaterial()
{
}

int MFMat_Standard_Begin(MFMaterial *pMaterial)
{
	MFCALLSTACK;

	MFMat_Standard_Data *pData = (MFMat_Standard_Data*)pMaterial->pInstanceData;

	if(pSetMaterial != pMaterial)
	{
	    bool premultipliedAlpha = false;

		// set some render states
		if(pData->pTextures[pData->diffuseMapIndex])
		{
			GLuint textureID = *(GLuint*)&pData->pTextures[pData->diffuseMapIndex]->pInternalData;
			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
//			glActiveTexture(0);
			premultipliedAlpha = !!(pData->pTextures[pData->diffuseMapIndex]->pTemplateData->flags & TEX_PreMultipliedAlpha);

			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf((GLfloat *)&pData->textureMatrix);
		}
		else
		{
//			glActiveTexture(texUnit);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		switch(pData->materialType&MF_BlendMask)
		{
			case 0:
				glDisable(GL_BLEND);
				break;
			case MF_AlphaBlend:
				glEnable(GL_BLEND);
				glBlendFunc(premultipliedAlpha ? GL_ONE : GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case MF_Additive:
				glEnable(GL_BLEND);
				glBlendFunc(premultipliedAlpha ? GL_ONE : GL_SRC_ALPHA, GL_ONE);
				break;
			case MF_Subtractive:
				glEnable(GL_BLEND);
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				break;
		}

		switch(pData->materialType&MF_CullMode)
		{
			case 0<<6:
				glDisable(GL_CULL_FACE);
				break;
			case 1<<6:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			case 2<<6:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			case 3<<6:
				// 'default' ?
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
		}

		glEnable(GL_DEPTH_TEST);
		glDepthFunc((pData->materialType&MF_NoZRead) ? GL_ALWAYS : GL_LEQUAL);
		glDepthMask((pData->materialType&MF_NoZWrite) ? 0 : 1);
	}

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

#endif
