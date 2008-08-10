/**
 * @file MFString.h
 * @brief Mount Fuji string funcitons.
 * Provides a set of useful string management functions.
 * @author Manu Evans
 * @defgroup MFString String Functions
 * @{
 */

#if !defined(_MFSTRING_H)
#define _MFSTRING_H

/**
 * Copy memory.
 * Copy memory from one location to another.
 * @param pDest Pointer to the destination buffer.
 * @param pSrc Pointer to the source buffer.
 * @param size Size of data to copy in bytes.
 * @return None.
 */
void MFCopyMemory(void *pDest, const void *pSrc, uint32 size);

/**
 * Zero memory.
 * Initialise a region of memory to zero.
 * @param pDest Pointer to the target buffer.
 * @param size Size of data to initialise in bytes.
 * @return None.
 */
void MFZeroMemory(void *pDest, uint32 size);

/**
 * Initialise memory.
 * Initialise a region of memory to a specified value.
 * @param pDest Pointer to the target buffer.
 * @param value Value written to each byte in the memory region.
 * @param size Number of bytes to initialise.
 * @return None.
 */
void MFMemSet(void *pDest, int value, uint32 size);

/**
 * Compare memory.
 * Compare bytes returning the difference.
 * @param pBuf1 Pointer to the first buffer.
 * @param pBuf2 Pointer to the second buffer.
 * @param size Number of bytes to compate.
 * @return Return the difference of the first encountered differing byte. Returns 0 if buffers are identical.
 */
int MFMemCompare(const void *pBuf1, const void *pBuf2, uint32 size);

/**
 * Get the length of a string.
 * Get the length of a string, in bytes.
 * @param pString String to find the length of.
 * @return Returns the length of the string, in bytes, excluding the terminating NULL character.
 * @see MFString_Copy()
 * @see MFString_CaseCmp()
 */
int MFString_Length(const char *pString);

/**
 * Copy a string.
 * Copies the source string to the target buffer.
 * @param pBuffer Target buffer to receive a copy of the string.
 * @param pString Source string.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFString_CopyN()
 */
char* MFString_Copy(char *pBuffer, const char *pString);

/**
 * Copy a string with a maximum number of characters.
 * Copies the source string or the maximum number of characters from the source string to the target buffer.
 * @param pBuffer Target buffer to receive a copy of the string.
 * @param pString Source string.
 * @param maxChars Maximum characters to copy.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFString_Copy()
 */
char* MFString_CopyN(char *pBuffer, const char *pString, int maxChars);

/**
 * Concatinate a string.
 * Concatinate the source string with the target buffer.
 * @param pBuffer Target buffer to receive the concatinated string.
 * @param pString Source string.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFString_Copy()
 */
char* MFString_Cat(char *pBuffer, const char *pString);

/**
 * Concatinate 2 strings into a target buffer.
 * Concatinates the 2 strings into the target buffer.
 * @param pBuffer Target buffer to receive a copy of the string.
 * @param pString Source string.
 * @param pString2 Second source string.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFString_CopyN()
 */
char* MFString_CopyCat(char *pBuffer, const char *pString, const char *pString2);

/**
 * Convert a string to lower case.
 * Generates a copy of a string converted to lower case in the MFStr buffer.
 * @param pString String to convert.
 * @return Returns a pointer to a string in the MFStr buffer converted to lower case.
 * @see MFString_ToUpper()
 */
const char * MFString_ToLower(const char *pString);

/**
 * Convert a string to upper case.
 * Generates a copy of a string converted to upper case in the MFStr buffer.
 * @param pString String to convert.
 * @return Returns a pointer to a string in the MFStr buffer converted to upper case.
 * @see MFString_ToLower()
 */
const char * MFString_ToUpper(const char *pString);

/**
 * Compares 2 strings.
 * Compares 2 strings and returns the difference between them.
 * @param pString1 First source string.
 * @param pString2 Second source string.
 * @return Returns the difference between the 2 strings. 0 if the strings are identical.
 * @see MFString_CompareN()
 */
