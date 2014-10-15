
#include "cs499rProgramCommon.h"


// ----------------------------------------------------------------------------- CONSTANTES

__constant float32_t kRecursionCount = 9;
__constant float32_t kRandomPerRecursion = 2;


// ----------------------------------------------------------------------------- PRECOMPUTED VALUES

#define pixelBorderSubpixelCount shotCx->render.z
#define subpixelSampleCount get_local_size(2)
#define subpixelSampleId get_local_id(2)
#define pixelSubpixelCount (pixelBorderSubpixelCount * pixelBorderSubpixelCount)
#define pixelSubpixelId (pixelSubpixelCoord.x + pixelSubpixelCoord.y * pixelBorderSubpixelCount)
#define pixelId (pixelCoord.x + pixelCoord.y * shotCx->render.x)


// ----------------------------------------------------------------------------- FUNCTIONS

inline
void
path_tracer_rebound(
    sample_context_t * const sampleCx
)
{
    sampleCx->raySceneOrigin += sampleCx->raySceneDirection * sampleCx->rayInterDistance;

    __global common_mesh_instance_t const * const meshInstance = sampleCx->rayInterMeshInstance;

    float32x3_t const meshNormal = sampleCx->rayInterMeshNormal;
    float32x3_t const n = (
        meshInstance->meshSceneMatrix.x * meshNormal.x +
        meshInstance->meshSceneMatrix.y * meshNormal.y +
        meshInstance->meshSceneMatrix.z * meshNormal.z
    );

    float32x3_t u;

    if (fabs(n.x) > fabs(n.y))
    {
        u = (float32x3_t)(-n.z, 0.0f, n.x);
    }
    else
    {
        u = (float32x3_t)(0.0f, -n.z, n.y);
    }

    float32x3_t const u2 = u * u;
    float32_t const r2 = random(sampleCx);
    float32_t const r2s = sqrt(r2) * rsqrt(u2.x + u2.y + u2.z);

    float32x3_t const v = cross(n, u);
    float32_t const r1 = 2.0f * kPi * random(sampleCx);

    sampleCx->raySceneDirection = (
        u * (cos(r1) * r2s) +
        v * (sin(r1) * r2s) +
        n * sqrt(1.0f - r2)
    );
}

/*
 * Path tracer's main entry point
 */
__kernel void
kernel_path_tracer_main(
    __global common_shot_context_t const * shotCx,
    __global common_mesh_instance_t const * meshInstances,
    __global common_mesh_octree_node_t const * meshOctreeNodes,
    __global common_primitive_t const * primitives,
    __global float32_t * renderTarget
)
{
    uint32x2_t const pixelCoord = (uint32x2_t)(
        get_global_id(0) / pixelBorderSubpixelCount,
        get_global_id(1) / pixelBorderSubpixelCount
    );

    if (pixelCoord.x >= shotCx->render.x || pixelCoord.y >= shotCx->render.y)
    {
        return;
    }

    uint32x2_t const pixelSubpixelCoord = (uint32x2_t)(get_local_id(0), get_local_id(1));

    sample_context_t sampleCx;

    { // set up random seed
        uint32_t const subpixelId = (
            (
                (pixelCoord.x & 0x3F) +
                (pixelCoord.y & 0x3F) * 0x3F
            ) * pixelSubpixelCount +
            pixelSubpixelId
        );
        uint32_t const sampleId = subpixelId * subpixelSampleCount + subpixelSampleId;

        sampleCx.randomSeed = sampleId * (kRecursionCount * kRandomPerRecursion);
        sampleCx.randomSeed = sampleCx.randomSeed % 1436283;
    }

    camera_first_ray(&sampleCx, shotCx, pixelCoord, pixelSubpixelCoord);
    scene_intersection(&sampleCx, shotCx, meshInstances, meshOctreeNodes, primitives);

    float32x3_t sampleColor = sampleCx.rayInterMeshInstance->emitColor;
    float32x3_t sampleColorMultiply = sampleCx.rayInterMeshInstance->diffuseColor;

    for (uint32_t i = 0; i < kRecursionCount; i++)
    {
        path_tracer_rebound(&sampleCx);
        scene_intersection(&sampleCx, shotCx, meshInstances, meshOctreeNodes, primitives);

        sampleColor += sampleCx.rayInterMeshInstance->emitColor * sampleColorMultiply;
        sampleColorMultiply *= sampleCx.rayInterMeshInstance->diffuseColor;
    }

    { // logarithmic sum of pixelSampleColors[]
        /*
         * This algorithm is dedicated to compute the average color
         * value over pixelSampleColors[]. Here is an example of the
         * optimization for a warp size of 2 threads and group size of
         * a total of 8 threads.
         *
         *  warp id | local id | offset 4 | offset 2 | offset 1
         *          |          |
         *     0    |    0     | -+------- ---+------ ----+-----
         *     0    |    1     | -|-+----- ---|-+---- ----/
         *     1    |    2     | -|-|-+--- ---/ |  * warps 1 looping
         *     1    |    3     | -|-|-|-+- -----/  * on the barrier()
         *     2    |    4     | -/ | | |  *
         *     2    |    5     | ---/ | |  * warps 2 & 3 looping
         *     3    |    6     | -----/ |  * on the barrier()
         *     3    |    7     | -------/  *
         */
        uint32_t const pixelSampleId = pixelSubpixelId + subpixelSampleId * pixelSubpixelCount;
        uint32_t const pixelSampleCount = pixelSubpixelCount * subpixelSampleCount;

        float32x3_t pixelColor = sampleColor;

        __local float32x3_t pixelSampleColors[1024];
        __local float32x3_t * pixelSampleColor = pixelSampleColors + pixelSampleId;

        pixelSampleColor[0] = pixelColor;

        for (uint32_t sampleOffset = (pixelSampleCount >> 1); sampleOffset != 0; sampleOffset >>= 1)
        {
            barrier(CLK_LOCAL_MEM_FENCE);

            if (pixelSampleId < sampleOffset)
            {
                /*
                 * This condition here is very important, when the group
                 * is instantiated with severals warps... Indeed, this
                 * condition will then avoid most of the warps to do
                 * useless operations while other still have threads
                 * doing the additions to finish up the sum. Therefor
                 * this idle time because of the barrier() can releases
                 * GPU's ALUs usages.
                 */
                pixelColor += pixelSampleColor[sampleOffset];

                pixelSampleColor[0] = pixelColor;
            }
        }

        if (pixelSampleId == 0)
        {
            pixelColor *= (1.0f / (float32_t) pixelSampleCount);

            __global float32_t * pixelTarget = renderTarget + pixelId * 3;

            pixelTarget[0] = pixelColor.x;
            pixelTarget[1] = pixelColor.y;
            pixelTarget[2] = pixelColor.z;
        }
    }
}
