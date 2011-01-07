//
// Mount Fuji Simple string cache.
//
//=============================================================================
#include "Fuji.h"
#include "MFHeap.h"
#include "MFStringCache.h"

struct MFStringCache
{
	uint32 size;
	uint32 used;
	char *pMem;
};


MFStringCache* MFStringCache_Create(uint32 maxSize)
{
	MFCALLSTACK;

	MFStringCache *pCache;
	pCache = (MFStringCache*)MFHeap_Alloc(sizeof(MFStringCache) + maxSize);

	pCache->size = maxSize;
	pCache->pMem = (char*)&pCache[1];
	pCache->pMem[0] = pCache->pMem[1] = 0;
	pCache->used = 2;

	return pCache;
}

void MFStringCache_Destroy(MFStringCache *pCache)
{
	MFCALLSTACK;

	MFHeap_Free(pCache);
}

const char *MFStringCache_Add(MFStringCache *pCache, const char *pNewString)
{
	MFCALLSTACK;

	MFDebug_Assert(pCache, "NULL String cache!");
	MFDebug_Assert(pNewString, "Cannot add NULL string");

	if(!*pNewString)
		return pCache->pMem;

	// find the string
	char *pCurr = pCache->pMem + 1;
	int newLength = MFString_Length(pNewString)+1;

	while (pCurr[0] && pCurr < &pCache->pMem[pCache->size])
	{
		if (!MFString_Compare(pCurr, pNewString))
		{
			// found string
			return pCurr;
		}
		pCurr += MFString_Length(pCurr)+1;
	}

	if(pCurr+newLength+1 > pCache->pMem+pCache->size)
		return NULL;

	MFString_Copy(pCurr, pNewString);
	pCurr[newLength]=0;
	pCache->used = ((uintp)pCurr + newLength) - (uintp)pCache->pMem + 1;
	return pCurr;
}

// get cache for direct read/write
char* MFStringCache_GetCache(MFStringCache *pCache)
{
	return pCache->pMem;
}

// get the actual used size of the cache
uint32 MFStringCache_GetSize(MFStringCache *pCache)
{
	return pCache->used;
}

// use this if you have filled the cache yourself
void MFStringCache_SetSize(MFStringCache *pCache, uint32 used)
{
	pCache->used = used;
}


//
// unicode support
//

struct MFWStringCache
{
	uint32 size;
	uint32 used;
	uint16 *pMem;
};


MFWStringCache* MFWStringCache_Create(uint32 maxSize)
{
	MFCALLSTACK;

	MFWStringCache *pCache;
	pCache = (MFWStringCache*)MFHeap_Alloc(sizeof(MFWStringCache) + maxSize);

	pCache->size = maxSize;
	pCache->pMem = (uint16*)&pCache[1];
	pCache->pMem[0] = 0;
	pCache->used = 2;

	return pCache;
}

void MFWStringCache_Destroy(MFWStringCache *pCache)
{
	MFCALLSTACK;

	MFHeap_Free(pCache);
}

const uint16 *MFWStringCache_Add(MFWStringCache *pCache, const uint16 *pNewString)
{
	MFCALLSTACK;

	MFDebug_Assert(pCache, "NULL String cache!");
	MFDebug_Assert(pNewString, "Cannot add NULL string");

	// find the string
	uint16 *pCurr = pCache->pMem;
	int newLength = MFWString_Length(pNewString)+1;

	while (pCurr[0] && pCurr < &pCache->pMem[pCache->size])
	{
		if (!MFWString_Compare(pCurr, pNewString))
		{
			// found string
			return pCurr;
		}
		pCurr += MFWString_Length(pCurr)+1;
	}

	MFDebug_Assert(&pCurr[newLength+1] < &pCache->pMem[pCache->size], "No memory for string!");

	MFWString_Copy(pCurr, pNewString);
	pCurr[newLength]=0;
	pCache->used = (((uintp)pCurr + newLength) - (uintp)pCache->pMem + 1) * 2;
	return pCurr;
}

// get cache for direct read/write
uint16* MFWStringCache_GetCache(MFWStringCache *pCache)
{
	return pCache->pMem;
}

// get the actual used size of the cache
uint32 MFWStringCache_GetSize(MFWStringCache *pCache)
{
	return pCache->used;
}

// use this if you have filled the cache yourself
void MFWStringCache_SetSize(MFWStringCache *pCache, uint32 used)
{
	pCache->used = used;
}
