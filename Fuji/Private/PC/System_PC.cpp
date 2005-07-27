#include "Common.h"
#include "System_Internal.h"

HINSTANCE apphInstance;
extern int gQuit;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmsShow)
{
	apphInstance = hInstance;

	srand(GetTickCount());

	System_GameLoop();
}

void DoMessageLoop()
{
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

void System_InitModulePlatformSpecific()
{
}

void System_DeinitModulePlatformSpecific()
{
}

void System_UpdatePlatformSpecific()
{
}

void System_DrawPlatformSpecific()
{
}

uint64 RDTSC()
{
	uint64 tickCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&tickCount);
	return tickCount;
}

uint64 GetTSCFrequency()
{
	uint64 freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	return freq;
}
