#include "Common.h"
#include "Display.h"

IDirect3D8 *d3d8;
IDirect3DDevice8 *pd3dDevice;

bool isortho = false;
float fieldOfView;

void Display_InitModule()
{
	int error;

	// create the display
	error = CreateDisplay(640, 480, 32, 60, true, false, false, false);
	if(error) return;
}

void Display_DeinitModule()
{
	DestroyDisplay();
}

int CreateDisplay(int width, int height, int bpp, int rate, bool vsync, bool triplebuffer, bool wide, bool progressive)
{
	D3DPRESENT_PARAMETERS presentparams;
	HRESULT hr;
	
	display.width = width;
	display.height = height;
	display.progressive = progressive;
	display.rate = rate;
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

void DestroyDisplay()
{
	pd3dDevice->Release();
	d3d8->Release();
}

void Display_BeginFrame()
{
	pd3dDevice->BeginScene();
}

void Display_EndFrame()
{
	pd3dDevice->EndScene();
	pd3dDevice->Present(NULL, NULL, NULL, NULL);

}

void ClearScreen()
{
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000030, 1.0f, 0);
}

void SetProjection(float fov)
{
	D3DXMATRIX proj;

	fieldOfView = fov;

	// construct and apply perspective projection
	D3DXMatrixPerspectiveFovLH(&proj, fov, (display.wide ? WIDE_ASPECT : STANDARD_ASPECT), 0.1f, 1000.0f);
	pd3dDevice->SetTransform(D3DTS_PROJECTION, &proj);
}

bool SetOrtho(bool enable, float width, float height)
{
	D3DXMATRIX proj;

	bool t = isortho;
	isortho = enable;

	if(enable)
	{
		float extend = 0.0f;

		// correct for widescreen
		if(display.wide) extend = (((width/1.333333333f)*1.77777777778f)-width)/2.0f;

		// construct and apply ortho projection
		D3DXMatrixOrthoOffCenterLH(&proj, -extend, width + extend, height, 0, 0.0f, 1000.0f);
		pd3dDevice->SetTransform(D3DTS_PROJECTION, &proj);
	}
	else
	{
		SetProjection(fieldOfView);
	}

	return t;
}
