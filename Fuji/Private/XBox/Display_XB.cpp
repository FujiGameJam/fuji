#include "Common.h"
#include "Display_Internal.h"
#include "DebugMenu_Internal.h"
#include "View_Internal.h"

IDirect3D8 *d3d8;
IDirect3DDevice8 *pd3dDevice;

bool isortho = false;
float fieldOfView;

extern MFVector gClearColour;

int Display_CreateDisplay(int width, int height, int bpp, int rate, bool vsync, bool triplebuffer, bool wide, bool progressive)
{
	CALLSTACK;

	D3DPRESENT_PARAMETERS presentparams;
	HRESULT hr;

	display.width = width;
	display.height = height;
	display.progressive = progressive;
	display.refreshRate = rate;
	display.wide = wide;

	d3d8 = Direct3DCreate8(D3D_SDK_VERSION);
	if(!d3d8) return 1;

	memset(&presentparams, 0, sizeof(D3DPRESENT_PARAMETERS));
	presentparams.BackBufferWidth = width;
	presentparams.BackBufferHeight = height;
	presentparams.BackBufferFormat = (bpp == 32) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5;
	presentparams.BackBufferCount = triplebuffer ? 2 : 1;
	presentparams.MultiSampleType = D3DMULTISAMPLE_NONE;
	presentparams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentparams.EnableAutoDepthStencil = TRUE;
	presentparams.AutoDepthStencilFormat = D3DFMT_D24S8;
	presentparams.FullScreen_RefreshRateInHz = rate; 
	presentparams.FullScreen_PresentationInterval = vsync ? D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE : D3DPRESENT_INTERVAL_IMMEDIATE;
	presentparams.Flags = (wide ? D3DPRESENTFLAG_WIDESCREEN : NULL) | (progressive ? D3DPRESENTFLAG_PROGRESSIVE : NULL);

	hr = d3d8->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentparams, &pd3dDevice);
	if(hr != D3D_OK) return 2;

	return 0;
}

void Display_DestroyDisplay()
{
	CALLSTACK;

	pd3dDevice->Release();
	d3d8->Release();
}

void Display_BeginFrame()
{
	CALLSTACK;

	pd3dDevice->BeginScene();

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
	pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void Display_EndFrame()
{
	CALLSTACKc;

	pd3dDevice->EndScene();
	pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

void Display_SetClearColour(float r, float g, float b, float a)
{
	gClearColour.x = r;
	gClearColour.y = g;
	gClearColour.z = b;
	gClearColour.w = a;
}

void Display_ClearScreen(uint32 flags)
{
	CALLSTACKc;

	pd3dDevice->Clear(0, NULL, (CS_Colour ? D3DCLEAR_TARGET : NULL)|(CS_ZBuffer ? D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL : NULL), gClearColour.ToPackedColour(), 1.0f, 0);
}

void SetViewport(float x, float y, float width, float height)
{
	CALLSTACK;

	D3DVIEWPORT8 vp;
	vp.X = (DWORD)((x / 640.0f) * (float)display.width);
	vp.Y = (DWORD)((y / 480.0f) * (float)display.height);
	vp.Width = (DWORD)((width / 640.0f) * (float)display.width);
	vp.Height = (DWORD)((height / 480.0f) * (float)display.height);
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	pd3dDevice->SetViewport(&vp);
}

void ResetViewport()
{
	CALLSTACK;

	D3DVIEWPORT8 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = display.width;
	vp.Height = display.height;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	pd3dDevice->SetViewport(&vp);
}