int MFString_Compare(const char *pString1, const char *pString2);

/**
 * Compares 2 strings with a maximum character limit.
 * Compares 2 strings with a maximum character limit and returns the difference between them.
 * @param pString1 First source string.
 * @param pString2 Second source string.
 * @param n Maximum number of characters to compare.
 * @return Returns the difference between the 2 strings. 0 if the strings are identical.
 * @see MFString_Compare()
 */
int MFString_CompareN(const char *pString1, const char *pString2, int n);

/**
 * Compares 2 strings with case insensitivity.
 * Compares 2 strings with case insensitivity and returns the difference between them.
 * @param pString1 First source string.
 * @param pString2 Second source string.
 * @return Returns the difference between the 2 strings. 0 if the strings are identical.
 * @see MFString_CaseCmpN()
 */
int MFString_CaseCmp(const char *pString1, const char *pString2);

/**
 * Compares 2 strings with case insensitivity and a maximum character limit.
 * Compares 2 strings with case insensitivity and a maximum character limit and returns the difference between them.
 * @param pString1 First source string.
 * @param pString2 Second source string.
 * @param n Maximum number of characters to compare.
 * @return Returns the difference between the 2 strings. 0 if the strings are identical.
 * @see MFString_CaseCmp()
 */
int MFString_CaseCmpN(const char *pString1, const char *pString2, uint32 n);

/**
 * Searches through a string for the specified character.
 * Searches through a string for the specified character.
 * @param pString String to search.
 * @param c Character to search for.
 * @return Returns a pointer to the first instance of the character \a c in \a pString.
 * @see MFString_RChr()
 */
char* MFString_Chr(const char *pString, int c);

/**
 * Searches backwards through a string for the specified character.
 * Searches backwards through a string for the specified character.
 * @param pString String to search.
 * @param c Character to search for.
 * @return Returns a pointer to the last instance of the character \a c in \a pString.
 * @see MFString_Chr()
 */
char* MFString_RChr(const char *pString, int c);

/**
 * Get file extension from filename.
 * Get the file extension part from a filename.
 * @param pFilename String containing the filename.
 * @return Returns a pointer to the file extension (the part after the final '.' in the filename).
 * @remarks The pointer returned points into the source string. It is only valid while the source string remains unchanged.
 */
char* MFString_GetFileExtension(const char *pFilename);

/**
 * Get the filename from a path.
 * Gets the filename part from a path.
 * @param pFilename String containing a file path.
 * @return Returns a string containing just the filename taken from the path.
 * @remarks The pointer returned points into the source string. It is only valid while the source string remains unchanged.
 */
const char* MFStr_GetFileName(const char *pFilename);

/**
 * Get the filename without its extension from a path.
 * Gets the filename without its extension from a path.
 * @param pFilename String containing a file path.
 * @return Returns a string containing just the filename taken from the path with the file extension removed.
 * @remarks The pointer returned is in the MFStr buffer. If persistence is desired, the client should take a copy.
 */
const char* MFStr_GetFileNameWithoutExtension(const char *pFilename);

/**
 * Get the file path from a path string.
 * Gets the file path with the filename removed from a path string.
 * @param pFilename String containing a file path.
 * @return Returns a string containing just the file path (everything preceeding the final '/').
 * @remarks The pointer returned is in the MFStr buffer. If persistence is desired, the client should take a copy.
 */
const char* MFStr_GetFilePath(const char *pFilename);

/**
 * Truncate the file extension.
 * Truncates the file extension from a filename.
 * @param pFilename String containing a filename.
 * @return Returns a string containing the filename with the final file extension truncated.
 * @remarks The pointer returned is in the MFStr buffer. If persistence is desired, the client should take a copy.
 */
const char* MFStr_TruncateExtension(const char *pFilename);

