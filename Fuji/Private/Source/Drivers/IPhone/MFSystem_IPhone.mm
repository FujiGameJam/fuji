#include "Fuji.h"

#if MF_SYSTEM == MF_DRIVER_IPHONE

#import <UIKit/UIKit.h>

#include "MFSystem.h"

#include <time.h>

MFPlatform gCurrentPlatform = FP_IPhone;

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
	return (uint64)clock();
}

uint64 MFSystem_GetRTCFrequency()
{
	return CLOCKS_PER_SEC;
}

const char * MFSystem_GetSystemName()
{
	return "iPhone";
}

#endif
