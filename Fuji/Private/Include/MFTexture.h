/**
 * @file MFTexture.h
 * @brief Declares structures and functions used when accessing the Fuji Heap.
 * @author Manu Evans
 * @defgroup MFTexture Texture Management
 * @{
 */

#if !defined(_MFTEXTURE_H)
#define _MFTEXTURE_H

#include "MFVector.h"

/**
 * @struct MFTexture
 * Represents a Fuji Texture.
 */
struct MFTexture;

/**
 * Texture format.
 * Describes a texture format.
 */
enum MFTextureFormat
{
	TexFmt_Unknown = -1, /**< Unknown texture format */

	TexFmt_A8R8G8B8,	/**< 32bit BGRA format */
	TexFmt_A8B8G8R8,	/**< 32bit RGBA format */
	TexFmt_B8G8R8A8,	/**< 32bit ARGB format */
	TexFmt_R8G8B8A8,	/**< 32bit ABGR format */

	TexFmt_R8G8B8,	/**< 24bit BGR format */
	TexFmt_B8G8R8,	/**< 24bit RGB format */

	TexFmt_A2R10G10B10,	/**< 32bit BGRA format with 10 bits per colour channel */
	TexFmt_A2B10G10R10,	/**< 32bit RGBA format with 10 bits per colour channel */

	TexFmt_A16B16G16R16, /**< 64bit RGBA format with 16 bits per colour channel */

	TexFmt_R5G6B5,		/**< 16bit BGR format with no alpha */
	TexFmt_R6G5B5,		/**< 16bit BGR format with no alpha and 6 bits for red */
	TexFmt_B5G6R5,		/**< 16bit RGB format with no alpha */

	TexFmt_A1R5G5B5,	/**< 16bit BGRA format with 1 bit alpha */
	TexFmt_R5G5B5A1,	/**< 16bit ABGR format with 1 bit alpha */
	TexFmt_A1B5G5R5,	/**< 16bit RGBA format with 1 bit alpha */

	TexFmt_A4R4G4B4,	/**< 16bit BGRA format with 4 bits per colour channel */
	TexFmt_A4B4G4R4,	/**< 16bit RGBA format with 4 bits per colour channel */
	TexFmt_R4G4B4A4,	/**< 16bit ABGR format with 4 bits per colour channel */

	TexFmt_ABGR_F16,	/**< 64bit RGBA floating point format - 16bit floats are described as follows, sign1-exp5-mantissa10 - seeeeemmmmmmmmmm */
	TexFmt_ABGR_F32,	/**< 128bit RGBA floating point format */

	TexFmt_I8,			/**< 8bit paletted format */
	TexFmt_I4,			/**< 4bit paletted format */

	TexFmt_DXT1,		/**< Compressed DXT1 texture */
	TexFmt_DXT2,		/**< Compressed DXT2 texture */
	TexFmt_DXT3,		/**< Compressed DXT3 texture */
	TexFmt_DXT4,		/**< Compressed DXT4 texture */
	TexFmt_DXT5,		/**< Compressed DXT5 texture */

	TexFmt_PSP_DXT1,	/**< Special DXT1 for PSP */
	TexFmt_PSP_DXT3,	/**< Special DXT3 for PSP */
	TexFmt_PSP_DXT5,	/**< Special DXT5 for PSP */

	// platform specific swizzled formats
	TexFmt_XB_A8R8G8B8s,	/**< 32bit BGRA format, swizzled for XBox */
	TexFmt_XB_A8B8G8R8s,	/**< 32bit RGBA format, swizzled for XBox */
	TexFmt_XB_B8G8R8A8s,	/**< 32bit ARGB format, swizzled for XBox */
	TexFmt_XB_R8G8B8A8s,	/**< 32bit ABGR format, swizzled for XBox */

	TexFmt_XB_R5G6B5s,		/**< 16bit BGR format, swizzled for XBox */
	TexFmt_XB_R6G5B5s,		/**< 16bit BGR format, swizzled for XBox */

	TexFmt_XB_A1R5G5B5s,	/**< 16bit BGRA format, swizzled for XBox */
	TexFmt_XB_R5G5B5A1s,	/**< 16bit ABGR format, swizzled for XBox */

	TexFmt_XB_A4R4G4B4s,	/**< 16bit BGRA format, swizzled for XBox */
	TexFmt_XB_R4G4B4A4s,	/**< 16bit ABGR format, swizzled for XBox */

