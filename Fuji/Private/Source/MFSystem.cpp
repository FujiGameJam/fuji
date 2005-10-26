#include "Fuji.h"
#include "MFSystem_Internal.h"
#include "Display_Internal.h"
#include "MFTexture_Internal.h"
#include "MFMaterial_Internal.h"
#include "MFModel_Internal.h"
#include "MFInput_Internal.h"
#include "MFView_Internal.h"
#include "MFFileSystem_Internal.h"
#include "Font.h"
#include "Primitive.h"
#include "DebugMenu.h"
#include "Timer.h"
#include "Font.h"
#include "Renderer_Internal.h"
#include "Sound.h"
#include "MFSockets_Internal.h"

// externs
void MFSystem_HandleEventsPlatformSpecific();

// extern to platform
extern MFPlatform gCurrentPlatform;

// local variables
MFDefaults gDefaults = 
{
	// HeapDefaults
	{
		4*1024*1024,	// dynamicHeapSize
		4*1024*1024,	// staticHeapSize
		0,				// dynamicHeapCount
		0,				// staticHeapCount
		256,			// maxResources
		2048			// maxStaticMarkers
	},

	// DisplayDefaults
	{
#if !defined(_PSP)
		640,			// displayWidth
		480				// displayHeight
#else
		480,			// displayWidth
		272				// displayHeight
#endif
	},

	// ViewDefaults
	{
		16,				// maxViewsOnStack
		60.0f,			// defaultFOV;
		4.0f/3.0f,		// defaultAspect;
		0.1f,			// defaultNearPlane;
		1000.0f,		// defaultFarPlane;
		0.0f,			// orthoMinX
		0.0f,			// orthoMinY
		640.0f,			// orthoMaxX
		480.0f			// orthoMaxY
	},

	// TextureDefaults
	{
		256				// maxTextures
	},

	// MaterialDefaults
	{
		32,				// maxMaterialTypes
		32,				// maxMaterialDefs
		2048			// maxMaterials
	},

	// ModelDefaults
	{
		256				// maxModels
	},

	// FileSystemDefaults
	{
		128,			// maxOpenFiles
		16,				// maxFileSystems
		16,				// maxFileSystemStackSize
	},

	// SoundDefaults
	{
		4				// maxMusicTracks
	},

	// InputDefaults
	{
		true,			// allowMultipleMice
		true,			// mouseZeroIsSystemMouse
		true			// systemMouseUseWindowsCursor
	},

	// MiscellaneousDefaults
	{
		true			// enableUSBOnStartup
	}
};

bool gDrawSystemInfo = true;
int gQuit = 0;
int gRestart = 1;
uint32 gFrameCount = 0;
float gSystemTimeDelta;

MenuItemStatic quitOption;
MenuItemStatic restartOption;

void QuitCallback(MenuObject *pMenu, void *pData)
{
	gQuit = 1;
}

void RestartCallback(MenuObject *pMenu, void *pData)
{
	gQuit = 1;
	gRestart = 1;
}

void MFSystem_Init()
{
	CALLSTACK;

	CrcInit();

	Heap_InitModule();

	DebugMenu_InitModule();
	Callstack_InitModule();

	MFSystem_InitModulePlatformSpecific();

	Timer_InitModule();
	gSystemTimer.Init(NULL);
	gSystemTimeDelta = gSystemTimer.TimeDeltaF();

	MFSockets_InitModule();

	MFFileSystem_InitModule();

	MFView_InitModule();
	MFDisplay_InitModule();
	MFInput_InitModule();

	Sound_InitModule();

	Renderer_InitModule();
	MFTexture_InitModule();
	MFMaterial_InitModule();

	MFModel_InitModule();

	Primitive_InitModule();
	Font_InitModule();

	DebugMenu_AddItem("Restart", "Fuji Options", &restartOption, RestartCallback, NULL);
	DebugMenu_AddItem("Quit", "Fuji Options", &quitOption, QuitCallback, NULL);

	Heap_MarkHeap();
}

void MFSystem_Deinit()
{
	CALLSTACK;

	Heap_ReleaseMark();

	Font_DeinitModule();
	Primitive_DeinitModule();

	MFModel_DeinitModule();

	MFMaterial_DeinitModule();
	MFTexture_DeinitModule();
	Renderer_DeinitModule();

	Sound_DeinitModule();

	MFInput_DeinitModule();
	MFDisplay_DeinitModule();
	MFView_DeinitModule();

	MFFileSystem_DeinitModule();

	MFSockets_DeinitModule();

	Timer_DeinitModule();

	MFSystem_DeinitModulePlatformSpecific();

	Callstack_DeinitModule();
	DebugMenu_DeinitModule();

	Heap_DeinitModule();
}

