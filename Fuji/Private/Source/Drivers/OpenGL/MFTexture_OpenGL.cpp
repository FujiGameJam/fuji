#include "Fuji.h"

#if MF_RENDERER == MF_DRIVER_OPENGL || defined(MF_RENDERPLUGIN_OPENGL)

#if defined(MF_RENDERPLUGIN_OPENGL)
	#define MFTexture_InitModulePlatformSpecific MFTexture_InitModulePlatformSpecific_OpenGL
	#define MFTexture_DeinitModulePlatformSpecific MFTexture_DeinitModulePlatformSpecific_OpenGL
	#define MFTexture_CreatePlatformSpecific MFTexture_CreatePlatformSpecific_OpenGL
	#define MFTexture_CreateRenderTarget MFTexture_CreateRenderTarget_OpenGL
	#define MFTexture_Destroy MFTexture_Destroy_OpenGL
#endif

/**** Includes ****/

#include "MFSystem.h"
#include "MFTexture_Internal.h"
#include "Display_Internal.h"
#include "MFFileSystem_Internal.h"
#include "MFPtrList.h"

#include "MFOpenGL.h"

/**** Globals ****/

extern MFPtrListDL<MFTexture> gTextureBank;
extern MFTexture *pNoneTexture;

struct GLFormat
{
	GLint internalFormat;
	GLenum format;
	GLenum type;
};

#if !defined(MF_OPENGL_ES)
// opengl format table.. man, opengl's texture format management is a complete shambles!!!!
// i havent verified that all of these formats are correct, some might be wrong...
GLFormat gGLFormats[] =
{
	{ GL_RGBA8, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV },						// TexFmt_A8R8G8B8
	{ GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV },							// TexFmt_A8B8G8R8
	{ GL_RGBA8, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8 },							// TexFmt_B8G8R8A8
	{ GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8 },								// TexFmt_R8G8B8A8
	{ GL_RGB10_A2, GL_BGRA_EXT, GL_UNSIGNED_INT_2_10_10_10_REV },				// TexFmt_A2R10G10B10
	{ GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV },					// TexFmt_A2B10G10R10
	{ GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT },									// TexFmt_A16B16G16R16
	{ GL_RGB5, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 },								// TexFmt_R5G6B5
																				// TexFmt_R6G5B5
	{ GL_RGB5, GL_BGR_EXT, GL_UNSIGNED_SHORT_5_6_5_REV },						// TexFmt_B5G6R5
	{ GL_RGB5_A1, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV },					// TexFmt_A1R5G5B5
	{ GL_RGB5_A1, GL_ABGR_EXT, GL_UNSIGNED_SHORT_5_5_5_1 },						// TexFmt_R5G5B5A1
	{ GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 },							// TexFmt_A1B5G5R5
	{ GL_RGBA4, GL_BGRA_EXT, GL_UNSIGNED_SHORT_4_4_4_4_REV },					// TexFmt_A4R4G4B4
	{ GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 },							// TexFmt_A4B4G4R4
	{ GL_RGBA4, GL_ABGR_EXT, GL_UNSIGNED_SHORT_4_4_4_4 },						// TexFmt_R4G4B4A4
	{ GL_RGBA16F_ARB, GL_RGBA, GL_HALF_FLOAT_ARB },								// TexFmt_ABGR_F16
	{ GL_RGBA32F_ARB, GL_RGBA, GL_FLOAT },										// TexFmt_ABGR_F32
	{ GL_COLOR_INDEX8_EXT, GL_COLOR_INDEX, GL_UNSIGNED_BYTE },					// TexFmt_I8
																				// TexFmt_I4
	{ GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0 },	// TexFmt_DXT1
																				// TexFmt_DXT2
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0 },	// TexFmt_DXT3
																				// TexFmt_DXT4
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0 }	// TexFmt_DXT5
};
#else
// opengl format table.. man, opengl's texture format management is a complete shambles!!!!
// i havent verified that all of these formats are correct, some might be wrong...
GLFormat gGLFormats[] =
{
	{ 0, 0, 0 },										// TexFmt_A8R8G8B8
	{ 0, GL_RGBA, 0 },									// TexFmt_A8B8G8R8
	{ 0, 0, 0 },										// TexFmt_B8G8R8A8
	{ 0, GL_RGBA, 0 },									// TexFmt_R8G8B8A8
	{ 0, 0, 0 },										// TexFmt_A2R10G10B10
	{ 0, GL_RGBA, 0 },									// TexFmt_A2B10G10R10
	{ 0, GL_RGBA, GL_UNSIGNED_SHORT },					// TexFmt_A16B16G16R16
														// TexFmt_R6G5B5
	{ GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 },		// TexFmt_R5G6B5
	{ GL_RGB565, 0, 0 },								// TexFmt_B5G6R5
	{ GL_RGB5_A1, 0, 0 },								// TexFmt_A1R5G5B5
	{ GL_RGB5_A1, 0, GL_UNSIGNED_SHORT_5_5_5_1 },		// TexFmt_R5G5B5A1
	{ GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1 },	// TexFmt_A1B5G5R5
	{ GL_RGBA4, 0, GL_UNSIGNED_SHORT_4_4_4_4_REV },		// TexFmt_A4R4G4B4
	{ GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4 },	// TexFmt_A4B4G4R4
	{ GL_RGBA4, 0, GL_UNSIGNED_SHORT_4_4_4_4 },			// TexFmt_R4G4B4A4
	{ 0, GL_RGBA, 0 },									// TexFmt_ABGR_F16
	{ 0, GL_RGBA, GL_FLOAT },							// TexFmt_ABGR_F32
	{ 0, 0, GL_UNSIGNED_BYTE },							// TexFmt_I8
														// TexFmt_I4
	{ 0, 0, 0 },										// TexFmt_DXT1
														// TexFmt_DXT2
	{ 0, 0, 0 },										// TexFmt_DXT3
														// TexFmt_DXT4
	{ 0, 0, 0 }											// TexFmt_DXT5
};
#endif

