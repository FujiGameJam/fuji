#if !defined(_TEXTURE_INTERNAL_H)
#define _TEXTURE_INTERNAL_H

#if defined(_WINDOWS)
#include <d3d9.h>
#endif

#if defined(_LINUX) || defined(_OSX)
#include <GL/gl.h>
#endif

#include "MFTexture.h"

// foreward declarations
struct MFTexture;
struct MFTextureSurfaceLevel;

// functions
void MFTexture_InitModule();
void MFTexture_DeinitModule();

void MFTexture_CreatePlatformSpecific(MFTexture *pTexture, bool generateMipChain);

// texture TemplateData
struct MFTextureTemplateData
{
	uint32 magicNumber;

	MFTextureFormat imageFormat;
	uint32 platformFormat;

	int mipLevels;

	uint32 flags;

	// padding
	uint32 res[2];

	MFTextureSurfaceLevel *pSurfaces;
};

struct MFTextureSurfaceLevel
{
	int width, height;
	int bitsPerPixel;

	int xBlocks, yBlocks;
	int bitsPerBlock;

	char *pImageData;
	int bufferLength;

	char *pPaletteEntries;
	int paletteBufferLength;

	uint32 res[2];
};

// texture structure
struct MFTexture
{
	// data members
	char name[64];

#if defined(_MFXBOX)
#if defined(XB_XGTEXTURES)
	IDirect3DTexture8 texture;
#endif
	IDirect3DTexture8 *pTexture;
#elif defined(_WINDOWS)
	IDirect3DTexture9 *pTexture;
#elif defined(_LINUX) || defined(_OSX)
	GLuint textureID;
#elif defined(_PS2)
	unsigned int vramAddr;
#else
	// nothing
#endif

	MFTextureTemplateData *pTemplateData;
	int refCount;
};

#if !defined(_FUJI_UTIL)
// a debug menu texture information display object
#include "DebugMenu_Internal.h"

class TextureBrowser : public MenuObject
{
public:
	TextureBrowser();

	virtual void Draw();
	virtual void Update();

	virtual float ListDraw(bool selected, const MFVector &pos, float maxWidth);
	virtual void ListUpdate(bool selected);
	virtual MFVector GetDimensions(float maxWidth);

	int selection;
};
#endif

extern uint32 gMFTexturePlatformFormat[FP_Max][TexFmt_Max];

#endif
