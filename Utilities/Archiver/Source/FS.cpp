#include "Fuji.h"
#include "MFHeap.h"
#include "FS.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

void FreeDirectoryEntries(std::vector<std::string> &entries)
{
	entries.clear();
}


#if defined(_WIN32) // *** WINDOWS CODE ***

bool GetCurrentDir(char *dir, int maxlen)
{
	// This is valid on Windows & Linux but not on some POSIXes, best enforce it
	if(maxlen == 0)
		return false;

	return (_getcwd(dir, maxlen) != NULL);
}

int GetDirectoryEntries(const char *directory, std::vector<std::string> &entries)
{
	int numEntries = 0;
	char tempDir[256];

	FreeDirectoryEntries(entries);

	MFString_CopyCat(tempDir, directory, "*");

	WIN32_FIND_DATAA findData;
	HANDLE dirHandle = FindFirstFile(tempDir, &findData);

	if(dirHandle == INVALID_HANDLE_VALUE)
		return 0;

	BOOL more = true;

	while(more)
	{
		if(MFString_Compare(findData.cFileName, ".") && MFString_Compare(findData.cFileName, "..") && MFString_Compare(findData.cFileName, ".svn"))
		{
			// check if it matches any exclude patterns...

			// if not
			entries.push_back(findData.cFileName);
			++numEntries;
		}

		more = FindNextFile(dirHandle, &findData);
	}

	FindClose(dirHandle);

	return numEntries;
}

bool IsDirectory(const char *entry)
{
	DWORD attr = GetFileAttributes(entry);
	if(attr == INVALID_FILE_ATTRIBUTES)
		return false;

	return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

#else // *** POSIX CODE ***

bool GetCurrentDir(char *dir, int maxlen)
{
	// This is valid on Windows & Linux but not on some POSIXes, best enforce it
	if(maxlen == 0)
		return false;

	return getcwd(dir, maxlen) != NULL;
}

int GetDirectoryEntries(const char *directory, std::vector<std::string> &entries)
{
	int numEntries = 0;
	struct dirent *entry;

	FreeDirectoryEntries(entries);

	char *tempDir = (char*)MFHeap_Alloc(strlen(directory) + 4);
	MFString_Copy(tempDir, directory);

	char *pSlash = &tempDir[MFString_Length(tempDir)-1];
	if(*pSlash == '/' || *pSlash == '\\')
		*pSlash = 0;

	DIR *dirHandle = opendir(tempDir);
	MFHeap_Free(tempDir);
	if(dirHandle == NULL)
		return 0;

	entry = readdir(dirHandle);

	while(entry)
	{
		if((MFString_Compare(entry->d_name, ".") != 0) && (MFString_Compare(entry->d_name, "..") != 0))
		{
			tempDir = new char[MFString_Length(entry->d_name) + 1];
			MFString_Copy(tempDir, entry->d_name);
			entries.push_back(tempDir);

			++numEntries;
		}

		entry = readdir(dirHandle);
	}

	closedir(dirHandle);

	return numEntries;
}

bool IsDirectory(const char *entry)
{
	struct stat statbuf;

	if(stat(entry, &statbuf) < 0)
		return false;

	return S_ISDIR(statbuf.st_mode);
}

#endif
