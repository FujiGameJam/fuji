//
// MFIni.cpp
//

#include "Fuji.h"
#include "MFHeap.h"
#include "MFFileSystem.h"
#include "MFIni.h"
#include "MFStringCache.h"

#if defined(_FUJI_UTIL)
#include <stdio.h>
#endif

//=============================================================================
const char *MFIniLine::GetString(int index)
{
	if(index < stringCount)
		return pIni->pStrings[firstString+index];
	return NULL;
}

float MFIniLine::GetFloat(int index)
{
	return (float)atof(GetString(index));
}

int MFIniLine::GetInt(int index)
{
	if(index >= stringCount)
		return 0;
	return atoi(GetString(index));
}

bool MFIniLine::GetBool(int index)
{
	if(index >= stringCount)
		return false;
	return atoi(GetString(index)) != 0;
}

MFVector MFIniLine::GetVector2(int index)
{
	MFDebug_Assert(stringCount > index + 2, "Line does not have enough data");
	return MakeVector(GetFloat(index), GetFloat(index+1));
}

MFVector MFIniLine::GetVector3(int index)
{
	MFDebug_Assert(stringCount > index + 3, "Line does not have enough data");
	return MakeVector(GetFloat(index), GetFloat(index+1), GetFloat(index+2));
}

MFVector MFIniLine::GetVector4(int index)
{
	MFDebug_Assert(stringCount > index + 4, "Line does not have enough data");
	return MakeVector(GetFloat(index), GetFloat(index+1), GetFloat(index+2), GetFloat(index+3));
}

MFMatrix MFIniLine::GetMatrix(int index)
{
	MFDebug_Assert(stringCount > index + 16, "Line does not have enough data");
	MFMatrix mat;
	mat.SetXAxis4(MakeVector(GetFloat(index), GetFloat(index+1), GetFloat(index+2), GetFloat(index+3)));
	mat.SetYAxis4(MakeVector(GetFloat(index+4), GetFloat(index+5), GetFloat(index+6), GetFloat(index+7)));
	mat.SetZAxis4(MakeVector(GetFloat(index+8), GetFloat(index+9), GetFloat(index+10), GetFloat(index+11)));
	mat.SetTrans4(MakeVector(GetFloat(index+12), GetFloat(index+13), GetFloat(index+14), GetFloat(index+15)));
	return mat;
}

// find a 2 string entry (ie. "label data")
MFIniLine *MFIniLine::FindEntry(const char *pLabel, const char *pData)
{
	MFCALLSTACK;

	MFIniLine *pLine = this;
	while (pLine)
	{
		if (pLine->IsString(0, pLabel) && pLine->IsString(1, pData))
			return pLine;
		pLine = pLine->Next();
	}
	return NULL;
}


#define MAX_LINES (32000)
#define MAX_STRINGS (64000)
#define MAX_STRINGCACHE (128000)
//=============================================================================
// Create INI file
MFIni *MFIni::Create(const char *pFilename)
{
	MFCALLSTACK;

	uint32 memSize;

	// load text file
#if !defined(_FUJI_UTIL)
	char *pMem = MFFileSystem_Load(MFStr("%s.ini", pFilename), &memSize);
#else
	FILE *pFile = fopen(pFilename, "rb");

	if(!pFile)
		return NULL;

	fseek(pFile, 0, SEEK_END);
	memSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	char *pMem = (char*)MFHeap_Alloc(memSize);
	fread(pMem, 1, memSize, pFile);
	fclose(pFile);
#endif

	MFDebug_Assert(pMem != NULL, "Couldnt load .ini file!");

	// allocate ini file
	MFIni *pMFIni;
	pMFIni = (MFIni *)MFHeap_Alloc(sizeof(MFIni));
	MFString_Copy(pMFIni->name, pFilename);

	// allocate temporary buffer for strings & lines
	pMFIni->pLines = (MFIniLine *)MFHeap_Alloc(sizeof(MFIniLine)*MAX_LINES);
	pMFIni->pStrings = (const char **)MFHeap_Alloc(4*MAX_STRINGS);
	pMFIni->pCache = MFStringCache_Create(MAX_STRINGCACHE);

	// scan though the file
	pMFIni->lineCount = 0;
	pMFIni->stringCount = 0;
//	char *pIn = pMem;

	// scan text file
	pMFIni->ScanRecursive(pMem, pMem+memSize);

	// TODO: copy lines, strings & cache to save on memory

	return pMFIni;
}

