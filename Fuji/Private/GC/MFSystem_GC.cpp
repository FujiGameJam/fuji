#include "Fuji.h"
#include "MFSystem.h"

MFPlatform gCurrentPlatform = FP_GC;

int main()
{

	return 0;
}

void MFSystem_InitModulePlatformSpecific()
{
}

void MFSystem_DeinitModulePlatformSpecific()
{
}

void MFSystem_HandleEventsPlatformSpecific()
{
}

void MFSystem_UpdatePlatformSpecific()
{
}

void MFSystem_DrawPlatformSpecific()
{
}

uint64 MFSystem_ReadRTC()
{
	return 0;
}

uint64 MFSystem_GetRTCFrequency()
{
	return 1000;
}

const char * MFSystem_GetSystemName()
{
	return "Gamecube";
}
