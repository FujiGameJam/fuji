#include <stdio.h>

#include "Common.h"
#include "Util.h"
#include "Vector3.h"
#if !defined(_FUJI_UTIL)
#include "Display.h"
#include "Primitive.h"
#include "Font.h"
#include "View.h"
#endif

char stringBuffer[1024*128];
uint32 stringOffset;

// Return only the last portion of the module name (exclude the path)
char *ModuleName(char *pSourceFileName)
{
	int x = (int)strlen(pSourceFileName)-1;

	while(x >= 0 && pSourceFileName[x] != '/' && pSourceFileName[x] != '\\')
	{
		--x;
	}

	return x == -1 ? pSourceFileName : &pSourceFileName[x+1];
}

#if defined(_WINDOWS)

bool Debug_MsgBox(const char *pMessageText, const char *pTitle, const char *pTime)
{
	bool bResult = false;

	if(IDYES == MessageBox(NULL, pMessageText, pTitle, MB_YESNO | MB_ICONSTOP | MB_TOPMOST))
	{
		bResult = true;
	}

	return bResult;
}

#else

bool Debug_MsgBox(const char *pMessageText, const char *pTitle, const char *pTime)
{
	bool bResult = false;

	printf("%s: %s", pTitle, pMessageText);

	return bResult;
}

#endif

#if defined(_FUJI_UTIL)

void dbgAssert(const char *pReason, const char *pMessage, const char *pFile, int line)
{
	LOGD(STR("%s(%d) : Assertion Failure.",pFile,line));
	LOGD(STR("Failed Condition: (%s)\n%s", pReason, pMessage));

	// query for debug or exit of process
	if(!Debug_MsgBox(STR("Failed Condition: (%s)\n%s\nFile: %s\nLine: %d", pReason, pMessage, pFile, line), "Assertion Failure, do you wish to debug?", "timewouldgohere"))
	{
		ExitProcess(0);
	}
}

#else

#if !defined(_RETAIL)

void Callstack_Log()
{
	for(int a=Callstack.size()-1; a>=0; a--)
	{
		LOGD(STR("  %s", Callstack[a]));
	}
}

#if defined(_XBOX)

void dbgAssert(const char *pReason, const char *pMessage, const char *pFile, int line)
{
//	TimerInfo time,diff;
//	Timer_GetSystemTime(&time);
//	Timer_GetDifference(&diff,&startTime,&time);

	LOGD(STR("%s(%d) : Assertion Failure.",pFile,line));
	LOGD(STR("Failed Condition: (%s)\n%s", pReason, pMessage));
/*
  char *pDayStr = gNullStr;
  switch (time.day)
  {
    case 1: 
    case 21:
    case 31:
      pDayStr = "st"; 
      break;
    case 2: 
    case 22:
    case 32:
      pDayStr = "nd"; 
      break;
    case 3:
    case 23:
    case 33:
      pDayStr = "rd"; 
      break;
    default: pDayStr = "th"; break;
  }
  LOGD(STR("Time : (%d%s):%02d:%02d:%02d (%02d:%02d:%02d from start)",time.day,pDayStr,time.hours,time.minutes,time.seconds,diff.day*24+diff.hours,diff.minutes,diff.seconds));
*/

	Callstack_Log();
}

#elif defined(_WINDOWS)

void dbgAssert(const char *pReason, const char *pMessage, const char *pFile, int line)
{
	LOGD(STR("%s(%d) : Assertion Failure.",pFile,line));
	LOGD(STR("Failed Condition: (%s)\n%s", pReason, pMessage));

	// build callstack log string for message box
#if !defined(_RETAIL)
	Callstack_Log();

	char callstack[2048] = "";

	for(int a=Callstack.size()-1; a>=0; a--)
	{
		char *pTemp = STR("  %-32s\n",Callstack[a]);
//		char *pTemp = STR("  %-32s\t(%s)%s\n",Callstack[a].c_str(),ModuleName(pFunc->pStats->pModuleName),pFunc->pComment ? STR(" [%s]",pFunc->pComment) : "");
		if(strlen(callstack) + strlen(pTemp) < sizeof(callstack) - 1)
		strcat(callstack, pTemp);
	}

#else
	char callstack[] = "Not available in _RETAIL builds";
#endif

	// query for debug or exit of process
	if(!Debug_MsgBox(STR("Failed Condition: (%s)\n%s\nFile: %s\nLine: %d\n\nCallstack:\n%s", pReason, pMessage, pFile, line, callstack),
						"Assertion Failure, do you wish to debug?",
						"timewouldgohere"))
	{
		ExitProcess(0);
	}
}
#elif defined(_LINUX)
void dbgAssert(const char *pReason, const char *pMessage, const char *pFile, int line)
{
	LOGD(STR("%s(%d) : Assertion Failure.",pFile,line));
	LOGD(STR("Failed Condition: (%s)\n%s", pReason, pMessage));

	// build callstack log string for message box

#if !defined(_RETAIL)
	Callstack_Log();
#endif
}
#endif

