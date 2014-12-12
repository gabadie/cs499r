
#include "cs499rProgramPrefix.h"


__kernel void
kernel_main(
    float32_t multiplyFactor,
    __global float32_t * renderTarget
)
{
    uint32_t const pixelId = get_global_id(0);

    renderTarget[pixelId * 3 + 0] *= multiplyFactor;
    renderTarget[pixelId * 3 + 1] *= multiplyFactor;
    renderTarget[pixelId * 3 + 2] *= multiplyFactor;
}