static const int gMaxGLFormats = sizeof(gGLFormats) / sizeof(GLFormat);

/**** Functions ****/

void MFTexture_InitModulePlatformSpecific()
{
}

void MFTexture_DeinitModulePlatformSpecific()
{
}

// interface functions
void MFTexture_CreatePlatformSpecific(MFTexture *pTexture, bool generateMipChain)
{
	MFCALLSTACK;

	MFTextureTemplateData *pTemplate = pTexture->pTemplateData;

	GLuint textureID;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	pTexture->pInternalData = (void*)textureID;

	uint32 platformFormat = MFTexture_GetPlatformFormatID(pTemplate->imageFormat, MFDD_OpenGL);
	MFDebug_Assert(platformFormat < (uint32)gMaxGLFormats, "Platform format is undefined...");

	GLFormat &format = gGLFormats[platformFormat];

	if(generateMipChain && pTemplate->mipLevels == 1)
	{
		MFTextureSurfaceLevel *pSurf = &pTemplate->pSurfaces[0];
		glTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, pSurf->width, pSurf->height, 0, format.format, format.type, pSurf->pImageData);

		int numMips = 0;
		// build mips from the top surface..

		// set this to however many mips we just constructed
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numMips);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, numMips ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pTemplate->mipLevels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pTemplate->mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

		for(int a=0; a<pTemplate->mipLevels; a++)
		{
			MFTextureSurfaceLevel *pSurf = &pTemplate->pSurfaces[a];
			glTexImage2D(GL_TEXTURE_2D, a, format.internalFormat, pSurf->width, pSurf->height, 0, format.format, format.type, pSurf->pImageData);
		}
	}
}

MFTexture* MFTexture_CreateRenderTarget(const char *pName, int width, int height)
{
	MFCALLSTACK;

	MFDebug_Assert(false, "Not Written...");

	return NULL;
}

int MFTexture_Destroy(MFTexture *pTexture)
{
	MFCALLSTACK;

	pTexture->refCount--;

	// if no references left, destroy texture
	if(!pTexture->refCount)
	{
		GLuint *pTextures = (GLuint*)&pTexture->pInternalData;
		glDeleteTextures(1, pTextures);

		MFHeap_Free(pTexture->pTemplateData);
		gTextureBank.Destroy(pTexture);

		return 0;
	}

	return pTexture->refCount;
}

#endif
