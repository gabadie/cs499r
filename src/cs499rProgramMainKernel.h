
#ifndef _CLH_CS499R_PROGRAM_MAIN_KERNEL
#define _CLH_CS499R_PROGRAM_MAIN_KERNEL

#include "cs499rProgramCommon.h"


/*
 * Path tracer's main entry point
 */
__kernel void
kernel_main(
    __global common_render_context_t const * coherencyCx,
    __global common_mesh_instance_t const * meshInstances,
    __global common_mesh_octree_node_t const * meshOctreeNodes,
    __global common_primitive_t const * primitives,
    __global float32_t * renderTarget
)
{
    // the coherency tile id in the kick of tile
    uint32_t const kickoffTileCoherencyTileId = get_group_id(0);

    // the coherency tile pos in the kick of tile
    uint32x2_t const kickoffTileCoherencyTilePos = (uint32x2_t)(
        kickoffTileCoherencyTileId & (coherencyCx->render.coherencyTilePerKickoffTileBorder - 1),
        kickoffTileCoherencyTileId >> coherencyCx->render.coherencyTilePerKickoffTileBorderLog
    );

    // the thread id in the coherency tile
    uint32_t const coherencyTileThreadId = get_local_id(0);

    // the warp id in the coherency tile
    uint32_t const coherencyTileWarpId = coherencyTileThreadId / kCS499RGpuWarpSize;

    // the pixel id in the coherency tile
    uint32_t const coherencyTilePixelId = coherencyTileThreadId;

    // the pixel pos in the coherency tile
    uint32x2_t const coherencyTilePixelPos = (uint32x2_t)(
        coherencyTilePixelId & (coherencyCx->render.coherencyTileSize - 1),
        coherencyTilePixelId >> coherencyCx->render.coherencyTileSizeLog
    );

    // the pixel pos in the kickoff tile
    uint32x2_t const kickoffTilePixelPos = (
        kickoffTileCoherencyTilePos * coherencyCx->render.coherencyTileSize +
        coherencyTilePixelPos
    );

    // the pixel pos
    uint32x2_t const pixelPos = kickoffTilePixelPos + coherencyCx->render.kickoffTilePos;

    if (pixelPos.x >= coherencyCx->render.resolution.x || pixelPos.y >= coherencyCx->render.resolution.y)
    {
        return;
    }

    sample_context_t sampleCx;

#if defined(_CL_PROGRAM_PATH_TRACER)
    /*
     * Here is the path tracer's code
     */
    { // set up random seed
        sampleCx.randomSeed = (
            coherencyCx->render.kickoffTileRandomSeedOffset
        );
    }

    float32x3_t sampleColor = (float32x3_t)(0.0f, 0.0f, 0.0f);

    for (uint32_t i = 0; i < coherencyCx->render.kickoffSampleIterationCount; i++)
    {
        camera_first_ray(&sampleCx, coherencyCx, pixelPos, coherencyCx->render.pixelSubpixelPos);
        scene_intersection(&sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

        sampleColor += sampleCx.rayInterMeshInstance->emitColor;
        float32x3_t sampleColorMultiply = sampleCx.rayInterMeshInstance->diffuseColor;

        for (uint32_t i = 0; i < coherencyCx->render.kickoffSampleRecursionCount; i++)
        {
            path_tracer_rebound(&sampleCx);
            scene_intersection(&sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

            sampleColor += sampleCx.rayInterMeshInstance->emitColor * sampleColorMultiply;
            sampleColorMultiply *= sampleCx.rayInterMeshInstance->diffuseColor;
        }
    }


#elif defined(_CL_PROGRAM_DEBUG_NORMAL)
    /*
     * Here is the normal debuger ray tracer's code
     */
    camera_first_ray(&sampleCx, coherencyCx, pixelPos, coherencyCx->render.pixelSubpixelPos);
    scene_intersection(&sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

    __global common_mesh_instance_t const * const meshInstance = sampleCx.rayInterMeshInstance;

    float32x3_t const meshNormal = sampleCx.rayInterMeshNormal;
    float32x3_t const sceneNormal = (
        meshInstance->meshSceneMatrix.x * meshNormal.x +
        meshInstance->meshSceneMatrix.y * meshNormal.y +
        meshInstance->meshSceneMatrix.z * meshNormal.z
    );

    float32x3_t sampleColor = sceneNormal * 0.5f + 0.5f;


#endif

#ifdef _CL_DEBUG
    {
        sampleColor = sampleCx.pixelColor;
    }
#endif // _CL_DEBUG

    { // save sample color
        uint32_t const pixelId = pixelPos.x + pixelPos.y * coherencyCx->render.resolution.x;

        renderTarget[pixelId * 3 + 0] += sampleColor.x;
        renderTarget[pixelId * 3 + 1] += sampleColor.y;
        renderTarget[pixelId * 3 + 2] += sampleColor.z;
    }
}


#endif
