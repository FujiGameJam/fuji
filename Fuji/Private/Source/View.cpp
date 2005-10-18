#include "Fuji.h"
#include "Display_Internal.h"
#include "View_Internal.h"
#include "System.h"

#include <math.h>

View View::defaultView;
MFRect View::defaultOrthoRect;

View *gpViewStack = NULL;
View *pCurrentView = NULL;

void View_InitModule()
{
	// allocate view stack
	gpViewStack = (View*)Heap_Alloc(sizeof(View) * gDefaults.view.maxViewsOnStack);

	// set default ortho rect
	View::defaultOrthoRect.x = gDefaults.view.orthoMinX;
	View::defaultOrthoRect.y = gDefaults.view.orthoMinY;
	View::defaultOrthoRect.width = gDefaults.view.orthoMaxX;
	View::defaultOrthoRect.height = gDefaults.view.orthoMaxY;

	// initialise default view
	memset(&View::defaultView, 0, sizeof(View));
	pCurrentView = &View::defaultView;

	pCurrentView->cameraMatrix = MFMatrix::identity;
	pCurrentView->view = MFMatrix::identity;
	pCurrentView->viewProj = MFMatrix::identity;

	View_SetOrtho(&View::defaultOrthoRect);
	View_SetProjection(MFDEGREES(gDefaults.view.defaultFOV), gDefaults.view.defaultAspect, gDefaults.view.defaultNearPlane, gDefaults.view.defaultFarPlane);

	pCurrentView = gpViewStack;
	View_SetDefault();
}

void View_DeinitModule()
{
	/// free view stack
	if(gpViewStack)
	{
		Heap_Free(gpViewStack);
		gpViewStack = NULL;
	}
}

void View_Push()
{
	DBGASSERT(pCurrentView - gpViewStack < (int)gDefaults.view.maxViewsOnStack, "Error: Exceeded maximum views on the stack. Increase 'gDefaults.view.maxViewsOnStack'.");

	// push view pointer foreward
	pCurrentView++;
	*pCurrentView = pCurrentView[-1];
}

void View_Pop()
{
	DBGASSERT(pCurrentView > gpViewStack, "Error: No views on the stack!");

	pCurrentView--;
}


void View_SetDefault()
{
	*pCurrentView = View::defaultView;
}

void View_SetProjection(float _fov, float _aspectRatio, float _nearPlane, float _farPlane)
{
	CALLSTACK;

	if(_fov)
		pCurrentView->fov = _fov;
	if(_nearPlane)
		pCurrentView->nearPlane = _nearPlane;
	if(_farPlane)
		pCurrentView->farPlane = _farPlane;
	if(_aspectRatio)
		pCurrentView->aspectRatio = _aspectRatio;

	pCurrentView->isOrtho = false;
	pCurrentView->viewProjDirty = true;

	// construct and apply perspective projection
	float zn = pCurrentView->nearPlane;
	float zf = pCurrentView->farPlane;

	float a = pCurrentView->fov * 0.5f;

	float h = MFCos(a) / MFSin(a);
	float w = h / pCurrentView->aspectRatio;

	pCurrentView->projection.m[0][0] = w;		pCurrentView->projection.m[0][1] = 0.0f;	pCurrentView->projection.m[0][2] = 0.0f;			pCurrentView->projection.m[0][3] = 0.0f;
	pCurrentView->projection.m[1][0] = 0.0f;	pCurrentView->projection.m[1][1] = h;		pCurrentView->projection.m[1][2] = 0.0f;			pCurrentView->projection.m[1][3] = 0.0f;
	pCurrentView->projection.m[2][0] = 0.0f;	pCurrentView->projection.m[2][1] = 0.0f;	pCurrentView->projection.m[2][2] = zf/(zf-zn);		pCurrentView->projection.m[2][3] = 1.0f;
	pCurrentView->projection.m[3][0] = 0.0f;	pCurrentView->projection.m[3][1] = 0.0f;	pCurrentView->projection.m[3][2] = -zn*zf/(zf-zn);	pCurrentView->projection.m[3][3] = 0.0f;
}

void View_GetProjection(float *pFov, float *pAspectRatio, float *pNearPlane, float *pFarPlane)
{
	*pFov = pCurrentView->fov;
	*pAspectRatio = pCurrentView->aspectRatio;
	*pNearPlane = pCurrentView->nearPlane;
	*pFarPlane = pCurrentView->farPlane;
}