MFIni *MFIni::CreateFromMemory(const char *pMemory)
{
	MFCALLSTACK;

	MFDebug_Assert(pMemory, "Cant create ini from NULL buffer");

	uint32 memSize = (uint32)MFString_Length(pMemory);
	const char *pMem = pMemory;

	// allocate ini file
	MFIni *pMFIni;
	pMFIni = (MFIni *)MFHeap_Alloc(sizeof(MFIni));
	MFString_Copy(pMFIni->name, "Memory Ini");

	// allocate temporary buffer for strings & lines
	pMFIni->pLines = (MFIniLine *)MFHeap_Alloc(sizeof(MFIniLine)*MAX_LINES);
	pMFIni->pStrings = (const char **)MFHeap_Alloc(4*MAX_STRINGS);
	pMFIni->pCache = MFStringCache_Create(MAX_STRINGCACHE);

	// scan though the file
	pMFIni->lineCount = 0;
	pMFIni->stringCount = 0;
//	const char *pIn = pMem;

	// scan text file
	pMFIni->ScanRecursive(pMem, pMem+memSize);

	// TODO: copy lines, strings & cache to save on memory

	return pMFIni;
}

void MFIni::Destroy(MFIni *pIni)
{
	MFCALLSTACK;

	MFHeap_Free(pIni->pLines);
	MFHeap_Free(pIni->pStrings);
	MFStringCache_Destroy(pIni->pCache);
	MFHeap_Free(pIni);
}

// returns how many lines it found
const char *MFIni::ScanRecursive(const char *pSrc, const char *pSrcEnd)
{
	MFCALLSTACK;

	bool bNewLine = true;
	int tokenLength = 0;
	char tokenBuffer[2048];

	MFIniLine *pCurrLine = &pLines[lineCount];
//	const char **pCurrString = &pStrings[stringCount];

	InitLine(pCurrLine);
	bool bIsSection;
	while (pSrc && (pSrc = ScanToken(pSrc, pSrcEnd, tokenBuffer, pCurrLine->stringCount, &bIsSection)) != NULL)
	{
		// newline
		tokenLength = MFString_Length(tokenBuffer);
		if (tokenLength == 1 && tokenBuffer[0] == 0xd)
		{
			bNewLine = true;
		}
		else if (tokenLength == 1 && tokenBuffer[0] == '{')
		{
			MFDebug_Assert(bNewLine, "open bracket must be at start of line!");

			// new sub section
			int oldLineCount = ++lineCount;
			pSrc = ScanRecursive(pSrc, pSrcEnd);
			pCurrLine->subtreeLineCount = lineCount - oldLineCount;
			lineCount--;
		}
		else if (tokenLength == 1 && tokenBuffer[0] == '}')
		{
			MFDebug_Assert(bNewLine, "close bracket must be at start of line!");

			if (pCurrLine->stringCount != 0 || pCurrLine->subtreeLineCount != 0)
			{
				pCurrLine->terminate = 1;
				lineCount++;
			}
			return pSrc;
		}
		else // must be a string token
		{
			if (bNewLine && (pCurrLine->stringCount != 0 || pCurrLine->subtreeLineCount != 0))
			{
				lineCount++;
				pCurrLine = &pLines[lineCount];
				InitLine(pCurrLine);
			}
			bNewLine = false;

			if (bIsSection)
			{
				pStrings[stringCount++] = MFStringCache_Add(pCache, "section");
				pCurrLine->stringCount++;
			}
			pStrings[stringCount++] = MFStringCache_Add(pCache, tokenBuffer);
			pCurrLine->stringCount++;
		}
	}

	if (pCurrLine->stringCount != 0 || pCurrLine->subtreeLineCount != 0)
	{
		pCurrLine->terminate = 1;
		lineCount++;
	}
	return pSrc;
}

