#if !defined(_MATERIAL_H)
#define _MATERIAL_H

#include "Vector4.h"
#include "Texture.h"

enum RederTypeFlags
{
	// Material Flags
	RT_Lit					= 0x00000001,	// L
	RT_Additive				= 0x00000002,	// A
	RT_AlphaBlend			= 0x00000004,	// Al

	RT_Omni					= 0x00000008,	// O
	RT_Mask					= 0x00000010,	// M

	RT_DoubleSided			= 0x00000020,	// Ds

	// Vertex Flags
	RT_Weighted				= 0x00000700,	// W (Up to 8 bone influence)
	RT_Tween				= 0x00000800,	// T

	// Renderer Flags
	RT_PerPixelLighting		= 0x00010000,	// P
	RT_CelShading			= 0x00020000,	// C
	RT_BumpMap				= 0x00040000,	// B
	RT_NormalMap			= 0x00080000,	// N
	RT_DetailTexture		= 0x00100000,	// D
	RT_CubeEnvMap			= 0x00200000,	// Ec
	RT_SphereEnvMap			= 0x00400000,	// E
	RT_ScreenEnvMap			= 0x00800000,	// Es
	RT_DiffuseMap2			= 0x01000000,	// D2
	RT_LightMap				= 0x02000000,	// Lm
	RT_Untextured			= 0x04000000,	// U
};

enum MaterialFlags
{
	// Material Flags
	MF_Lit					= 0x00000001,	// L
	MF_Additive				= 0x00000002,	// A
	MF_AlphaBlend			= 0x00000004,	// Al

	MF_Omni					= 0x00000008,	// O
	MF_Mask					= 0x00000010,	// M

	MF_DoubleSided			= 0x00000020,	// Ds

	// Renderer Flags
	MF_PerPixelLighting		= 0x00010000,	// P
	MF_CelShading			= 0x00020000,	// C
	MF_BumpMap				= 0x00040000,	// B
	MF_NormalMap			= 0x00080000,	// N
	MF_DetailTexture		= 0x00100000,	// D
	MF_CubeEnvMap			= 0x00200000,	// Ec
	MF_SphereEnvMap			= 0x00400000,	// E
	MF_ScreenEnvMap			= 0x00800000,	// Es
	MF_DiffuseMap2			= 0x01000000,	// D2
	MF_LightMap				= 0x02000000,	// Lm
	MF_Untextured			= 0x04000000,	// U
};

class Material
{
	void Use();
	static void UseNone();

	Vector4	diffuse;
	Vector4	ambient;
	Vector4	specular;
	Vector4	illum;
	float	specularPow;

	uint32	maskColour;

	uint32	materialType;

	Texture *pTextures[8];
	uint32	textureCount;

	char name[32];

	uint32 diffuseMapIndex		: 2; // diffuse required to be one of the first 4 map's
	uint32 diffuseMap2Index		: 3;
	uint32 lightMapIndex		: 3;
	uint32 specularMapIndex		: 3;
	uint32 bumpMapIndex			: 3;
	uint32 detailMapIndex		: 3;
	uint32 envMapIndex			: 3;
	uint32 reflectionMapIndex	: 3;
	uint32 normalMapIndex		: 3;

	uint32 cubeMapIndex			: 3; // some what if's
	uint32 displacementMapIndex	: 3;

#if defined(_XBOX)
	DWORD	vertexShader;
	DWORD	pixelShader;
#elif defined(_WINDOWS)
	IDirect3DVertexShader9	*pVertexShader;
	IDirect3DPixelShader9	*pPixelShader;
#endif
};

#endif