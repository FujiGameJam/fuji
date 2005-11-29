#include "Fuji.h"
#include "MFHeap.h"

#include <stdlib.h>

void MFHeap_InitModulePlatformSpecific()
{
}

void MFHeap_DeinitModulePlatformSpecific()
{
}

// use CRT memory functions
void* MFHeap_SystemMalloc(uint32 bytes)
{
	MFCALLSTACK;

	return malloc(bytes);
}

void* MFHeap_SystemRealloc(void *buffer, uint32 bytes)
{
	MFCALLSTACK;

	return realloc(buffer, bytes);
}

void MFHeap_SystemFree(void *buffer)
{
	MFCALLSTACK;

	return free(buffer);
}


void* MFHeap_GetUncachedPointer(void *pPointer)
{
	return pPointer;
}

void MFHeap_FlushDCache()
{
}