void MFSystem_Update()
{
	CALLSTACKc;

	MFSystem_UpdatePlatformSpecific();

	MFInput_Update();

#if defined(_XBOX)
	if(MFInput_Read(Button_XB_Start, IDD_Gamepad) && MFInput_Read(Button_XB_White, IDD_Gamepad) && MFInput_Read(Button_XB_LTrig, IDD_Gamepad) MF&& Input_Read(Button_XB_RTrig, IDD_Gamepad))
		RestartCallback(NULL, NULL);
#elif defined(_PSP)
	if(MFInput_Read(Button_DLeft, IDD_Gamepad) && MFInput_Read(Button_PP_Circle, IDD_Gamepad) && MFInput_Read(Button_PP_L, IDD_Gamepad) && MFInput_Read(Button_PP_R, IDD_Gamepad))
		RestartCallback(NULL, NULL);
#else//if defined(_WINDOWS)
	if(MFInput_Read(Button_P2_Start, IDD_Gamepad) && MFInput_Read(Button_P2_Select, IDD_Gamepad) && MFInput_Read(Button_P2_L1, IDD_Gamepad) && MFInput_Read(Button_P2_R1, IDD_Gamepad) && MFInput_Read(Button_P2_L2, IDD_Gamepad) && MFInput_Read(Button_P2_R2, IDD_Gamepad))
		RestartCallback(NULL, NULL);
#endif

#if defined(_PSP)
	if(MFInput_Read(Button_DLeft, IDD_Gamepad) && MFInput_Read(Button_PP_L, IDD_Gamepad) && MFInput_WasPressed(Button_PP_Start, IDD_Gamepad))
		gDrawSystemInfo = !gDrawSystemInfo;
#else
	if(MFInput_Read(Button_P2_L1, IDD_Gamepad) && MFInput_Read(Button_P2_L2, IDD_Gamepad) && MFInput_WasPressed(Button_P2_LThumb, IDD_Gamepad))
		gDrawSystemInfo = !gDrawSystemInfo;
#endif

#if !defined(_RETAIL)
	DebugMenu_Update();
#endif

	MFMaterial_Update();

	Sound_Update();
}

void MFSystem_PostUpdate()
{
	CALLSTACK;

}

void MFSystem_Draw()
{
	CALLSTACKc;

#if !defined(_RETAIL)
	MFView_Push();
	MFView_SetDefault();

	MFRect rect;

	rect.x = 0.0f;
	rect.y = 0.0f;
	rect.width = (float)gDefaults.display.displayWidth;
	rect.height = (float)gDefaults.display.displayHeight;

	MFView_SetOrtho(&rect);

	// should be the first thing rendered so we only display game vertices
	DrawMFPrimitiveStats();

	Sound_Draw();

	Callstack_DrawProfile();

	if(gDrawSystemInfo)
	{
		//FPS Display
#if defined(_PSP)
		float x = gDefaults.display.displayWidth-100.0f;
		float y = 10.0f;
#else
		float x = gDefaults.display.displayWidth-140.0f;
		float y = 30.0f;
#endif

		Font_DrawTextf(gpDebugFont, x, y, 0, 20.0f, MakeVector(1,1,0,1), "FPS: %.2f", MFSystem_GetFPS());

		float rate = (float)gSystemTimer.GetRate();
		if(rate != 1.0f)
			Font_DrawTextf(gpDebugFont, 80.0f, gDefaults.display.displayHeight-50.0f, 0, 20.0f, MakeVector(1,0,0,1), "Rate: %s", STR(rate == 0.0f ? "Paused" : "%.2f", rate));

		MFMaterial_SetMaterial(MFMaterial_GetStockMaterial(Mat_SysLogoSmall));
		const float iconSize = 55.0f;

		float yOffset = gDefaults.display.displayHeight-70.0f;

		MFPrimitive(PT_TriStrip);
		MFBegin(4);
		MFSetColour(1,1,1,0.5f);
		MFSetTexCoord1(0,0);
		MFSetPosition(15, yOffset, 0);
		MFSetTexCoord1(1,0);
		MFSetPosition(15+iconSize, yOffset, 0);
		MFSetTexCoord1(0,1);
		MFSetPosition(15, yOffset+iconSize, 0);
		MFSetTexCoord1(1,1);
		MFSetPosition(15+iconSize, yOffset+iconSize, 0);
		MFEnd();

		MFSystem_DrawPlatformSpecific();
	}

	DebugMenu_Draw();

	MFView_Pop();
#endif
}

int MFSystem_GameLoop()
{
	CALLSTACK;

	// allow's game to set defaults and what not
	// before the system begins initialisation
	Game_InitSystem();

	// initialise the system and create displays etc..
	MFSystem_Init();

	while(gRestart)
	{
		gRestart = 0;
		gQuit = 0;

		Game_Init();

		while(!gQuit)
		{
			MFSystem_HandleEventsPlatformSpecific();

			Callstack_BeginFrame();
			MFSystem_UpdateTimeDelta();
			gFrameCount++;

			MFSystem_Update();
			if(!DebugMenu_IsEnabled())
				Game_Update();
			MFSystem_PostUpdate();

			MFDisplay_BeginFrame();

			Game_Draw();
			MFSystem_Draw();

			Callstack_EndFrame();
			MFDisplay_EndFrame();
		}

		Game_Deinit();
	}

	MFSystem_Deinit();

	return gQuit;
}

void MFSystem_UpdateTimeDelta()
{
	CALLSTACK;

	gSystemTimer.Update();
	gSystemTimeDelta = gSystemTimer.TimeDeltaF();
}

float MFSystem_GetFPS()
{
	return gSystemTimer.GetFPS();
}

MFPlatform MFSystem_GetCurrentPlatform()
{
	return gCurrentPlatform;
}
