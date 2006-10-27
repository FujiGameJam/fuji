/**
 * @file MFAnimation.h
 * @brief A set of functions for managing mesh animation.
 * @author Manu Evans
 * @defgroup MFAnimation Animation Management
 * @{
 */

#if !defined(_MFANIMATION_H)
#define _MFANIMATION_H

struct MFModel;
class MFMatrix;

/**
 * @struct MFAnimation
 * Represents a Fuji animation.
 */
struct MFAnimation;

/**
 * Create an animation from the filesystem.
 * Creates an animation from the filesystem.
 * @param pFilename Filename of the animation to load.
 * @param pModel MFModel instance the animation will be bound to.
 * @return Returns a new instance of the specified animation.
 * @see MFAnimation_Destroy()
 * @see MFAnimation_CalculateMatrices()
 */
MFAnimation* MFAnimation_Create(const char *pFilename, MFModel *pModel);

/**
 * Destroy an animation.
 * Destroys an animation instance.
 * @param pAnimation Animation instance to be destroyed.
 * @return None.
 * @see MFAnimation_Create()
 */
void MFAnimation_Destroy(MFAnimation *pAnimation);

/**
 * Calculate the animation matrices.
 * Calculates the animation matrices for the current frame.
 * @param pAnimation Animation instance.
 * @param pLocalToWorld Optional pointer to a LocalToWorld matrix that will be multiplied into the animation matrices.
 * @return Returns a pointer to the array of calculated animation matrices.
 * @see MFAnimation_Create()
 */
MFMatrix *MFAnimation_CalculateMatrices(MFAnimation *pAnimation, MFMatrix *pLocalToWorld);

/**
 * Get the animations frame range.
 * Gets the animations valid frame range.
 * @param pAnimation Animation instance.
 * @param pStartTime Pointer to a float that will receive the start time. Can be NULL.
 * @param pEndTime Pointer to a float that will receive the end time. Can be NULL.
 * @return None.
 * @see MFAnimation_SetFrame()
 */
void MFAnimation_GetFrameRange(MFAnimation *pAnimation, float *pStartTime, float *pEndTime);

/**
 * Set the current frame.
 * Sets the current frame time.
 * @param pAnimation Animation instance.
 * @param frameTime Frame time to be set as the current frame.
 * @return None.
 * @see MFAnimation_GetFrameRange()
 */
void MFAnimation_SetFrame(MFAnimation *pAnimation, float frameTime);

#endif

/** @} */
