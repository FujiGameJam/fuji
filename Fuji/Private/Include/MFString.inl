
// some useful string parsing functions
inline bool MFIsWhite(char c)
{
	return c==' ' || c=='\t';
}

inline bool MFIsAlpha(char c)
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z');
}

inline bool MFIsNumeric(char c)
{
	return c>='0' && c<='9';
}

inline bool MFIsAlphaNumeric(char c)
{
	return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || (c=='_');
}

inline bool MFIsNewline(char c)
{
	return c=='\n' || c=='\r';
}

inline int MFToLower(int c)
{
	return c | 0x20;
}

inline int MFToUpper(int c)
{
	return c & ~0x20;
}

inline char* MFSeekNewline(char *pC)
{
	while(!MFIsNewline(*pC) && *pC != 0) pC++;
	while(MFIsNewline(*pC)) pC++;
	return pC;
}

inline const char* MFSeekNewline(const char *pC)
{
	while(!MFIsNewline(*pC) && *pC != 0) pC++;
	while(MFIsNewline(*pC)) pC++;
	return pC;
}

inline char* MFSkipWhite(char *pC)
{
	while(MFIsWhite(*pC)) pC++;
	return pC;
}

inline const char* MFSkipWhite(const char *pC)
{
	while(MFIsWhite(*pC)) pC++;
	return pC;
}

inline int MFString_Length(const char *pString)
{
	int l = 0;

	while(*pString)
	{
		++l;
		++pString;
	}

	return l;
}

inline void MFString_Copy(char *pBuffer, const char *pString)
{
	while(*pString)
	{
		*pBuffer = *pString;
		++pBuffer;
		++pString;
	}

	*pBuffer = 0;
}

inline void MFString_CopyN(char *pBuffer, const char *pString, int maxChars)
{
	while(*pString && maxChars)
	{
		*pBuffer = *pString;
		++pBuffer;
		++pString;
		--maxChars;
	}

	*pBuffer = 0;
}
