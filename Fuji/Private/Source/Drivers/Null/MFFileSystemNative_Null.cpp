#include "Fuji.h"

#if MF_FILESYSTEM == MF_DRIVER_NULL

#include "MFFileSystem_Internal.h"
#include "FileSystem/MFFileSystemNative_Internal.h"
#include "MFHeap.h"

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	#include <stdio.h>
	#include <io.h>
#endif

void MFFileSystemNative_Register()
{
}

void MFFileSystemNative_Unregister()
{
}

int MFFileNative_Open(MFFile *pFile, MFOpenData *pOpenData)
{
	MFCALLSTACK;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	MFDebug_Assert(pOpenData->cbSize == sizeof(MFOpenDataNative), "Incorrect size for MFOpenDataNative structure. Invalid pOpenData.");
	MFOpenDataNative *pNative = (MFOpenDataNative*)pOpenData;

	int flags = 0;
	const char *pAccess = "";

	if(pOpenData->openFlags & MFOF_Read)
	{
		if(pNative->openFlags & MFOF_Write)
		{
			pAccess = "w+b";
		}
		else
		{
			pAccess = "rb";
		}
	}
	else if(pOpenData->openFlags & MFOF_Write)
	{
		pAccess = "wb";
	}
	else
	{
		MFDebug_Assert(0, "Neither MFOF_Read nor MFOF_Write specified.");
	}

	pFile->pFilesysData = (void*)fopen(pNative->pFilename, pAccess);

	if(pFile->pFilesysData == NULL)
	{
		MFDebug_Warn(3, MFStr("Failed to open file '%s'.", pNative->pFilename));
		return -1;
	}

	pFile->state = MFFS_Ready;
	pFile->operation = MFFO_None;
	pFile->createFlags = pOpenData->openFlags;
	pFile->offset = 0;

	pFile->length = MFFileNative_GetSize(pFile);

#if defined(_DEBUG)
	MFString_Copy(pFile->fileIdentifier, pNative->pFilename);
#endif

	return 0;
#else
	return -1;
#endif
}

int MFFileNative_Close(MFFile* fileHandle)
{
	MFCALLSTACK;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	fclose((FILE*)fileHandle->pFilesysData);
	fileHandle->pFilesysData = 0;
#endif

	return 0;
}

int MFFileNative_Read(MFFile* fileHandle, void *pBuffer, uint32 bytes, bool async)
{
	MFCALLSTACK;

	MFDebug_Assert(async == false, "Asynchronous Filesystem not yet supported...");

	size_t bytesRead = 0;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	bytesRead = fread(pBuffer, 1, bytes, (FILE*)fileHandle->pFilesysData);
	if(bytesRead < 0) // read() returns -1 on error
		bytesRead = 0;

	fileHandle->offset += (uint32)bytesRead;
#endif

	return (int)bytesRead;
}

int MFFileNative_Write(MFFile* fileHandle, const void *pBuffer, uint32 bytes, bool async)
{
	MFCALLSTACK;

	MFDebug_Assert(async == false, "Asynchronous Filesystem not yet supported...");

	size_t bytesWritten = 0;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	bytesWritten = fwrite(pBuffer, 1, (size_t)bytes, (FILE*)fileHandle->pFilesysData);
	if(bytesWritten < 0) // write() returns -1 on error
		bytesWritten = 0;

	fileHandle->offset += (uint32)bytesWritten;
	fileHandle->length = MFMax(fileHandle->offset, (uint32)fileHandle->length);
#endif

	return (int)bytesWritten;
}

int MFFileNative_Seek(MFFile* fileHandle, int bytes, MFFileSeek relativity)
{
	MFCALLSTACK;

	if(!fileHandle->pFilesysData)
		return -1;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	int newOffset;
	int whence = 0;

	switch(relativity)
	{
		case MFSeek_Begin:
			whence = SEEK_SET;
			break;
		case MFSeek_End:
			whence = SEEK_END;
			break;
		case MFSeek_Current:
			whence = SEEK_CUR;
			break;
		default:
			MFDebug_Assert(false, "Invalid 'whence'.");
			break;
	}

	fseek((FILE*)fileHandle->pFilesysData, bytes, whence);
	newOffset = ftell((FILE*)fileHandle->pFilesysData);

	if(newOffset != -1)
	{
		fileHandle->offset = (uint32)newOffset;
	}
#endif

	return (int)fileHandle->offset;
}

