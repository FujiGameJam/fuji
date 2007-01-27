#include "Fuji.h"
#include "MFFileSystem_Internal.h"
#include "FileSystem/MFFileSystemNative_Internal.h"
#include "MFHeap.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

void MFFileSystemNative_Register()
{
}

void MFFileSystemNative_Unregister()
{
}

int MFFileSystemNative_GetNumEntries(const char *pFindPattern, bool recursive, bool flatten, int *pStringLengths)
{
	int numFiles = 0;

	DIR *pDir;
	dirent *pD;

	*pStringLengths += MFString_Length(pFindPattern) + 1;

	pDir = opendir(pFindPattern);

	while((pD = readdir(pDir)))
	{
		if(MFString_Compare(pD->d_name, ".") && MFString_Compare(pD->d_name, "..") && MFString_Compare(pD->d_name, ".svn"))
		{
			// check if the entry is a directory..
			const char *pSubDir = MFStr("%s%s/", pFindPattern, pD->d_name);
			DIR *pIsDir = opendir(pSubDir);

			if(pIsDir)
			{
				closedir(pIsDir);

				if(recursive)
				{
					if(flatten)
					{
						numFiles += MFFileSystemNative_GetNumEntries(pSubDir, recursive, flatten, pStringLengths);
					}
					else
					{
						*pStringLengths += MFString_Length(pD->d_name) + 1;
						++numFiles;
					}
				}
			}
			else
			{
				*pStringLengths += MFString_Length(pD->d_name) + 1;
				++numFiles;
			}
		}
	}

	closedir(pDir);

	return numFiles;
}

MFTOCEntry* MFFileSystemNative_BuildToc(const char *pFindPattern, MFTOCEntry *pToc, MFTOCEntry *pParent, char* &pStringCache, bool recursive, bool flatten)
{
	DIR *pDir;
	dirent *pD;

	pDir = opendir(pFindPattern);

	char *pCurrentDir = pStringCache;
	MFString_Copy(pCurrentDir, pFindPattern);
	pStringCache += MFString_Length(pCurrentDir) + 1;

	while((pD = readdir(pDir)))
	{
		if(MFString_Compare(pD->d_name, ".") && MFString_Compare(pD->d_name, "..") && MFString_Compare(pD->d_name, ".svn"))
		{
			// check if the entry is a directory..
			const char *pSubDir = MFStr("%s%s/", pFindPattern, pD->d_name);
			DIR *pIsDir = opendir(pSubDir);

			if(pIsDir)
			{
				closedir(pIsDir);

				if(recursive)
				{
					if(flatten)
					{
						pToc = MFFileSystemNative_BuildToc(pSubDir, pToc, pParent, pStringCache, recursive, flatten);
					}
					else
					{
						const char *pNewPath = pSubDir;

						int stringCacheSize = 0;
						pToc->size = MFFileSystemNative_GetNumEntries(pNewPath, recursive, flatten, &stringCacheSize);

						if(pToc->size)
						{
							MFString_Copy(pStringCache, pD->d_name);
							pToc->pName = pStringCache;
							pStringCache += MFString_Length(pStringCache)+1;

							pToc->flags = MFTF_Directory;
							pToc->pFilesysData = pCurrentDir;
							pToc->pParent = pParent;

							int sizeOfToc = sizeof(MFTOCEntry)*pToc->size;
							pToc->pChild = (MFTOCEntry*)MFHeap_Alloc(sizeof(MFTOCEntry)*sizeOfToc + stringCacheSize);

							char *pNewStringCache = ((char*)pToc->pChild)+sizeOfToc;
							MFFileSystemNative_BuildToc(pNewPath, pToc->pChild, pToc, pNewStringCache, recursive, flatten);

							++pToc;
						}
					}
				}
			}
			else
			{
				MFString_Copy(pStringCache, pD->d_name);
				pToc->pName = pStringCache;
				pStringCache += MFString_Length(pStringCache)+1;

				pToc->pFilesysData = pCurrentDir;

				pToc->pParent = pParent;
				pToc->pChild = NULL;

				pToc->flags = 0;
				pToc->size = 0;

				++pToc;
			}
		}
	}

	closedir(pDir);

	return pToc;
}