	TexFmt_PSP_A8B8G8R8s,	/**< 32bit RGBA format, swizzled for PSP */
	TexFmt_PSP_B5G6R5s,		/**< 16bit RGB format, swizzled for PSP */
	TexFmt_PSP_A1B5G5R5s,	/**< 16bit RGBA format, swizzled for PSP */
	TexFmt_PSP_A4B4G4R4s,	/**< 16bit RGBA format, swizzled for PSP */

	TexFmt_PSP_I8s,			/**< 8bit paletted format, swizzled for PSP */
	TexFmt_PSP_I4s,			/**< 4bit paletted format, swizzled for PSP */

	TexFmt_PSP_DXT1s,		/**< DXT1, swizzled for PSP */
	TexFmt_PSP_DXT3s,		/**< DXT3, swizzled for PSP */
	TexFmt_PSP_DXT5s,		/**< DXT5, swizzled for PSP */

	TexFmt_Max,				/**< Max texture format */

	TexFmt_SelectNicest = 0x1000,			/**< Select the nicest format. */
	TexFmt_SelectNicest_NoAlpha = 0x1001,	/**< Select the nicest format with no alpha channel. */
	TexFmt_SelectFastest = 0x1002,			/**< Select the fastest format. */
	TexFmt_SelectFastest_Masked = 0x1003,	/**< Select the fastest format requiring only a single bit of alpha. */
	TexFmt_SelectFastest_NoAlpha = 0x1004,	/**< Select the fastest format with no alpha channel. */

	TexFmt_ForceInt = 0x7FFFFFFF			/**< Force texture format to int type */
};

/**
 * Texture flags.
 * Flags to control the way textures are created.
 */
enum TextureFlags
{
	// Internal Flags
	TEX_AlphaMask = 0x3,				/**< Alpha mask. 0 = Opaque, 1 = Full Alpha, 3 = 1bit Alpha */
	TEX_PreMultipliedAlpha = MFBIT(2),	/**< Pre-multiplied alpha */
	TEX_Swizzled = MFBIT(3),			/**< Texture data is swizzled for the platform */
	TEX_RenderTarget = MFBIT(4),		/**< Texture is a render target */

	// User Flags
	TEX_CopyMemory = MFBIT(8)			/**< Takes a copy of the image buffer when calling MFTexture_CreateFromRawData() */
};


// interface functions

/**
 * Create a texture.
 * Creates a texture from the filesystem.
 * @param pName Name of texture to read from the filesystem.
 * @param generateMipChain If true, a mip-chain will be generated for the texture.
 * @return Pointer to an MFTexture structure representing the newly created texture.
 * @remarks If the specified texture has already been created, MFTexture_Create will return a new reference to the already created texture.
 * @see MFTexture_CreateDynamic(), MFTexture_CreateFromRawData(), MFTexture_CreateRenderTarget(), MFTexture_Destroy()
 */
MFTexture* MFTexture_Create(const char *pName, bool generateMipChain = true);

/**
 * Create a dynamic texture.
 * Creates a dynamic texture.
 * @param pName Name of the texture being created.
 * @param width Image width.
 * @param height Image height.
 * @param format Format of the image data. Only formats supported by the platform and TexFmt_A8R8G8B8 can be used.
 * @param flags Texture creation flags.
 * @return Pointer to an MFTexture structure representing the newly created texture.
 * @remarks If the specified texture has already been created, MFTexture_CreateDynamic will fail.
 * @see MFTexture_Create(), MFTexture_CreateFromRawData(), MFTexture_CreateRenderTarget(), MFTexture_Destroy()
 */
MFTexture* MFTexture_CreateDynamic(const char *pName, int width, int height, MFTextureFormat format, uint32 flags = 0);

/**
 * Create a texture from raw data.
 * Creates a texture from a raw data buffer.
 * @param pName Name of the texture being created.
 * @param pData Pointer to a buffer containing the image data
 * @param width Image width.
 * @param height Image height.
 * @param format Format of the image data being read. Only formats supported by the platform and TexFmt_A8R8G8B8 can be used.
 * @param flags Texture creation flags.
 * @param generateMipChain If true, a mip-chain will be generated for the texture.
 * @param pPalette Pointer to palette data. Use NULL for non-paletted image formats.
 * @return Pointer to an MFTexture structure representing the newly created texture.
 * @remarks If TexFmt_A8R8G8B8 is used, and it is not supported by the platform natively, a copy of the image is taken and the data is swizzled to the best available 32bit format on the target platform. Use MFTexture_GetPlatformAvailability() or MFTexture_IsAvailableOnPlatform() to determine what formats are supported on a particular platform.
 * @see MFTexture_Create(), MFTexture_Destroy(), MFTexture_GetPlatformAvailability(), MFTexture_IsAvailableOnPlatform()
 */