void MFIni::InitLine(MFIniLine *pLine)
{
	MFCALLSTACK;

	pLine->pIni = this;
	pLine->pIni = this;
	pLine->subtreeLineCount = 0;
	pLine->firstString = stringCount;
	pLine->stringCount = 0;
	pLine->terminate = 0;
}

const char *MFIni::ScanToken(const char *pSrc, const char *pSrcEnd, char *pTokenBuffer, int stringCount, bool *pbIsSection)
{
	MFCALLSTACK;

	// skip white space
	while (pSrc < pSrcEnd)
	{
		// skip comment lines
		if ((pSrc[0] == '/' && pSrc[1] == '/') || (pSrc[0] == ';') || (pSrc[0] == '#'))
		{
			while (pSrc < pSrcEnd && pSrc[0] != 0xd)
			{
				pSrc++;
			}
			if (pSrc == pSrcEnd)
				return NULL;
		}

		// check if we have found some non-whitespace
		if (pSrc[0] != ' ' && pSrc[0] != '\t' && pSrc[0] != 0xa && (stringCount!=1 || pSrc[0] != '='))
//		if (pSrc[0] != ' ' && pSrc[0] != '\t' && pSrc[0] != 0xd && pSrc[0] != 0xa && (stringCount!=1 || pSrc[0] != '='))
			break;

		pSrc++;
	}

	// end of file?
	if (pSrc == pSrcEnd)
		return NULL;

	// start of token
	char *pDst = pTokenBuffer;

	// handle special tokens (brackets and EOL)
	if (*pSrc == '{' || *pSrc == '}' || *pSrc == 0xd)
	{
		*pDst++ = *pSrc;
		*pDst++ = 0;
		return ++pSrc;
	}

	// find end of token
	bool bInQuotes = false;
	int sectionDepth = 0;
	*pbIsSection = false;
	while (pSrc < pSrcEnd && *pSrc != 0xd
		    && (bInQuotes || ((stringCount!=0 || *pSrc != '=') && *pSrc != ' ' && *pSrc != '\t' && *pSrc != ',' && (pSrc[0] != '/' || pSrc[1] != '/' ))))
	{
		if (!bInQuotes && *pSrc == '[')
		{
			sectionDepth++;
			*pbIsSection = true;
			pSrc++;
		}
		else if (!bInQuotes && *pSrc == ']')
		{
			sectionDepth--;
			if (sectionDepth < 0)
				MFDebug_Warn(1, "Malformed ini file, Missing '['");
			pSrc++;
		}
		else if (*pSrc == '"')
		{
			bInQuotes = !bInQuotes;
			pSrc++;
		}
		else
		{
			*pDst++ = *pSrc++;
		}
	}

	if (pDst != pTokenBuffer)
	{
		*pDst++ = 0;
		if (*pSrc == ',')
			pSrc++;
		return pSrc;
	}

	return NULL;
}

MFIniLine *MFIni::GetFirstLine()
{
	return pLines;
}

// Log the contents of this line, and following lines to the screen
// Mainly for debugging purposes
void MFIniLine::DumpRecursive(int depth)
{
	MFCALLSTACK;

	char prefix[256];
	int c;
	for (c=0; c<depth*2; c++)
	{
		prefix[c] = ' ';
	}
	prefix[c]=0;

	MFIniLine *pLine = this;

	char buffer[256];
	while (pLine)
	{
		MFString_Copy(buffer,prefix);
		for (int i=0; i<pLine->GetStringCount(); i++)
		{
			MFString_Cat(buffer, MFStr("'%s'",pLine->GetString(i)));
			MFString_Cat(buffer, "  ");
		}
		MFDebug_Message(buffer);
		if (pLine->Sub())
		{
			MFDebug_Message(MFStr("%s{",prefix));
			pLine->Sub()->DumpRecursive(depth+1);
			MFDebug_Message(MFStr("%s}",prefix));
		}
		pLine = pLine->Next();
	}
}

