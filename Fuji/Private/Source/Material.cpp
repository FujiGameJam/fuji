/**** Defines ****/

/**** Includes ****/

#include "Common.h"
#include "Display.h"
#include "Texture_Internal.h"
#include "Material_Internal.h"
#include "MFFileSystem.h"
#include "MFIni.h"

#include "SysLogo-256.h"
#include "SysLogo-64.h"


/**** internal functions ****/

void		MaterialInternal_Update(Material *pMaterial);
const char*	MaterialInternal_GetIDString(Material *pMaterial);

Material*	MaterialInternal_CreateDefault(const char *pName);
void		MaterialInternal_InitialiseFromDefinition(MFIni *pDefIni, Material *pMat, const char *pDefinition);


/**** Globals ****/

struct MaterialDefinition
{
	const char *pName;
	MFIni *pIni;
	bool ownsIni;

	MaterialDefinition *pNextDefinition;
};

void Material_DestroyDefinition(MaterialDefinition *pDefinition);

PtrListDL<MaterialDefinition> gMaterialDefList;
MaterialDefinition *pDefinitionRegistry = NULL;

PtrListDL<Material> gMaterialList;

Material *pCurrentMaterial = NULL;

Material *pNoneMaterial = NULL;
Material *pWhiteMaterial = NULL;
Material *pSysLogoLarge = NULL;
Material *pSysLogoSmall = NULL;

char matDesc[32][4] = {"M","Na","Ds","Ad","T","","A","A3","L","Ls","Le","Dm","E","Es","","","P","C","B","N","D","Ec","E","Es","D2","Lm","D","U","","","",""};


/**** Functions ****/

void Material_InitModule()
{
	CALLSTACK;

	gMaterialDefList.Init("Material Definitions List", gDefaults.material.maxMaterialDefs);
	gMaterialList.Init("Material List", gDefaults.material.maxMaterials);

	if(Material_AddDefinitionsFile("Materials.ini", "Materials.ini"))
	{
		LOGD("Warning: Failed to load Materials.ini");
	}

	// create the logo textures from raw data
	Texture *pSysLogoLargeTexture = Texture_CreateFromRawData("SysLogoLarge", SysLogo_256_data, SysLogo_256_width, SysLogo_256_height, TEXF_A8R8G8B8, TEX_VerticalMirror);
	Texture *pSysLogoSmallTexture = Texture_CreateFromRawData("SysLogoSmall", SysLogo_64_data, SysLogo_64_width, SysLogo_64_height, TEXF_A8R8G8B8, TEX_VerticalMirror);

	// create standard materials
	pNoneMaterial = Material_Create("_None");
	pWhiteMaterial = Material_Create("_White");
	pSysLogoLarge = Material_Create("SysLogoLarge");
	pSysLogoSmall = Material_Create("SysLogoSmall");

	// release a reference to the logo textures
	Texture_Destroy(pSysLogoLargeTexture);
	Texture_Destroy(pSysLogoSmallTexture);
}

void Material_DeinitModule()
{
	CALLSTACK;

	Material_Destroy(pNoneMaterial);
	Material_Destroy(pWhiteMaterial);
	Material_Destroy(pSysLogoLarge);
	Material_Destroy(pSysLogoSmall);

	MaterialDefinition *pDef = pDefinitionRegistry;

	while(pDef)
	{
		MaterialDefinition *pNext = pDef->pNextDefinition;
		Material_DestroyDefinition(pDef);
		pDef = pNext;
	}

	gMaterialList.Deinit();
	gMaterialDefList.Deinit();
}

void Material_Update()
{
	Material **ppMatIterator = gMaterialList.Begin();

	while(*ppMatIterator)
	{
		MaterialInternal_Update(*ppMatIterator);
		ppMatIterator++;
	}
}

// interface functions
int Material_AddDefinitionsFile(const char *pName, const char *pFilename)
{
	MaterialDefinition *pDef = gMaterialDefList.Create();

	pDef->pName = pName;
	pDef->pIni = MFIni::Create(pFilename);
	if (!pDef->pIni)
	{
		gMaterialDefList.Destroy(pDef);
		LOGD("Error: Couldnt create material definitions...");
		return 1;
	}

	pDef->ownsIni = true;

	pDef->pNextDefinition = pDefinitionRegistry;
	pDefinitionRegistry = pDef;

	return 0;
}

int Material_AddDefinitionsIni(const char *pName, MFIni *pMatDefs)
{
	MaterialDefinition *pDef = gMaterialDefList.Create();

	pDef->pName = pName;
	pDef->pIni = pMatDefs;
	pDef->ownsIni = false;

	pDef->pNextDefinition = pDefinitionRegistry;
	pDefinitionRegistry = pDef;

	return 0;
}

void Material_DestroyDefinition(MaterialDefinition *pDefinition)
{
	if(pDefinition->ownsIni)
	{
		MFIni::Destroy(pDefinition->pIni);
		pDefinition->pIni = NULL;
	}

	gMaterialDefList.Destroy(pDefinition);
}