MFTexture* MFTexture_CreateFromRawData(const char *pName, void *pData, int width, int height, MFTextureFormat format, uint32 flags = 0, bool generateMipChain = true, uint32 *pPalette = 0);

/**
 * Creates a render target texture.
 * Creates a render target texture.
 * @param pName Name of the texture being created.
 * @param width Width of render target.
 * @param height Height of render target.
 * @return Pointer to an MFTexture structure representing the newly created render target texture.
 * @see MFTexture_Create(), MFTexture_Destroy()
 */
MFTexture* MFTexture_CreateRenderTarget(const char *pName, int width, int height, MFTextureFormat targetFormat = TexFmt_SelectNicest);

/**
 * Destroys a Texture.
 * Release a reference to an MFTexture and destroy when the reference reaches 0.
 * @param pTexture Texture instance to be destroyed.
 * @return Returns the new reference count of the texture. If the returned reference count is 0, the texture is destroyed.
 * @see MFTexture_Create()
 */
int MFTexture_Destroy(MFTexture *pTexture);

/**
 * Find an existing texture.
 * Finds an existing instance of the specified texture and returns a pointer. If the texture is not found, NULL is returned.
 * Note: The reference count is NOT incremented by MFTexture_FindTexture().
 * @param pName Name of texture to find.
 * @return Returns a pointer to the texture if it is found, otherwise NULL is returned.
 * @remarks MFTexture_Create does NOT increase the reference count of the texture so it is not required to destroy any texture returned by MFTexture_FindTexture().
 * @see MFTexture_Create()
 */
MFTexture* MFTexture_FindTexture(const char *pName);

/**
 * Create a blank plain coloured texture.
 * Create a new texture that is a solid colour.
 * @param pName Name for the texture being created.
 * @param colour Colour to fill the texture when it is created.
 * @return Returns a pointer to a newly created blank texture.
 * @see MFTexture_Create()
 */
MFTexture* MFTexture_CreateBlank(const char *pName, const MFVector &colour);

/**
 * Get a string representing the texture format.
 * Gets a human readable string representing the texture format.
 * @param format Texture format to get the name of.
 * @return Pointer to a string representing the texture format.
 * @see MFTexture_GetPlatformAvailability(), MFTexture_GetBitsPerPixel()
 */
const char * const MFTexture_GetFormatString(int format);

/**
 * Gets all platforms that support the specified texture format in hardware.
 * Gets a variable representing which platforms support the specified texture format in hardware.
 * @param format Format to test for hardware support.
 * @return Result is a bitfield where each bit represents hardware support for a specific platform. Platform support can be tested, for example, using: ( result & MFBIT(FP_PC) ) != 0.
 * @see MFTexture_GetFormatString(), MFTexture_GetBitsPerPixel()
 */
uint32 MFTexture_GetPlatformAvailability(int format);

/**
 * Tests to see if a texture format is available on the current platform.
 * Tests if a texture format is supported in hardware on the current platform.
 * @param format Texture format to be tested.
 * @return Returns true if specified format is supported in hardware.
 * @see MFTexture_GetPlatformAvailability()
 */
bool MFTexture_IsAvailable(int format);

/**
 * Tests to see if a texture format is available on a specified platform.
 * Tests if a texture format is supported in hardware on a specified platform.
 * @param format Texture format to be tested.
 * @param platform Platform to test for hardware support.
 * @return Returns true if specified format is supported in hardware.
 * @see MFTexture_GetPlatformAvailability()
 */
bool MFTexture_IsAvailableOnPlatform(int format, int platform);

/**
 * Get the average number of bits per pixel for a specified format.
 * Get the average number of bits per pixel for the specified format.
 * @param format Name for the texture being created.
 * @return Returns the number of bits per pixel for the specified format. If a compressed format is specified, the average number of bits per pixel is returned.
 * @see MFTexture_GetPlatformAvailability(), MFTexture_GetFormatString()
 */
int MFTexture_GetBitsPerPixel(int format);

void MFTexture_GetTextureDimensions(MFTexture *pTexture, int *pWidth, int *pHeight);

#endif // _TEXTURE_H

/** @} */
