#if !defined(_MFMODEL_INTERNAL_H)
#define _MFMODEL_INTERNAL_H

#include "MFModel.h"
#include "MFMaterial_Internal.h"

struct MFModelTemplate;

void MFModel_InitModule();
void MFModel_DeinitModule();

void MFModel_CreateMeshChunk(MFMeshChunk *pMeshChunk);
void MFModel_DestroyMeshChunk(MFMeshChunk *pMeshChunk);
void MFModel_FixUpMeshChunk(MFMeshChunk *pMeshChunk, uint32 base, bool load);

enum MFModelDataChunkType
{
	MFCT_Unknown = -1,

	MFCT_SubObjects = 0,
	MFCT_Bones,
	MFCT_Tags,
	MFCT_Materials,
	MFCT_BinaryFilesystem,

	MFCT_Max,
	MFCT_ForceInt = 0x7FFFFFFF
};

struct MFModelDataChunk
{
	MFModelDataChunkType chunkType;
	int count;
	void *pData;
	uint32 reserved;
};

MFModelDataChunk *MFModel_GetDataChunk(MFModelTemplate *pModelTemplate, MFModelDataChunkType chunkID);

struct MFModel
{
	MFMatrix worldMatrix;
	MFVector modelColour;

	// subobject flags
	// * enabled flags
	// * subobject colours

	MFModelTemplate *pTemplate;
};

struct MFModelTemplate
{
	union
	{
		uint32 IDtag;
		const char *pFilename;
	};

	const char *pName;

	int numDataChunks;
	MFModelDataChunk *pDataChunks;

	int refCount;
};

struct SubObjectChunk
{
	const char *pSubObjectName;
//	MFMaterial *pMaterial;

	int numMeshChunks;
	MFMeshChunk *pMeshChunks;
};

struct BoneChunk
{
	MFVector boneOrigin;
	const char *pBoneName;
	const char *pParentName;

	uint32 reserved[2];
};

struct TagChunk
{
	MFMatrix tagMatrix;
	const char *pTagName;

	uint32 reserved[3];
};

struct BinaryFilesystemChunk
{
	// this is a binary archive that can be mounted and accessed with the memory filesystem..
	// it could contain texture data, or other relevant model related data..
};


#endif
