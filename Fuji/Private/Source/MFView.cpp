#include "Fuji.h"
#include "MFHeap.h"
#include "Display_Internal.h"
#include "MFView_Internal.h"
#include "MFSystem.h"

#include <math.h>

MFView MFView::defaultView;
MFRect MFView::defaultOrthoRect;

MFView *gpViewStack = NULL;
MFView *pCurrentView = NULL;

void MFView_InitModule()
{
	// allocate view stack
	gpViewStack = (MFView*)MFHeap_Alloc(sizeof(MFView) * gDefaults.view.maxViewsOnStack);

	// set default ortho rect
	MFView::defaultOrthoRect.x = gDefaults.view.orthoMinX;
	MFView::defaultOrthoRect.y = gDefaults.view.orthoMinY;
	MFView::defaultOrthoRect.width = gDefaults.view.orthoMaxX;
	MFView::defaultOrthoRect.height = gDefaults.view.orthoMaxY;

	// initialise default view
	MFZeroMemory(&MFView::defaultView, sizeof(MFView));
	pCurrentView = &MFView::defaultView;

	pCurrentView->cameraMatrix = MFMatrix::identity;
	pCurrentView->view = MFMatrix::identity;
	pCurrentView->viewProj = MFMatrix::identity;

	MFView_SetOrtho(&MFView::defaultOrthoRect);
	MFView_ConfigureProjection(MFDEGREES(gDefaults.view.defaultFOV), gDefaults.view.defaultNearPlane, gDefaults.view.defaultFarPlane);
	MFView_SetAspectRatio(gDefaults.view.defaultAspect);
	MFView_SetProjection();

	pCurrentView = gpViewStack;
	MFView_SetDefault();
}

void MFView_DeinitModule()
{
	/// free view stack
	if(gpViewStack)
	{
		MFHeap_Free(gpViewStack);
		gpViewStack = NULL;
	}
}

void MFView_Push()
{
	MFDebug_Assert(pCurrentView - gpViewStack < (int)gDefaults.view.maxViewsOnStack, "Error: Exceeded maximum views on the stack. Increase 'gDefaults.view.maxViewsOnStack'.");

	// push view pointer foreward
	pCurrentView++;
	*pCurrentView = pCurrentView[-1];
}

void MFView_Pop()
{
	MFDebug_Assert(pCurrentView > gpViewStack, "Error: No views on the stack!");

	pCurrentView--;
}


void MFView_SetDefault()
{
	*pCurrentView = MFView::defaultView;
}

void MFView_ConfigureProjection(float fieldOfView, float nearPlane, float farPlane)
{
	pCurrentView->fov = fieldOfView;
	pCurrentView->nearPlane = nearPlane;
	pCurrentView->farPlane = farPlane;

	if(!pCurrentView->isOrtho)
	{
		pCurrentView->viewProjDirty = true;
		pCurrentView->projDirty = true;
	}
}

void MFView_SetAspectRatio(float aspectRatio)
{
	pCurrentView->aspectRatio = aspectRatio;

	if(!pCurrentView->isOrtho)
	{
		pCurrentView->viewProjDirty = true;
		pCurrentView->projDirty = true;
	}
}

void MFView_SetProjection()
{
	MFCALLSTACK;

	if(pCurrentView->isOrtho)
	{
		pCurrentView->isOrtho = false;
		pCurrentView->viewProjDirty = true;
		pCurrentView->projDirty = true;
	}
}

void MFView_SetOrtho(MFRect *pOrthoRect)
{
	MFCALLSTACK;

	if(!pCurrentView->isOrtho || pOrthoRect)
	{
		pCurrentView->isOrtho = true;

		pCurrentView->viewProjDirty = true;
		pCurrentView->projDirty = true;

		if(pOrthoRect)
		{
			pCurrentView->orthoRect = *pOrthoRect;
		}
	}
}

void MFView_GetOrthoRect(MFRect *pOrthoRect)
{
	*pOrthoRect = pCurrentView->orthoRect;
}

bool MFView_IsOrtho()
{
	return pCurrentView->isOrtho;
}

void MFView_SetCameraMatrix(const MFMatrix &cameraMatrix)
{
	pCurrentView->cameraMatrix = cameraMatrix;

	pCurrentView->viewDirty = true;
	pCurrentView->viewProjDirty = true;
}

const MFMatrix& MFView_GetCameraMatrix()
{
	return pCurrentView->cameraMatrix;
}

const MFMatrix& MFView_GetWorldToViewMatrix()
{
	if(pCurrentView->viewDirty)
	{
		pCurrentView->view.Inverse(pCurrentView->cameraMatrix);
		pCurrentView->viewDirty = false;
	}

	return pCurrentView->view;
}

