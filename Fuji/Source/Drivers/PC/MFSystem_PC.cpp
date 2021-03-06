#include "Fuji.h"

#if MF_SYSTEM == MF_DRIVER_PC

#include "MFSystem_Internal.h"
#include "MFHeap.h"
#include "MFThread.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// support delay loading of DLL's in windows build.
#pragma comment(lib, "delayimp")

extern MFInitParams gInitParams;

HINSTANCE apphInstance;
extern int gQuit;

MFPlatform gCurrentPlatform = FP_Windows;

char *gpCommandLineBuffer = NULL;

#if !defined(_FUJI_UTIL)
void MFSystem_InitModulePlatformSpecific()
{
	apphInstance = (HINSTANCE)gInitParams.hInstance;
	gpCommandLineBuffer = (char*)gInitParams.pCommandLine;

	// HACK: this is to force dual-core or multi-processor systems to use the first cpu for timing.
//	DWORD proc, system;
//	GetProcessAffinityMask(GetCurrentProcess(), &proc, &system);
	SetProcessAffinityMask(GetCurrentProcess(), 1);

	if(gDefaults.system.threadPriority != MFPriority_Normal)
	{
		int priority = THREAD_PRIORITY_NORMAL;

		switch(gDefaults.system.threadPriority)
		{
			case MFPriority_Normal:			priority = THREAD_PRIORITY_NORMAL;			break;
			case MFPriority_AboveNormal:	priority = THREAD_PRIORITY_ABOVE_NORMAL;	break;
			case MFPriority_BelowNormal:	priority = THREAD_PRIORITY_BELOW_NORMAL;	break;
			case MFPriority_Maximum:		priority = THREAD_PRIORITY_HIGHEST;			break;
			case MFPriority_Low:			priority = THREAD_PRIORITY_LOWEST;			break;
			case MFPriority_Idle:			priority = THREAD_PRIORITY_IDLE;			break;
			default:
				break;
		}

		SetThreadPriority(GetCurrentThread(), priority);
	}
}

void MFSystem_DeinitModulePlatformSpecific()
{
}

void MFSystem_HandleEventsPlatformSpecific()
{
	MFCALLSTACK;

	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if(!GetMessage(&msg, NULL, 0, 0)) gQuit=true;
		else
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg);
		}
	}
}

void MFSystem_UpdatePlatformSpecific()
{
}

void MFSystem_DrawPlatformSpecific()
{
}
#endif

MF_API uint64 MFSystem_ReadRTC()
{
	uint64 tickCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&tickCount);
	return tickCount;
}

MF_API uint64 MFSystem_GetRTCFrequency()
{
	uint64 freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	return freq;
}

MF_API const char * MFSystem_GetSystemName()
{
	static char buffer[128];
	DWORD bufferSize = sizeof(buffer);
	GetComputerName(buffer, &bufferSize);
	return buffer;
}

const char * MFSystemPC_GetLastError()
{
	char errorMessage[256];
//	int len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, errorMessage, sizeof(errorMessage), NULL);
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, errorMessage, sizeof(errorMessage), NULL);
	return MFStr(errorMessage);
}

#if _MSC_VER == 1700
	#include <intrin.h>
	#pragma intrinsic (_InterlockedIncrement)
	#pragma intrinsic (_InterlockedDecrement)

	extern "C"
	{
		long _Atomic_fetch_add_4(long volatile *addend)
		{
			return _InterlockedIncrement(addend);
		}

		long _Atomic_fetch_sub_4(long volatile *addend)
		{
			return _InterlockedDecrement(addend);
		}
	}
#endif

#endif // MF_SYSTEM