/**
 * String pattern match.
 * Performs a string pattern match.
 * @param pPattern String containing the filename pattern to test. May contain wild cards, etc.
 * @param pFilename String containing the filename to compare against.
 * @param ppMatchDirectory Cant remember what this is used for. Dont use it...
 * @return bool Returns true if the pattern matches the filename.
 */
bool MFString_PatternMatch(const char *pPattern, const char *pFilename, const char **ppMatchDirectory = NULL);

/**
 * Get a formatted a string.
 * Generates a new string from a format specifier and parameters.
 * @param format The format string. Supports all the standard formatting provided by the CRT like printf().
 * @param ... Variable argument list.
 * @return Pointer to the newly created string.
 * @remarks MFStr() uses a circular buffer for storing the strings it produces. You should NOT keep a long term string generated by MFStr. MFStr() is best suited to generating temporary and intermediate strings for immediate use.
 * @see MFStrN()
 */
const char* MFStr(const char *format, ...);

/**
 * Copy a specified number of characters from one string into a new string.
 * Generates a new string from a format specifier and parameters.
 * @param source The string from which characters will be copied.
 * @param n Number of characters to copy from the source string.
 * @return Pointer to the newly created string.
 * @remarks MFStrN() uses a circular buffer for storing the strings it produces. You should NOT keep a long term string generated by MFStrN. MFStrN() is best suited to generating temporary and intermediate strings for immediate use.
 * @see MFStrN()
 */
const char* MFStrN(const char *source, size_t n);

/**
 * Test if the specified character is a while space character.
 */
bool MFIsWhite(int c);

/**
 * Test if the specified character is an alphabetic character.
 */
bool MFIsAlpha(int c);

/**
 * Test if the specified character is a numeric character.
 */
bool MFIsNumeric(int c);

/**
 * Test if the specified character is an alphabetic or numeric character.
 */
bool MFIsAlphaNumeric(int c);

/**
 * Test if the specified character is a hexadecimal character.
 */
bool MFIsHex(int c);

/**
 * Test if the specified character is a newline character.
 */
bool MFIsNewline(int c);

/**
 * Test if a character is a lower case alphabetic character.
 */
bool MFIsLower(int c);

/**
 * Test if a character is an upper case alphabetic character.
 */
bool MFIsUpper(int c);

/**
 * Converts a character to lower case.
 */
int MFToLower(int c);

/**
 * Converts a character to lower case.
 */
int MFToUpper(int c);

/**
 * Returns a pointer to the start of the next line in the provided string.
 */
char* MFSeekNewline(char *pC);

/**
 * Returns a pointer to the next non-white space character in the provided string.
 */
char* MFSkipWhite(char *pC);

//
// UTF8 support
//

/**
 * Get the number of actual characters in a string.
 * Gets the number of actual characters in a string, with consideration to UTF8 multibyte encoding.
 * @param pString String to find the number of characters.
 * @return Returns the number of actual characters in the string, excluding the terminating NULL character.
 * @remarks Takes into account UTF8 multibyte encoding when calculating the number of characters.
 * @see MFString_GetCharacterOffset()
 */
int MFString_GetNumChars(const char *pString);

/**
 * Get the character offset from the start of a string.
 * Gets the character offset in bytes from the start of a string, with consideration to UTF8 multibyte encoding.
 * @param pString String to find the character offset.
 * @param character Character index to find the offset of.
 * @return Returns the character offset in bytes of the specified character index.
 * @remarks Takes into account UTF8 multibyte encoding when calculating the character offset.
 * @see MFString_GetNumChars()
 */
int MFString_GetCharacterOffset(const char *pString, int character);

/**
 * Get the number of bytes in a multibyte character.
 * Gets the number of bytes in a multibyte encoded character.
 * @param pMBChar Pointer to a multibyte character sequence.
 * @return Returns the number of bytes in the multibyte character sequence8
 */
int MFString_GetNumBytesInMBChar(const char *pMBChar);

char *MFString_NextChar(const char *pString);
char *MFString_PrevChar(const char *pString);

