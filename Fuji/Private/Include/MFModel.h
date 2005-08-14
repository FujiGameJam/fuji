#if !defined(_MFMODEL_INTERNAL_H)
#define _MFMODEL_INTERNAL_H

struct MFModel;
struct MFMeshChunk;

// create/destroy
MFModel* MFModel_Create(const char *pFilename);
void MFModel_Destroy(MFModel *pModel);

// draw model
void MFModel_Draw(MFModel *pModel);

// set model data
void MFModel_SetWorldMatrix(MFModel *pModel, const MFMatrix &worldMatrix);
void MFModel_SetColour(MFModel *pModel, const MFVector &colour);

// get model info
const char* MFModel_GetName();

// get subobject info
int MFModel_GetNumSubObjects();
int MFModel_GetSubObjectIndex(const char *pSubobjectName);
const char* MFModel_GetSubObjectName(int index);

MFMeshChunk* MFModel_GetMeshChunk(int subobjectIndex, int meshChunkIndex);

// bone structure
int MFModel_GetNumBones();
const char* MFModel_GetBoneName(int boneIndex);
const MFVector& MFModel_GetBoneOrigin(int boneIndex);
int MFModel_GetBoneIndex(const char *pName);

// tag's
int MFModel_GetNumTags();
const char* MFModel_GetTagName(int tagIndex);
const MFVector& MFModel_GetTagMatrix(int tagIndex);
int MFModel_GetTagIndex(const char *pName);

// ... etc

#endif
