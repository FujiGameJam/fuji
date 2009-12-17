#include "Fuji.h"
#include "MFSystem.h"
#include "MFFileSystem.h"
#include "MFHeap.h"
#include "MFString.h"
#include "MFModel_Internal.h"
#include "Asset/MFIntModel.h"
#include "Util/F3D.h"

/**** Structures ****/


/**** Globals ****/

static char *gFileExtensions[] =
{
	".f3d",
	".dae",
	".x",
	".ase",
	".obj",
	".md2",
	".md3",
	".memd2"
};


/**** Functions ****/

MFIntModel *MFIntModel_CreateFromFile(const char *pFilename)
{
	// find format
	const char *pExt = MFString_GetFileExtension(pFilename);

	int format;
	for(format=0; format<MFIMF_Max; ++format)
	{
		if(!MFString_Compare(pExt, gFileExtensions[format]))
			break;
	}
	if(format == MFIMF_Max)
		return NULL; // unsupported model format

	// some text formats need a null character appended for processing
	bool bAppendNull = false;
	if(format == MFIMF_X || format == MFIMF_ASE || format == MFIMF_OBJ)
		bAppendNull = true;

	// load file
	uint32 size;
	char *pData = MFFileSystem_Load(pFilename, &size, bAppendNull);
	if(!pData)
		return NULL;

	// load the image
	MFIntModel *pModel = MFIntModel_CreateFromFileInMemory(pData, size, (MFIntModelFormat)format);

	// free file
	MFHeap_Free(pData);

	return pModel;
}

MFIntModel *MFIntModel_CreateFromFileInMemory(const void *pMemory, uint32 size, MFIntModelFormat format)
{
	F3DFile *pF3D = new F3DFile;

	switch(format)
	{
		case MFIMF_ASE:
			void ParseASEFile(char *, F3DFile *);
			ParseASEFile((char*)pMemory, pF3D);
			break;
		default:
			MFDebug_Assert(false, "Unsupported model format.");
	}

	return (MFIntModel*)pF3D;
}

void MFIntModel_Optimise(MFIntModel *pModel)
{
	F3DFile *pF3D = (F3DFile*)pModel;

	pF3D->Optimise();
}

void MFIntModel_CreateRuntimeData(MFIntModel *pModel, void **ppOutput, uint32 *pSize, MFPlatform platform)
{
	F3DFile *pF3D = (F3DFile*)pModel;

	*ppOutput = NULL;
	if(pSize)
		*pSize = 0;

	pF3D->ProcessSkeletonData();
	pF3D->ProcessCollisionData();
	pF3D->BuildBatches(platform);
	pF3D->StripModel();

	if(pF3D->GetMeshChunk()->subObjects.size() ||
		pF3D->GetCollisionChunk()->collisionObjects.size() ||
		pF3D->GetRefPointChunk()->refPoints.size())
	{
		*ppOutput = pF3D->CreateMDL(pSize, platform);
	}
}

void MFIntModel_CreateAnimationData(MFIntModel *pModel, void **ppOutput, uint32 *pSize, MFPlatform platform)
{
	F3DFile *pF3D = (F3DFile*)pModel;

	*ppOutput = NULL;
	*pSize = 0;

	// we'll assume the user wrote out the model first (so it does the processing and stripping/etc)

	if(pF3D->GetAnimationChunk()->anims.size())
	{
		*ppOutput = pF3D->CreateANM(pSize, platform);
	}
}

void MFIntModel_Destroy(MFIntModel *pModel)
{
	F3DFile *pF3D = (F3DFile*)pModel;
	delete pF3D;
}