void hardAssert(const char *pReason, const char *pMessage, const char *pFile, int line)
{
	LOGD(STR("%s(%d) : Assertion Failure.",pFile,line));
	LOGD(STR("Failed Condition: %s\n%s", pReason, pMessage));
	Callstack_Log();

	while(1)
	{
#if defined(_WINDOWS)
		DoMessageLoop();
#endif
		System_UpdateTimeDelta();
		gFrameCount++;

		System_Update();

		Display_BeginFrame();

		View_SetDefault();
		View_SetOrtho();

		// Set some renderstates
#if defined(_WINDOWS) || defined(_XBOX)		
		pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
#endif

		pd3dDevice->SetTexture(0, NULL);

		MFPrimitive(PT_TriStrip);

		MFBegin(4);
		MFSetColour(0xFF000000);
		MFSetPosition(0, 0, 0);
		MFSetPosition(640, 0, 0);
		MFSetPosition(0, 480, 0);
		MFSetPosition(640, 480, 0);
		MFEnd();

		if(!(((uint32)gSystemTimer.GetSecondsF()) % 2))
		{
			MFBegin(4);
			MFSetColour(0xFFFF0000);
			MFSetPosition(50, 50, 0);
			MFSetPosition(590, 50, 0);
			MFSetPosition(50, 110, 0);
			MFSetPosition(590, 110, 0);
			MFEnd();

			MFBegin(4);
			MFSetColour(0xFF000000);
			MFSetPosition(55, 55, 0);
			MFSetPosition(585, 55, 0);
			MFSetPosition(55, 105, 0);
			MFSetPosition(585, 105, 0);
			MFEnd();
		}

		Font_DrawTextf(gpDebugFont, 110, 60, 20, 0xFFFF0000, "Software Failure. Press left mouse button to continue");
		Font_DrawTextf(gpDebugFont, 240, 80, 20, 0xFFFF0000, "Guru Meditation: ");

		Font_DrawTextf(gpDebugFont, 80, 120, 20, 0xFFFF0000, "Assertion Failure:");
		Font_DrawTextf(gpDebugFont, 80, 140, 20, 0xFFFF0000, STR("Failed Condition: %s", pReason));
		Font_DrawTextf(gpDebugFont, 80, 160, 20, 0xFFFF0000, STR("File: %s, Line: %d", pFile, line));
		Font_DrawTextf(gpDebugFont, 80, 190, 20, 0xFFFF0000, STR("Message: %s", pMessage));

#if !defined(_RETAIL)
		Font_DrawTextf(gpDebugFont, 80, 230, 20, 0xFFFF0000, "Callstack:");
		float y = 250.0f;
		for(int a=Callstack.size()-1; a>=0; a--)
		{
			Font_DrawTextf(gpDebugFont, 100, y, 20, 0xFFFF0000, Callstack[a]);
			y+=20.0f;
		}
#else
		Font_DrawTextf(gpDebugFont, 80, 230, 20, 0xFFFF0000, "Callstack not available in _RETAIL builds");
#endif

		Display_EndFrame();
	}
}

#endif // _RETAIL
#endif // _FUJI_UTIL

//
// Output a formatted string to the debugger.
//
int dprintf(const char *format, ...)
{
	char buffer[2048];
	va_list arglist;
	int nRes = 0;

	va_start(arglist, format);

	nRes = vsprintf((char*)buffer, format, arglist);

#if defined(_WINDOWS) || defined(_XBOX)
	OutputDebugString((LPCTSTR)buffer);
#else
	fprintf(stderr, buffer);
#endif

	va_end(arglist);
	return nRes;
}

void LOGD(const char *string)
{
	dprintf("%s\n", string);
}

char* STR(const char *format, ...)
{
	va_list arglist;
	char *buffer = &stringBuffer[stringOffset];
	int nRes = 0;

	va_start(arglist, format);

	nRes = vsprintf(buffer, format, arglist);
	stringOffset += nRes+1;

	if(stringOffset >= sizeof(stringBuffer) - 1024) stringOffset = 0;

	va_end(arglist);

	return buffer;
}

char* STRn(const char *source, int n)
{
	char *buffer = &stringBuffer[stringOffset];

	strncpy(buffer, source, n);
	buffer[n] = NULL;

	stringOffset += n+1;

	if(stringOffset >= sizeof(stringBuffer) - 1024) stringOffset = 0;

	return buffer;
}

uint32 Rand()
{
	return rand();
}

float RandomUnit()
{
	return (float)rand() / RAND_MAX;
}

float RandomRange(float min, float max)
{
	return ((float)rand()/RAND_MAX)*(max-min) + min;
}

Vector3 RandomVector()
{
	Vector3 t;

	t.x = RandomUnit();
	t.y = RandomUnit();
	t.z = RandomUnit();

	return t;
}

int StrCaseCmp(const char *s1, const char *s2)
{
	while(*s1 != '\0' && tolower(*s1) == tolower(*s2))
	{
		s1++;
		s2++;
	}

	return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}

int StrNCaseCmp(const char *s1, const char *s2, size_t n)
{
	if(n == 0)
		return 0;

	while(n-- != 0 && tolower(*s1) == tolower(*s2))
	{
		if(n == 0 || *s1 == '\0' || *s2 == '\0')
			break;
		s1++;
		s2++;
	}

	return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}


char* StrRChr(const char *s, int i)
{
	const char *last = NULL;

	if(i)
	{
		while(s=strchr(s, i))
		{
			last = s;
			s++;
		}
	}
	else
	{
		last = strchr(s, i);
	}

	return (char *) last;
}