int MFFileNative_Tell(MFFile* fileHandle)
{
	MFCALLSTACK;
	return fileHandle->offset;
}

MFFileState MFFileNative_Query(MFFile* fileHandle)
{
	MFCALLSTACK;

	if(!fileHandle->pFilesysData)
		return MFFS_Unavailable;

	return fileHandle->state;
}

int MFFileNative_GetSize(MFFile* fileHandle)
{
	MFCALLSTACK;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	fseek((FILE*)fileHandle->pFilesysData, 0, SEEK_END);
	long size = ftell((FILE*)fileHandle->pFilesysData);
	fseek((FILE*)fileHandle->pFilesysData, fileHandle->offset, SEEK_SET);

	return size;
#else
	return 0;
#endif
}

uint32 MFFileNative_GetSize(const char* pFilename)
{
	MFCALLSTACK;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	FILE *pFile = fopen(pFilename, "rb");
	if(!pFile)
		return 0;

	fseek(pFile, 0, SEEK_END);
	long size = ftell(pFile);
	fclose(pFile);

	return size;
#else
	return 0;
#endif
}

bool MFFileNative_Exists(const char* pFilename)
{
	MFCALLSTACK;

#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	FILE *pFile = fopen(pFilename, "rb");

	if(!pFile)
		return false;

	fclose(pFile);
	return true;
#else
	return false;
#endif
}

bool MFFileNative_FindFirst(MFFind *pFind, const char *pSearchPattern, MFFindData *pFindData)
{
#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	// open the directory
	_finddata_t fd;

	char *pPath = (char*)MFStr("%s%s", (char*)pFind->pMount->pFilesysData, pSearchPattern);
	intptr_t find = _findfirst(pPath, &fd);

	if(find == -1)
		return false;

	pFindData->attributes = ((fd.attrib & _A_SUBDIR) ? MFFA_Directory : 0) |
							((fd.attrib & _A_HIDDEN) ? MFFA_Hidden : 0) |
							((fd.attrib & _A_RDONLY) ? MFFA_ReadOnly : 0);
	pFindData->fileSize = fd.size;
	MFString_Copy((char*)pFindData->pFilename, fd.name);

	MFString_CopyCat(pFindData->pSystemPath, (char*)pFind->pMount->pFilesysData, pSearchPattern);
	char *pLast = MFString_RChr(pFindData->pSystemPath, '/');
	if(pLast)
		pLast[1] = 0;
	else
		pFindData->pSystemPath[0] = NULL;

	pFind->pFilesystemData = (void*)find;

	return true;
#else
	return false;
#endif
}

bool MFFileNative_FindNext(MFFind *pFind, MFFindData *pFindData)
{
#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	_finddata_t fd;

	int more = _findnext((intptr_t)pFind->pFilesystemData, &fd);

	if(more == -1)
		return false;

	MFDebug_Assert(more == 0, "Something's not quite right here.. You have a bad CRT.");

	pFindData->attributes = ((fd.attrib & _A_SUBDIR) ? MFFA_Directory : 0) |
							((fd.attrib & _A_HIDDEN) ? MFFA_Hidden : 0) |
							((fd.attrib & _A_RDONLY) ? MFFA_ReadOnly : 0);
	pFindData->fileSize = fd.size;
	MFString_Copy((char*)pFindData->pFilename, fd.name);

	return true;
#else
	return false;
#endif
}

void MFFileNative_FindClose(MFFind *pFind)
{
#if defined(_USE_CRT_FOR_NULL_DRIVERS)
	_findclose((intptr_t)pFind->pFilesystemData);
#endif
}

#endif
