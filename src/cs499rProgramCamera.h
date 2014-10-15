
#ifndef _CLH_CS499R_PROGRAM_CAMERA
#define _CLH_CS499R_PROGRAM_CAMERA

#include "cs499rProgramSampleContext.h"


// ----------------------------------------------------------------------------- FUNCTIONS

inline
void
camera_first_ray(
    sample_context_t * const sampleCx,
    __global common_render_context_t const * shotCx,
    uint32x2_t const pixelCoord,
    uint32x2_t const pixelSubpixelCoord
)
{
    float32x2_t const subpixelCoord = (float32x2_t)(pixelCoord.x, pixelCoord.y) +
            (1.0f + 2.0f * (float32x2_t)(pixelSubpixelCoord.x, pixelSubpixelCoord.y)) *
            (0.5f / (float32_t) shotCx->render.subpixelPerPixelBorder);

    float32x2_t areaCoord;
    areaCoord.x = subpixelCoord.x / ((float32_t)shotCx->render.resolution.x);
    areaCoord.y = subpixelCoord.y / ((float32_t)shotCx->render.resolution.y);
    areaCoord = areaCoord * 2.0f - 1.0f;

    float32x3_t rayFocusPoint = (
        shotCx->camera.focusBasisU * areaCoord.x +
        shotCx->camera.focusBasisV * areaCoord.y +
        shotCx->camera.focusPosition
    );

    sampleCx->raySceneOrigin = (
        shotCx->camera.shotBasisU * areaCoord.x +
        shotCx->camera.shotBasisV * areaCoord.y +
        shotCx->camera.shotPosition
    );

    sampleCx->raySceneDirection = normalize(rayFocusPoint - sampleCx->raySceneOrigin);
}

#endif // _CLH_CS499R_PROGRAM_CAMERA