void View_SetOrtho(MFRect *pOrthoRect)
{
	CALLSTACK;

	pCurrentView->viewProjDirty = true;
	pCurrentView->isOrtho = true;

	if(pOrthoRect)
	{
		pCurrentView->orthoRect = *pOrthoRect;
	}

	// construct and apply ortho projection
	float l = pCurrentView->orthoRect.x;
	float r = pCurrentView->orthoRect.x + pCurrentView->orthoRect.width;
	float b = pCurrentView->orthoRect.y + pCurrentView->orthoRect.height;
	float t = pCurrentView->orthoRect.y;
	float zn = 0.0f;
	float zf = 1.0f;

#if 1//!defined(_PSP)
	pCurrentView->projection.m[0][0] = 2.0f/(r-l);	pCurrentView->projection.m[0][1] = 0.0f;		pCurrentView->projection.m[0][2] = 0.0f;			pCurrentView->projection.m[0][3] = 0.0f;
	pCurrentView->projection.m[1][0] = 0.0f;		pCurrentView->projection.m[1][1] = 2.0f/(t-b);	pCurrentView->projection.m[1][2] = 0.0f;			pCurrentView->projection.m[1][3] = 0.0f;
	pCurrentView->projection.m[2][0] = 0.0f;		pCurrentView->projection.m[2][1] = 0.0f;		pCurrentView->projection.m[2][2] = 1.0f/(zf-zn);	pCurrentView->projection.m[2][3] = 0.0f;
	pCurrentView->projection.m[3][0] = (l+r)/(l-r);	pCurrentView->projection.m[3][1] = (t+b)/(b-t);	pCurrentView->projection.m[3][2] = zn/(zn-zf);		pCurrentView->projection.m[3][3] = 1.0f;
#else
	pCurrentView->projection.m[0][0] = 2.0f/(r-l);	pCurrentView->projection.m[0][1] = 0.0f;		pCurrentView->projection.m[0][2] = 0.0f;			pCurrentView->projection.m[0][3] = 0.0f;
	pCurrentView->projection.m[1][0] = 0.0f;		pCurrentView->projection.m[1][1] = 2.0f/(t-b);	pCurrentView->projection.m[1][2] = 0.0f;			pCurrentView->projection.m[1][3] = 0.0f;
	pCurrentView->projection.m[2][0] = 0.0f;		pCurrentView->projection.m[2][1] = 0.0f;		pCurrentView->projection.m[2][2] = 1.0f/(zn-zf);	pCurrentView->projection.m[2][3] = 0.0f;
	pCurrentView->projection.m[3][0] = (l+r)/(l-r);	pCurrentView->projection.m[3][1] = (t+b)/(b-t);	pCurrentView->projection.m[3][2] = zn/(zn-zf);		pCurrentView->projection.m[3][3] = 1.0f;
#endif
}

void View_GetOrtho(MFRect *pOrthoRect)
{
	*pOrthoRect = pCurrentView->orthoRect;
}

bool View_IsOrtho()
{
	return pCurrentView->isOrtho;
}

void View_SetCameraMatrix(const MFMatrix &cameraMatrix)
{
	pCurrentView->cameraMatrix = cameraMatrix;
	pCurrentView->viewDirty = true;
	pCurrentView->viewProjDirty = true;
}

const MFMatrix& View_GetCameraMatrix()
{
	return pCurrentView->cameraMatrix;
}

const MFMatrix& View_GetWorldToViewMatrix()
{
	if(pCurrentView->viewDirty)
	{
		pCurrentView->view.Inverse(pCurrentView->cameraMatrix);
		pCurrentView->viewDirty = false;
	}

	return pCurrentView->view;
}

const MFMatrix& View_GetViewToScreenMatrix()
{
	return pCurrentView->projection;
}

const MFMatrix& View_GetWorldToScreenMatrix()
{
	if(pCurrentView->viewProjDirty)
	{
		pCurrentView->viewProj.Multiply(View_GetWorldToViewMatrix(), pCurrentView->projection);
		pCurrentView->viewProjDirty = false;
	}

	return pCurrentView->viewProj;
}

MFMatrix* View_GetLocalToScreen(const MFMatrix& localToWorld, MFMatrix *pOutput)
{
	pOutput->Multiply(localToWorld, pCurrentView->view);
	pOutput->Multiply(pCurrentView->projection);

	return pOutput;
}

MFMatrix* View_GetLocalToView(const MFMatrix& localToWorld, MFMatrix *pOutput)
{
	pOutput->Multiply(localToWorld, pCurrentView->view);

	return pOutput;
}