int MFFileSystemNative_Mount(MFMount *pMount, MFMountData *pMountData)
{
	MFDebug_Assert(pMountData->cbSize == sizeof(MFMountDataNative), "Incorrect size for MFMountDataNative structure. Invalid pMountData.");

	MFMountDataNative *pMountNative = (MFMountDataNative*)pMountData;

	DIR *pDir;

	bool flatten = (pMountData->flags & MFMF_FlattenDirectoryStructure) != 0;
	bool recursive = (pMountData->flags & MFMF_Recursive) != 0;

	const char *pFindPattern = pMountNative->pPath;

	if(pFindPattern[MFString_Length(pFindPattern)-1] != '/')
		pFindPattern = MFStr("%s/", pFindPattern);

	pDir = opendir(pFindPattern);

	if(!pDir)
	{
		MFDebug_Warn(1, MFStr("FileSystem: Couldnt Mount Native FileSystem '%s'.", pMountNative->pPath));
		return -1;
	}

	closedir(pDir);

	int stringCacheSize = 0;
	pMount->numFiles = MFFileSystemNative_GetNumEntries(pFindPattern, recursive, flatten, &stringCacheSize);

	int sizeOfToc = sizeof(MFTOCEntry)*pMount->numFiles;
	pMount->pEntries = (MFTOCEntry*)MFHeap_Alloc(sizeOfToc + stringCacheSize);

	char *pStringCache = ((char*)pMount->pEntries)+sizeOfToc;
	MFFileSystemNative_BuildToc(pFindPattern, pMount->pEntries, NULL, pStringCache, recursive, flatten);

	return 0;
}

int MFFileNative_Open(MFFile *pFile, MFOpenData *pOpenData)
{
	MFCALLSTACK;

	MFDebug_Assert(pOpenData->cbSize == sizeof(MFOpenDataNative), "Incorrect size for MFOpenDataNative structure. Invalid pOpenData.");
	MFOpenDataNative *pNative = (MFOpenDataNative*)pOpenData;

	int flags = 0;

	if(pOpenData->openFlags & MFOF_Read)
	{
		if(pNative->openFlags & MFOF_Write)
		{
			flags = O_RDWR | O_CREAT;
		}
		else
		{
			flags = O_RDONLY;
		}
	}
	else if(pOpenData->openFlags & MFOF_Write)
	{
		flags = O_WRONLY | O_CREAT;
	}
	else
	{
		MFDebug_Assert(0, "Neither MFOF_Read nor MFOF_Write specified.");
	}

	pFile->pFilesysData = (void*)open(pNative->pFilename, flags);

	if((int)pFile->pFilesysData == -1)
	{
		MFDebug_Warn(3, MFStr("Failed to open file '%s'.", pNative->pFilename));
		pFile->pFilesysData = 0;
		return -1 ;
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
}

int MFFileNative_Close(MFFile* fileHandle)
{
	MFCALLSTACK;

	close((int)fileHandle->pFilesysData);
	fileHandle->pFilesysData = 0;

	return 0;
}

int MFFileNative_Read(MFFile* fileHandle, void *pBuffer, uint32 bytes, bool async)
{
	MFCALLSTACK;

	MFDebug_Assert(async == false, "Asynchronous Filesystem not yet supported...");

	ssize_t bytesRead;

	bytesRead = read((int)fileHandle->pFilesysData, pBuffer, bytes);
	if(bytesRead < 0) // read() returns -1 on error
		bytesRead = 0;

	fileHandle->offset += (uint32)bytesRead;

	return (int)bytesRead;
}

int MFFileNative_Write(MFFile* fileHandle, const void *pBuffer, uint32 bytes, bool async)
{
	MFCALLSTACK;

	MFDebug_Assert(async == false, "Asynchronous Filesystem not yet supported...");

	ssize_t bytesWritten;

	bytesWritten = write((int)fileHandle->pFilesysData, pBuffer, (size_t)bytes);
	if(bytesWritten < 0) // write() returns -1 on error
		bytesWritten = 0;

	fileHandle->offset += (uint32)bytesWritten;
	fileHandle->length = MFMax(fileHandle->offset, (uint32)fileHandle->length);

	return (int)bytesWritten;
}

int MFFileNative_Seek(MFFile* fileHandle, int bytes, MFFileSeek relativity)
{
	MFCALLSTACK;

	off_t newOffset;
	int whence = 0;

	if(!fileHandle->pFilesysData)
		return -1;

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

	newOffset = lseek((int)fileHandle->pFilesysData, bytes, whence);
//	lseek((int)fileHandle->pFilesysData, bytes, whence);
//	newOffset = tell((int)fileHandle->pFilesysData);

	if(newOffset != -1)
	{
		fileHandle->offset = (uint32)newOffset;
	}

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
	{
		return MFFS_Unavailable;
	}

	return fileHandle->state;
}

int MFFileNative_GetSize(MFFile* fileHandle)
{
	MFCALLSTACK;

	struct stat fileStats;

	if(fstat((int)fileHandle->pFilesysData, &fileStats) == -1)
	{
		return 0;
	}

	return fileStats.st_size;
}

uint32 MFFileNative_GetSize(const char* pFilename)
{
	MFCALLSTACK;

	struct stat fileStats;

	if(stat(pFilename, &fileStats) == -1)
	{
		return 0;
	}

	return fileStats.st_size;
}

bool MFFileNative_Exists(const char* pFilename)
{
	MFCALLSTACK;

	struct stat fileStats;
	int rv;

	rv = stat(pFilename, &fileStats);

	if(rv == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}