void MFViewInternal_ProjectionMatrix(MFMatrix *pMat)
{
	// construct perspective projection
	float zn = pCurrentView->nearPlane;
	float zf = pCurrentView->farPlane;

	float a = pCurrentView->fov * 0.5f;

	float h = MFCos(a) / MFSin(a);
	float w = h / pCurrentView->aspectRatio;

	float zd = zf-zn;
	float zs = zf/zd;

#if defined(_OPENGL_CLIP_SPACE)
	pMat->m[0] = w;		pMat->m[1] = 0.0f;	pMat->m[2] = 0.0f;				pMat->m[3] = 0.0f;
	pMat->m[4] = 0.0f;	pMat->m[5] = h;		pMat->m[6] = 0.0f;				pMat->m[7] = 0.0f;
	pMat->m[8] = 0.0f;	pMat->m[9] = 0.0f;	pMat->m[10] = 2.0f*zs;			pMat->m[11] = 1.0f;
	pMat->m[12] = 0.0f;	pMat->m[13] = 0.0f;	pMat->m[14] = -2.0f*zn*zs-zf;	pMat->m[15] = 0.0f;
#else
	pMat->m[0] = w;		pMat->m[1] = 0.0f;	pMat->m[2] = 0.0f;		pMat->m[3] = 0.0f;
	pMat->m[4] = 0.0f;	pMat->m[5] = h;		pMat->m[6] = 0.0f;		pMat->m[7] = 0.0f;
	pMat->m[8] = 0.0f;	pMat->m[9] = 0.0f;	pMat->m[10] = zs;		pMat->m[11] = 1.0f;
	pMat->m[12] = 0.0f;	pMat->m[13] = 0.0f;	pMat->m[14] = -zn*zs;	pMat->m[15] = 0.0f;
#endif
}

void MFViewInternal_OrthoMatrix(MFMatrix *pMat)
{
	// construct ortho projection
	float l = pCurrentView->orthoRect.x;
	float r = pCurrentView->orthoRect.x + pCurrentView->orthoRect.width;
	float b = pCurrentView->orthoRect.y + pCurrentView->orthoRect.height;
	float t = pCurrentView->orthoRect.y;
	float zn = 0.0f;
	float zf = 1.0f;

#if defined(_OPENGL_CLIP_SPACE)
	pMat->m[0] = 2.0f/(r-l);	pMat->m[1] = 0.0f;				pMat->m[2] = 0.0f;				pMat->m[3] = 0.0f;
	pMat->m[4] = 0.0f;			pMat->m[1][1] = 2.0f/(t-b);		pMat->m[1][2] = 0.0f;			pMat->m[1][3] = 0.0f;
	pMat->m[8] = 0.0f;			pMat->m[2][1] = 0.0f;			pMat->m[2][2] = 1.0f/(zf-zn);	pMat->m[2][3] = 0.0f;
	pMat->m[12] = (l+r)/(l-r);	pMat->m[3][1] = (t+b)/(b-t);	pMat->m[3][2] = zn/(zn-zf);		pMat->m[3][3] = 1.0f;
#else
	pMat->m[0] = 2.0f/(r-l);	pMat->m[1] = 0.0f;			pMat->m[2] = 0.0f;			pMat->m[3] = 0.0f;
	pMat->m[4] = 0.0f;			pMat->m[5] = 2.0f/(t-b);	pMat->m[6] = 0.0f;			pMat->m[7] = 0.0f;
	pMat->m[8] = 0.0f;			pMat->m[9] = 0.0f;			pMat->m[10] = 1.0f/(zf-zn);	pMat->m[11] = 0.0f;
	pMat->m[12] = (l+r)/(l-r);	pMat->m[13] = (t+b)/(b-t);	pMat->m[14] = zn/(zn-zf);	pMat->m[15] = 1.0f;
#endif
}

const MFMatrix& MFView_GetViewToScreenMatrix()
{
	if(pCurrentView->projDirty)
	{
		if(pCurrentView->isOrtho)
		{
			MFViewInternal_OrthoMatrix(&pCurrentView->projection);
		}
		else
		{
			MFViewInternal_ProjectionMatrix(&pCurrentView->projection);
		}

		pCurrentView->projDirty = false;
	}

	return pCurrentView->projection;
}

const MFMatrix& MFView_GetWorldToScreenMatrix()
{
	if(pCurrentView->viewProjDirty)
	{
		pCurrentView->viewProj.Multiply4x4(MFView_GetWorldToViewMatrix(), MFView_GetViewToScreenMatrix());
		pCurrentView->viewProjDirty = false;
	}

	return pCurrentView->viewProj;
}

MFMatrix* MFView_GetLocalToScreen(const MFMatrix& localToWorld, MFMatrix *pOutput)
{
	pOutput->Multiply(localToWorld, MFView_GetWorldToScreenMatrix());

	return pOutput;
}

MFMatrix* MFView_GetLocalToView(const MFMatrix& localToWorld, MFMatrix *pOutput)
{
	pOutput->Multiply(localToWorld, MFView_GetWorldToViewMatrix());

	return pOutput;
}

void MFView_TransformPoint3DTo2D(const MFVector& point, MFVector *pResult)
{
	MFMatrix proj, viewProj;
	MFViewInternal_ProjectionMatrix(&proj);

	viewProj.Multiply4x4(MFView_GetWorldToViewMatrix(), proj);

	MFVector transformed;
	transformed = ApplyMatrix(point, viewProj);
	transformed *= MFRcp(transformed.w);

	transformed.x += 1.0f;
	transformed.y = -transformed.y + 1.0f;

	*pResult = transformed * MakeVector(pCurrentView->orthoRect.width*0.5f, pCurrentView->orthoRect.height*0.5f) + MakeVector(pCurrentView->orthoRect.x, pCurrentView->orthoRect.y);
}