void Material_RemoveDefinitions(const char *pName)
{
	MaterialDefinition *pDef = pDefinitionRegistry;

	if(!pDef)
		return;

	if(!strcmp(pDef->pName, pName))
	{
		pDefinitionRegistry = pDef->pNextDefinition;
		Material_DestroyDefinition(pDef);
		return;
	}

	while(pDef->pNextDefinition)
	{
		if(!strcmp(pDef->pNextDefinition->pName, pName))
		{
			MaterialDefinition *pDestroy = pDef->pNextDefinition;
			pDef->pNextDefinition = pDef->pNextDefinition->pNextDefinition;

			Material_DestroyDefinition(pDestroy);
			return;
		}

		pDef = pDef->pNextDefinition;
	}
}

Material* Material_Create(const char *pName)
{
	CALLSTACK;

	Material *pMat = Material_Find(pName);

	if(!pMat)
	{
		pMat = MaterialInternal_CreateDefault(pName);

		MaterialDefinition *pDef = pDefinitionRegistry;
		while(pDef)
		{
			MFIniLine *pLine = pDef->pIni->GetFirstLine()->FindEntry("Material",pName);
			if (pLine)
			{
				MaterialInternal_InitialiseFromDefinition(pDef->pIni, pMat, pName);
				break;
			}

			pDef = pDef->pNextDefinition;
		}

		if(!pDef)
		{
			Texture *pTexture = Texture_Create(pName);

			pMat->pTextures[0] = pTexture;
			pMat->diffuseMapIndex = 0;
			pMat->textureCount = 1;
		}
	}

	pMat->refCount++;

	return pMat;
}

int Material_Destroy(Material *pMaterial)
{
	CALLSTACK;

	pMaterial->refCount--;

	if(!pMaterial->refCount)
	{
		for(uint16 a=0; a<pMaterial->textureCount; a++)
		{
			Texture_Destroy(pMaterial->pTextures[a]);
		}

		gMaterialList.Destroy(pMaterial);

		return 0;
	}

	return pMaterial->refCount;
}

Material* Material_Find(const char *pName)
{
	CALLSTACK;

	Material **ppIterator = gMaterialList.Begin();

	while(*ppIterator)
	{
		if(!StrCaseCmp(pName, (*ppIterator)->name)) return *ppIterator;

		ppIterator++;
	}

	return NULL;
}

Material* Material_GetCurrent()
{
	return pCurrentMaterial;
}

void Material_SetMaterial(Material *pMaterial)
{
	if(!pMaterial)
		pMaterial = Material_GetStockMaterial(Mat_White);

	pCurrentMaterial = pMaterial;
}

Material* Material_GetStockMaterial(StockMaterials materialIdentifier)
{
	switch(materialIdentifier)
	{
		case Mat_White:
			return pWhiteMaterial;
		case Mat_Unavailable:
			return pNoneMaterial;
		case Mat_SysLogoSmall:
			return pSysLogoSmall;
		case Mat_SysLogoLarge:
			return pSysLogoLarge;
	}

	return NULL;
}


// internal functions
const char* MaterialInternal_GetIDString()
{
	CALLSTACK;
/*
	char *id = &stringBuffer[stringBufferOffset];
	*id = NULL;

	for(int a=0; a<32; a++)
	{
		if((flags>>a)&1) strcat(id, matDesc[a]);
	}

	stringBufferOffset += strlen(id) + 1;
*/
	return NULL;
}

Material* MaterialInternal_CreateDefault(const char *pName)
{
	CALLSTACK;

	Material *pMat = gMaterialList.Create();
	memset(pMat, 0, sizeof(Material));

	strcpy(pMat->name, pName);

	pMat->ambient = Vector4::one;
	pMat->diffuse = Vector4::one;
/*
	pMat->illum = Vector4::zero;
	pMat->specular = Vector4::zero;
	pMat->specularPow = 0;
*/
	pMat->materialType = MF_AlphaBlend;
	pMat->opaque = true;

	pMat->textureMatrix = Matrix::identity;
	pMat->uFrames = 1;
	pMat->vFrames = 1;
/*
	pMat->curTime = 0.0f;
	pMat->frameTime = 0.0f;
	pMat->curFrame = 0;

	pMat->refCount = 0;
*/
	return pMat;
}

