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

Font debugFont;

void Font_InitModule()
{
	CALLSTACK;

	debugFont.LoadFont(File_SystemPath("Font/Arial"));
}

void Font_DeinitModule()
{
	CALLSTACK;

	debugFont.Release();
}

int Font::LoadFont(const char *filename)
{
	CALLSTACK;

	uint32 hfile;
	char tempbuffer[1024];

	int a;
	for(a=strlen(filename); a>0 && filename[a-1] != '\\' && filename[a-1] != '/'; a--) {}

	pMaterial = Material::Create(&filename[a]);

	strcpy(tempbuffer, filename);
	strcat(tempbuffer, ".dat");

	hfile = File_Open(tempbuffer, OF_Read|OF_Binary);
	File_Read(charwidths, 256, hfile);
	File_Close(hfile);

	return 0;
}

void Font::Release()
{
	CALLSTACK;

	pMaterial->Release();
}

int Font::DrawText(float pos_x, float pos_y, float pos_z, float height, uint32 colour, const char *text, bool invert)
{
	CALLSTACKc;

	int textlen = strlen(text);

	DBGASSERT(textlen < 2048, "Exceeded Font Vertex Buffer Limit");

	float x,y,w,h, p, cwidth;

	pMaterial->Use();
	MFPrimitive(PT_TriList|PT_Prelit);

	MFBegin(textlen*2*3);

	for(int i=0; i<textlen; i++)
	{
		x = (float)((uint8)text[i] & 0x0F) * (float)(pMaterial->pTextures[0]->width / 16);
		y = (float)((uint8)text[i] >> 4) * (float)(pMaterial->pTextures[0]->height / 16);

		w = (float)charwidths[(uint8)text[i]];
		h = (float)pMaterial->pTextures[0]->height/16.0f;

		x /= (float)pMaterial->pTextures[0]->width;
		y /= (float)pMaterial->pTextures[0]->height;
		w /= (float)pMaterial->pTextures[0]->width;
		h /= (float)pMaterial->pTextures[0]->height;

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

int Font::DrawText(float pos_x, float pos_y, float height, uint32 colour, const char *text, bool invert)
{
	return DrawText(pos_x, pos_y, 0, height, colour, text, invert);
}

int Font::DrawText(const Vector3 &pos, float height, uint32 colour, const char *text, bool invert)
{
	return DrawText(pos.x, pos.y, pos.z, height, colour, text, invert);
}

int Font::DrawTextf(float pos_x, float pos_y, float height, uint32 colour, const char *format, ...)
{
	CALLSTACK;

	char buffer[1024];

	va_list args;
	va_start(args, format);

	vsprintf(buffer, format, args);

	return DrawText(pos_x, pos_y, 0, height, colour, buffer);
}

int Font::DrawTextf(float pos_x, float pos_y, float pos_z, float height, uint32 colour, const char *format, ...)
{
	CALLSTACK;

	char buffer[1024];

	va_list args;
	va_start(args, format);

	vsprintf(buffer, format, args);

	return DrawText(pos_x, pos_y, pos_z, height, colour, buffer);
}

int Font::DrawTextf(const Vector3 &pos, float height, uint32 colour, const char *format, ...)
{
	CALLSTACK;

	char buffer[1024];

	va_list args;
	va_start(args, format);

	vsprintf(buffer, format, args);

	return DrawText(pos.x, pos.y, pos.z, height, colour, buffer);
}
