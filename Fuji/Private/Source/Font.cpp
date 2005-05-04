#include <string.h>
#include <stdio.h>

#include "Common.h"
#include "Display.h"
#include "FileSystem.h"
#include "Texture.h"
#include "Material.h"
#include "Font.h"
#include "Primitive.h"
#include "FileSystem.h"

Font *gpDebugFont;

void Font_InitModule()
{
	CALLSTACK;

	gpDebugFont = Font_Create("Font/Arial");
}

void Font_DeinitModule()
{
	CALLSTACK;

	Font_Destroy(gpDebugFont);
}

Font* Font_Create(const char *pFilename)
{
	CALLSTACK;

	Font *pFont = NULL;

	int hFile;
	char tempbuffer[1024];

	int a;
	for(a=strlen(pFilename); a>0 && pFilename[a-1] != '\\' && pFilename[a-1] != '/'; a--) {}

	hFile = File_Open(File_SystemPath(tempbuffer), OF_Read|OF_Binary);
	DBGASSERT(hFile > -1, STR("Unable to open charinfo file for font '%s'", pFilename));

	if(hFile > -1)
	{
		pFont = (Font*)Heap_Alloc(sizeof(Font));

		pFont->pMaterial = Material_Create(&pFilename[a]);

		strcpy(tempbuffer, pFilename);
		strcat(tempbuffer, ".dat");

		File_Read(pFont->charwidths, 256, hFile);
		File_Close(hFile);
	}

	return pFont;
}

void Font_Destroy(Font *pFont)
{
	CALLSTACK;

	Material_Destroy(pFont->pMaterial);
	Heap_Free(pFont);
}

int Font_DrawText(Font *pFont, float pos_x, float pos_y, float pos_z, float height, uint32 colour, const char *text, bool invert)
{
	CALLSTACKc;

	int textlen = strlen(text);

	DBGASSERT(textlen < 2048, "Exceeded Font Vertex Buffer Limit");

	float x,y,w,h, p, cwidth;

	Material_Use(pFont->pMaterial);
	MFPrimitive(PT_TriList|PT_Prelit);

	MFBegin(textlen*2*3);

	for(int i=0; i<textlen; i++)
	{
		x = (float)((uint8)text[i] & 0x0F) * (float)(pFont->pMaterial->pTextures[0]->width / 16);
		y = (float)((uint8)text[i] >> 4) * (float)(pFont->pMaterial->pTextures[0]->height / 16);

		w = (float)pFont->charwidths[(uint8)text[i]];
		h = (float)pFont->pMaterial->pTextures[0]->height/16.0f;

		x /= (float)pFont->pMaterial->pTextures[0]->width;
		y /= (float)pFont->pMaterial->pTextures[0]->height;
		w /= (float)pFont->pMaterial->pTextures[0]->width;
		h /= (float)pFont->pMaterial->pTextures[0]->height;

		p = w/h;
		cwidth = height*p;

		MFSetColour(colour);
		MFSetTexCoord1(x, y);
		MFSetPosition(pos_x, pos_y, pos_z);
		MFSetTexCoord1(x+w, y+h);
		MFSetPosition(pos_x + cwidth, pos_y + (invert?-height:height), pos_z);
		MFSetTexCoord1(x, y+h);
		MFSetPosition(pos_x, pos_y + (invert?-height:height), pos_z);
		MFSetTexCoord1(x, y);
		MFSetPosition(pos_x, pos_y, pos_z);
		MFSetTexCoord1(x+w, y);
		MFSetPosition(pos_x + cwidth, pos_y, pos_z);
		MFSetTexCoord1(x+w, y+h);
		MFSetPosition(pos_x + cwidth, pos_y + (invert?-height:height), pos_z);

		pos_x += cwidth;
	}

	MFEnd();

	return 0;
}

int Font_DrawText(Font *pFont, float pos_x, float pos_y, float height, uint32 colour, const char *text, bool invert)
{
	return Font_DrawText(pFont, pos_x, pos_y, 0, height, colour, text, invert);
}

int Font_DrawText(Font *pFont, const Vector3 &pos, float height, uint32 colour, const char *text, bool invert)
{
	return Font_DrawText(pFont, pos.x, pos.y, pos.z, height, colour, text, invert);
}

int Font_DrawTextf(Font *pFont, float pos_x, float pos_y, float height, uint32 colour, const char *format, ...)
{
	CALLSTACK;

	char buffer[1024];

	va_list args;
	va_start(args, format);

	vsprintf(buffer, format, args);

	return Font_DrawText(pFont, pos_x, pos_y, 0, height, colour, buffer);
}

int Font_DrawTextf(Font *pFont, float pos_x, float pos_y, float pos_z, float height, uint32 colour, const char *format, ...)
{
	CALLSTACK;

	char buffer[1024];

	va_list args;
	va_start(args, format);

	vsprintf(buffer, format, args);

	return Font_DrawText(pFont, pos_x, pos_y, pos_z, height, colour, buffer);
}

int Font_DrawTextf(Font *pFont, const Vector3 &pos, float height, uint32 colour, const char *format, ...)
{
	CALLSTACK;

	char buffer[1024];

	va_list args;
	va_start(args, format);

	vsprintf(buffer, format, args);

	return Font_DrawText(pFont, pos.x, pos.y, pos.z, height, colour, buffer);
}
