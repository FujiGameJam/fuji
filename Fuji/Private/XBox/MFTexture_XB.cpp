/**** Defines ****/

/**** Includes ****/

#include "Common.h"
#include "MFTexture_Internal.h"
#include "Display_Internal.h"
#include "MFFileSystem_Internal.h"
#include "PtrList.h"

#include <xgraphics.h>

/**** Globals ****/

extern PtrListDL<MFTexture> gTextureBank;
extern MFTexture *pNoneTexture;

/**** Functions ****/

MFTexture* MFTexture_Create(const char *pName, bool generateMipChain)
{
	MFTexture *pTexture = MFTexture_FindTexture(pName);

	if(!pTexture)
	{
		const char *pFileName = STR("%s.tga", pName);

		uint32 fileSize;
		char *pBuffer = MFFileSystem_Load(pFileName, &fileSize);

		if(!pBuffer)
		{
			LOGD(STR("Texture '%s' does not exist. Using '_None'.\n", pFileName));
			return MFTexture_Create("_None");
		}

		pTexture = gTextureBank.Create();
		pTexture->refCount = 0;

		D3DSURFACE_DESC imageDesc;
		HRESULT hr;

		hr = D3DXCreateTextureFromFileInMemoryEx(pd3dDevice, pBuffer, fileSize, 0, 0, generateMipChain ? 0 : 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture->pTexture);

		DBGASSERT(hr != D3DERR_NOTAVAILABLE, STR("LoadTexture failed: D3DERR_NOTAVAILABLE, 0x%08X", hr));
		DBGASSERT(hr != D3DERR_OUTOFVIDEOMEMORY, STR("LoadTexture failed: D3DERR_OUTOFVIDEOMEMORY, 0x%08X", hr));
		DBGASSERT(hr != D3DERR_INVALIDCALL, STR("LoadTexture failed: D3DERR_INVALIDCALL, 0x%08X", hr));
		DBGASSERT(hr != D3DXERR_INVALIDDATA, STR("LoadTexture failed: D3DXERR_INVALIDDATA, 0x%08X", hr));

		DBGASSERT(hr == D3D_OK, STR("Failed to create texture '%s'.", pFileName));

		Heap_Free(pBuffer);

		strcpy(pTexture->name, pName);

		pTexture->pTexture->GetLevelDesc(0, &imageDesc);

		// create template data
		uint32 levelCount = pTexture->pTexture->GetLevelCount();

		char *pTemplate = (char*)Heap_Alloc(sizeof(MFTextureTemplateData) + sizeof(MFTextureSurfaceLevel)*levelCount);

		pTexture->pTemplateData = (MFTextureTemplateData*)pTemplate;
		pTexture->pTemplateData->pSurfaces = (MFTextureSurfaceLevel*)(pTemplate + sizeof(MFTextureTemplateData));

		pTexture->pTexture->GetLevelDesc(0, &imageDesc);
		pTexture->pTemplateData->imageFormat = TexFmt_Unknown;
		pTexture->pTemplateData->platformFormat = imageDesc.Format;

		int a;

		for(a=0; a<TexFmt_Max; a++)
		{
			if((int)gMFTexturePlatformFormat[a] == imageDesc.Format)
			{
				pTexture->pTemplateData->imageFormat = (MFTextureFormats)a;
				break;
			}
		}

		pTexture->pTemplateData->mipLevels = levelCount;

		for(a=0; a<(int)levelCount; a++)
		{
			pTexture->pTexture->GetLevelDesc(a, &imageDesc);

			pTexture->pTemplateData->pSurfaces[a].width = imageDesc.Width;
			pTexture->pTemplateData->pSurfaces[a].height = imageDesc.Height;
		}
	}

	pTexture->refCount++;

	return pTexture;
}

MFTexture* MFTexture_CreateFromRawData(const char *pName, void *pData, int width, int height, MFTextureFormats format, uint32 flags, bool generateMipChain, uint32 *pPalette)
{
	MFTexture *pTexture = MFTexture_FindTexture(pName);

	if(!pTexture)
	{
		pTexture = gTextureBank.Create();
		pTexture->refCount = 0;

		D3DSURFACE_DESC imageDesc;
		HRESULT hr;
		D3DFORMAT fmt = D3DFMT_UNKNOWN;

		switch(format)
		{
			case TexFmt_Unknown:
				DBGASSERT(false, "Invalid Texture format: 'TEXF_Unknown'");
				break;
			case TexFmt_A8R8G8B8:
				fmt = D3DFMT_A8R8G8B8;
				break;
			default:
				DBGASSERT(false, "Texture format not yet supported..");
		}

		hr = pd3dDevice->CreateTexture(width, height, generateMipChain ? 0 : 1, 0, fmt, 0, &pTexture->pTexture);

		DBGASSERT(SUCCEEDED(hr), STR("CreateTexture failed: hr = 0x%08X", hr));
		if(FAILED(hr))
		{
			LOGD("Couldnt Create Texture");
			return NULL;
		}

		D3DLOCKED_RECT rect;

		pTexture->pTexture->LockRect(0, &rect, NULL, 0);

		switch(format)
		{
			case TexFmt_Unknown:
				DBGASSERT(false, "Invalid Texture format: 'TEXF_Unknown'");
				break;

			case TexFmt_A8R8G8B8:
			{
				if(0)//flags & TEX_VerticalMirror)
				{
					(char*&)pData += width*height*sizeof(uint32);

					for(int a=0; a<(int)height; a++)
					{
						(char*&)pData -= width*sizeof(uint32);
						memcpy(rect.pBits, pData, width*sizeof(uint32));
						(char*&)rect.pBits += rect.Pitch;
					}
				}
				else
				{
					XGSwizzleRect(pData, 0, NULL, rect.pBits, width, height, NULL, 4);
	//				memcpy(rect.pBits, pData, width*height*sizeof(uint32));
				}
				break;
			}

			default:
				DBGASSERT(false, "Texture format not yet supported..");
		}

		pTexture->pTexture->UnlockRect(0);

		strcpy(pTexture->name, pName);

		// create template data
		uint32 levelCount = pTexture->pTexture->GetLevelCount();

		char *pTemplate = (char*)Heap_Alloc(sizeof(MFTextureTemplateData) + sizeof(MFTextureSurfaceLevel)*levelCount);

		pTexture->pTemplateData = (MFTextureTemplateData*)pTemplate;
		pTexture->pTemplateData->pSurfaces = (MFTextureSurfaceLevel*)(pTemplate + sizeof(MFTextureTemplateData));

		pTexture->pTexture->GetLevelDesc(0, &imageDesc);
		pTexture->pTemplateData->imageFormat = format;
		pTexture->pTemplateData->platformFormat = imageDesc.Format;

		pTexture->pTemplateData->mipLevels = levelCount;

		for(int a=0; a<(int)levelCount; a++)
		{
			pTexture->pTexture->GetLevelDesc(a, &imageDesc);

			pTexture->pTemplateData->pSurfaces[a].width = imageDesc.Width;
			pTexture->pTemplateData->pSurfaces[a].height = imageDesc.Height;
		}
	}

	pTexture->refCount++;

	return pTexture;
}

MFTexture* MFTexture_CreateRenderTarget(const char *pName, int width, int height)
{
	DBGASSERT(false, "Not Written...");

	return NULL;
}

int MFTexture_Destroy(MFTexture *pTexture)
{
	pTexture->refCount--;

	// if no references left, destroy texture
	if(!pTexture->refCount)
	{
		pTexture->pTexture->Release();
		gTextureBank.Destroy(pTexture);

		return 0;
	}

	return pTexture->refCount;
}