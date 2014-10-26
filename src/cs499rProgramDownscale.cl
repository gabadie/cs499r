
#include "cs499rProgramPrefix.h"


__kernel void
kernel_main(
    common_downscale_context_t const ctx,
    __global float32_t const * const srcRenderTarget,
    __global float32_t * const destRenderTarget
)
{
    // the pixel id in the destination tile
    uint32_t const destTilePixelId = get_global_id(0);
    uint32x2_t const destTilePixelPos = (uint32x2_t)(
        destTilePixelId & (ctx.destTileSize.value - 1),
        destTilePixelId >> ctx.destTileSize.log
    );

    uint32x2_t const destPixelPos = destTilePixelPos + ctx.destTilePos;

    if (destPixelPos.x >= ctx.destResolution.x || destPixelPos.y >= ctx.destResolution.y)
    {
        return;
    }

    uint32_t const destPixelId = destPixelPos.x + destPixelPos.y * ctx.destResolution.x;

    float32x3_t sampleColor = (float32x3_t)(0.0f, 0.0f, 0.0f);

    for (uint32_t y = 0; y < ctx.downscaleFactor.value; y++)
    {
        for (uint32_t x = 0; x < ctx.downscaleFactor.value; x++)
        {
            uint32x2_t const srcPixelPos = (
                ctx.srcTilePos +
                destTilePixelPos * ctx.downscaleFactor.value +
                (uint32x2_t)(x, y)
            );
            uint32_t const srcPixelId = srcPixelPos.x + srcPixelPos.y * ctx.srcResolution.x;

            sampleColor.x += srcRenderTarget[srcPixelId * 3 + 0];
            sampleColor.y += srcRenderTarget[srcPixelId * 3 + 1];
            sampleColor.z += srcRenderTarget[srcPixelId * 3 + 2];
        }
    }

    sampleColor = sampleColor * ctx.multiplyFactor;

    destRenderTarget[destPixelId * 3 + 0] = sampleColor.x;
    destRenderTarget[destPixelId * 3 + 1] = sampleColor.y;
    destRenderTarget[destPixelId * 3 + 2] = sampleColor.z;
}
