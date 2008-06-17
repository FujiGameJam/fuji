#if !defined(_MFINTTEXTURE_H)
#define _MFINTTEXTURE_H

enum MFIntTextureFormat
{
	MFIMF_Unknown = -1,

	MFIMF_TGA = 0,
	MFIMF_BMP,
	MFIMF_PNG,

	MFIMF_Max,
	MFIMF_ForceInt = 0x7FFFFFFF
};

enum MFIntTextureFlags
{
	MFITF_PreMultipliedAlpha = 1,	// use pre-multiply alpha
	MFITF_FadeAlpha = 2				// fade the alpha to transparent across mip levels
};

struct MFTextureTemplateData;

// very high precision intermediate image format
struct MFIntTexturePixel
{
	float r,g,b,a;
};

struct MFIntTextureSurface
{
	int width, height;
	MFIntTexturePixel *pData;
};

struct MFIntTexture
{
	MFIntTextureSurface *pSurfaces;
	int numSurfaces;

	bool opaque;
	bool oneBitAlpha;
};

MFIntTexture *MFIntTexture_CreateFromFile(const char *pFilename);
MFIntTexture *MFIntTexture_CreateFromFileInMemory(const void *pMemory, uint32 size, MFIntTextureFormat format);

void MFIntTexture_Destroy(MFIntTexture *pTexture);

void MFIntTexture_CreateRuntimeData(MFIntTexture *pTexture, MFTextureTemplateData **ppTemplateData, uint32 *pSize, MFPlatform platform, uint32 flags = 0, MFTextureFormat targetFormat = TexFmt_Unknown);

void MFIntTexture_WriteToHeaderFile(MFIntTexture *pTexture, const char *pFilename);

void MFIntTexture_FilterMipMaps(MFIntTexture *pTexture, int numMipLevels, uint32 mipFilterOptions);
void MFIntTexture_ScanImage(MFIntTexture *pTexture);
void MFIntTexture_FlipImage(MFIntTexture *pTexture);

#endif