int MFString_MBToWChar(const char *pMBChar, uint16 *pWC);
int MFString_WCharToMB(int wc, char *pMBChar);

int MFString_UFT8ToWChar(uint16 *pBuffer, const char *pUTF8String);
int MFString_ToUFT8(char *pBuffer, const char *pString);

uint16* MFString_UFT8AsWChar(const char *pUTF8String, int *pNumChars);

//
// unicode support
//

/**
 * Convert a unicode string to a UTF8 string.
 * Converts a unicode string to a UTF8 string placing the result in the target buffer.
 * @param pBuffer Target buffer to output the resulting string.
 * @param pUnicodeString Unicode string to convert.
 * @return Returns the number of characters copied, NOT the length of the result string in bytes.
 * @see MFString_UFT8ToWChar()
 * @see MFString_UFT8AsWChar()
 */
int MFWString_ToUFT8(char *pBuffer, const uint16 *pUnicodeString);

/**
 * Get the length of a unicode string.
 * Get the length of a unicode string, in wide chars.
 * @param pString String to find the length of.
 * @return Returns the length of the unicode string, in wide chars, excluding the terminating NULL character.
 * @see MFWString_Copy()
 * @see MFWString_CaseCmp()
 */
int MFWString_Length(const uint16 *pString);

/**
 * Copy a unicode string.
 * Copies the source unicode string to the target buffer.
 * @param pBuffer Target buffer to receive a copy of the string.
 * @param pString Source string.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFWString_CopyN()
 */
uint16* MFWString_Copy(uint16 *pBuffer, const uint16 *pString);

/**
 * Copy a unicode string with a maximum number of characters.
 * Copies the source unicode string or the maximum number of characters from the source unicode string to the target buffer.
 * @param pBuffer Target buffer to receive a copy of the string.
 * @param pString Source string.
 * @param maxChars Maximum characters to copy.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFWString_Copy()
 */
uint16* MFWString_CopyN(uint16 *pBuffer, const uint16 *pString, int maxChars);

/**
 * Concatinate a unicode string.
 * Concatinate the source unicode string with the target buffer.
 * @param pBuffer Target buffer to receive the concatinated string.
 * @param pString Source string.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFWString_Copy()
 */
uint16* MFWString_Cat(uint16 *pBuffer, const uint16 *pString);

/**
 * Concatinate 2 unicode strings into a target buffer.
 * Concatinates the 2 unicode strings into the target buffer.
 * @param pBuffer Target buffer to receive a copy of the string.
 * @param pString Source string.
 * @param pString2 Second source string.
 * @return \a pBuffer which can be used as a paramater to other functions.
 * @see MFWString_CopyN()
 */
uint16* MFWString_CopyCat(uint16 *pBuffer, const uint16 *pString, const uint16 *pString2);

/**
 * Compares 2 unicode strings.
 * Compares 2 unicode strings and returns the difference between them.
 * @param pString1 First source string.
 * @param pString2 Second source string.
 * @return Returns the difference between the 2 strings. 0 if the strings are identical.
 * @see MFWString_CompareN()
 */
int MFWString_Compare(const uint16 *pString1, const uint16 *pString2);

/**
 * Compares 2 unicode strings with case insensitivity.
 * Compares 2 unicode strings with case insensitivity and returns the difference between them.
 * @param pString1 First source string.
 * @param pString2 Second source string.
 * @return Returns the difference between the 2 unicode strings. 0 if the strings are identical.
 * @see MFWString_CaseCmpN()
 */
int MFWString_CaseCmp(const uint16 *pString1, const uint16 *pString2);

/**
 * Returns a pointer to the start of the next line in the provided unicode string.
 */
uint16* MFSeekNewlineW(uint16 *pC);

/**
 * Returns a pointer to the next non-white space character in the provided unicode string.
 */
uint16* MFSkipWhiteW(uint16 *pC);

#include "MFString.inl"

#endif // _MFSTRING_H

/** @} */
