#if !defined(_MFRENDERER_D3D11_H)
#define _MFRENDERER_D3D11_H

#include <D3D11.h>

//---------------------------------------------------------------------------------------------------------------------
void MFRenderer_D3D11_SetWorldToScreenMatrix(const MFMatrix &worldToScreen);
//---------------------------------------------------------------------------------------------------------------------
void MFRenderer_D3D11_SetDebugName(ID3D11DeviceChild* pResource, const char* pName);
//---------------------------------------------------------------------------------------------------------------------

#endif