void MaterialInternal_InitialiseFromDefinition(MFIni *pDefIni, Material *pMat, const char *pDefinition)
{
	CALLSTACK;

	MFIniLine *pLine = pDefIni->GetFirstLine()->FindEntry("material",pDefinition);

	if(pLine)
	{
		pLine = pLine->Sub();
		while(pLine)
		{
			if(pLine->IsString(0,"alias"))
			{
#pragma message("Alias's should be able to come from other material ini's")

				const char *pAlias = pLine->GetString(1);
				MaterialInternal_InitialiseFromDefinition(pDefIni, pMat, pAlias);
			}
			else if(pLine->IsString(0, "lit"))
			{
				pMat->materialType = (pMat->materialType & ~MF_Lit) | (pLine->GetBool(1) ? MF_Lit : NULL);
			}
			else if(pLine->IsString(0, "prelit"))
			{
				pMat->materialType = (pMat->materialType & ~MF_Lit) | (!pLine->GetBool(1) ? MF_Lit : NULL);
			}
			else if(pLine->IsString(0, "diffusecolour"))
			{
				pMat->diffuse = pLine->GetVector4(1);
			}
			else if(pLine->IsString(0, "ambientcolour"))
			{
				pMat->ambient = pLine->GetVector4(1);
			}
			else if(pLine->IsString(0, "specularcolour"))
			{
				pMat->specular = pLine->GetVector4(1);
			}
			else if(pLine->IsString(0, "specularpower"))
			{
				pMat->specularPow = pLine->GetFloat(1);
			}
			else if(pLine->IsString(0, "emissivecolour"))
			{
				pMat->illum = pLine->GetVector4(1);
			}
			else if(pLine->IsString(0, "mask"))
			{
				pMat->materialType = (pMat->materialType & ~MF_Mask) | (pLine->GetBool(1) ? MF_Mask : NULL);
			}
			else if(pLine->IsString(0, "doublesided"))
			{
				pMat->materialType = (pMat->materialType & ~MF_DoubleSided) | (pLine->GetBool(1) ? MF_DoubleSided : NULL);
			}
			else if(pLine->IsString(0, "backfacecull"))
			{
				pMat->materialType = (pMat->materialType & ~MF_DoubleSided) | (!pLine->GetBool(1) ? MF_DoubleSided : NULL);
			}
			else if(pLine->IsString(0, "additive"))
			{
				pMat->materialType = (pMat->materialType & ~MF_BlendMask) | (pLine->GetBool(1) ? MF_Additive : NULL);
			}
			else if(pLine->IsString(0, "subtractive"))
			{
				pMat->materialType = (pMat->materialType & ~MF_BlendMask) | (pLine->GetBool(1) ? MF_Subtractive : NULL);
			}
			else if(pLine->IsString(0, "alpha"))
			{
				pMat->materialType = (pMat->materialType & ~MF_BlendMask) | (pLine->GetBool(1) ? MF_AlphaBlend : NULL);
			}
			else if(pLine->IsString(0, "blend"))
			{
				pMat->materialType = (pMat->materialType & ~MF_BlendMask) | (pLine->GetInt(1) << 1);
			}
			else if(pLine->IsString(0, "texture"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "diffusemap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->diffuseMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "diffusemap2"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_DiffuseMap2;
				pMat->diffuseMap2Index = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "normalmap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_NormalMap;
				pMat->normalMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "detailmap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_DetailTexture;
				pMat->detailMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "envmap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_SphereEnvMap;
				pMat->envMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "lightmap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_LightMap;
				pMat->lightMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "bumpmap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_BumpMap;
				pMat->bumpMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "reflectionmap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_BumpMap;
				pMat->bumpMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "specularmap"))
			{
				pMat->pTextures[pMat->textureCount] = Texture_Create(pLine->GetString(1));
				pMat->materialType |= MF_BumpMap;
				pMat->bumpMapIndex = pMat->textureCount;
				pMat->textureCount++;
			}
			else if(pLine->IsString(0, "celshading"))
			{
				pMat->materialType |= MF_CelShading;
	//				pMat-> = gMaterialDefinitions.AsInt(0);
			}
			else if(pLine->IsString(0, "phong"))
			{
				pMat->materialType |= MF_LitPerPixel;
	//				pMat-> = gMaterialDefinitions.AsInt(0);
			}
			else if(pLine->IsString(0, "animated"))
			{
				pMat->materialType |= MF_Animating;
				pMat->uFrames = pLine->GetInt(1);
				pMat->vFrames = pLine->GetInt(2);
				pMat->frameTime = pLine->GetFloat(3);

				pMat->textureMatrix.SetScale(Vector(1.0f/(float)pMat->uFrames, 1.0f/(float)pMat->vFrames, 1.0f));
			}

			pLine = pLine->Next();
		}
	}
}

void MaterialInternal_Update(Material *pMaterial)
{
	if(pMaterial->materialType & MF_Animating)
	{
		pMaterial->curTime += TIMEDELTA;

		while(pMaterial->curTime >= pMaterial->frameTime)
		{
			pMaterial->curTime -= pMaterial->frameTime;

			pMaterial->curFrame++;
			pMaterial->curFrame = pMaterial->curFrame % (pMaterial->uFrames*pMaterial->vFrames);

			pMaterial->textureMatrix.SetZAxis(Vector((1.0f/(float)pMaterial->uFrames) * (float)(pMaterial->curFrame%pMaterial->uFrames), (1.0f/(float)pMaterial->vFrames) * (float)(pMaterial->curFrame/pMaterial->vFrames), 0.0f));
		}
	}
}


