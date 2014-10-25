
#ifndef _CLH_CS499R_PROGRAM_MAIN_KERNEL
#define _CLH_CS499R_PROGRAM_MAIN_KERNEL

#include "cs499rProgramCommon.h"


/*
 * Coherent path tracing algorithm coherent tiles
 *
 *
 *      |    kickoff tile   |
 *           (0,1)
 *  --   -------------------   --  --
 *      |                   |
 *      |    kickoff tile   | kickoff tile
 *      |    (0,0)          | (1,0)
 *      |                   |
 *      |                   |
 *      |     coherency     |
 *      |---- tile (0, 0)   |
 *      |    |              |
 *      |    |              |
 *  --   -------------------   --  --
 *
 *      |                   |
 */

/*
 * Tiled Coherent Path Tracing
 */
inline
uint32x2_t
kernel_pixel_pos_tiled_cpt(
    __global common_render_context_t const * const coherencyCx,
    sample_context_t * const sampleCx
)
{
    // the coherency tile id in the kick of tile
    uint32_t const kickoffTileCoherencyTileId = get_group_id(0) >> coherencyCx->render.cbt.groupPerCoherencyTile.log;

    // the coherency tile pos in the kick of tile
    uint32x2_t const kickoffTileCoherencyTilePos = (uint32x2_t)(
        kickoffTileCoherencyTileId & (coherencyCx->render.cbt.coherencyTilePerKickoffTileBorder.value - 1),
        kickoffTileCoherencyTileId >> coherencyCx->render.cbt.coherencyTilePerKickoffTileBorder.log
    );

    // the thread id in the coherency tile
    uint32_t const coherencyTileGroupId = get_group_id(0) & (coherencyCx->render.cbt.groupPerCoherencyTile.value - 1);

    // the thread id in the coherency tile
    uint32_t const coherencyTileThreadId = get_local_id(0) + get_local_size(0) * coherencyTileGroupId;

    // the pixel id in the coherency tile
    uint32_t const coherencyTilePixelId = coherencyTileThreadId;

    // the pixel pos in the coherency tile
    uint32x2_t const coherencyTilePixelPos = (uint32x2_t)(
        coherencyTilePixelId & (coherencyCx->render.cbt.coherencyTileSize.value - 1),
        coherencyTilePixelId >> coherencyCx->render.cbt.coherencyTileSize.log
    );

    // the pixel pos in the kickoff tile
    uint32x2_t const kickoffTilePixelPos = (
        kickoffTileCoherencyTilePos * coherencyCx->render.cbt.coherencyTileSize.value +
        coherencyTilePixelPos
    );

    { // set up random seed
        sampleCx->randomSeed = (
            kPathTracerRandomPerRay *
            coherencyCx->render.kickoffSampleRecursionCount *
            coherencyCx->render.passId
        );
    }

    // the pixel pos
    return kickoffTilePixelPos + coherencyCx->render.kickoffTilePos;
}

/*
 * Tiled Interleaved Coherent Path Tracing
 */
inline
uint32x2_t
kernel_pixel_pos_tiled_icpt(
    __global common_render_context_t const * const coherencyCx,
    sample_context_t * const sampleCx
)
{
    // the coherency tile id in the kick of tile
    uint32_t const kickoffTileCoherencyTileId = get_group_id(0) >> coherencyCx->render.icpt.groupPerCoherencyTile.log;

    // the coherency tile pos in the kick of tile
    uint32x2_t const kickoffTileCoherencyTilePos = (uint32x2_t)(
        kickoffTileCoherencyTileId & (coherencyCx->render.icpt.coherencyTilePerKickoffTileBorder.value - 1),
        kickoffTileCoherencyTileId >> coherencyCx->render.icpt.coherencyTilePerKickoffTileBorder.log
    );

    // the group id in the coherency tile
    uint32_t const coherencyTileGroupId = get_group_id(0) & (coherencyCx->render.icpt.groupPerCoherencyTile.value - 1);

    // the thread id in the coherency tile
    uint32_t const coherencyTileThreadId = get_local_id(0) + get_local_size(0) * coherencyTileGroupId;

    // the pixel id in the coherency tile
    uint32_t const coherencyTilePixelId = coherencyTileThreadId;

    uint32_t const coherencyTilePixelPosY = latin_hypercube_x_pos(
        (2 * coherencyCx->render.passId + 0) & (kLatinHypercubeSize - 1),
        coherencyTilePixelId & (kLatinHypercubeSize - 1)
    );
    uint32_t const coherencyTileInterleaveId = latin_hypercube_x_pos(
        (2 * coherencyCx->render.passId + 1) & (kLatinHypercubeSize - 1),
        coherencyTilePixelId >> kLatinHypercubeSizeLog
    );

    // the pixel pos in the coherency tile
    uint32x2_t const coherencyTilePixelPos = (uint32x2_t)(
        latin_hypercube_x_pos(coherencyTileInterleaveId, coherencyTilePixelPosY),
        coherencyTilePixelPosY
    );

    // the pixel pos in the kickoff tile
    uint32x2_t const kickoffTilePixelPos = (
        kickoffTileCoherencyTilePos * kLatinHypercubeSize +
        coherencyTilePixelPos
    );

    { // set up random seed
        sampleCx->randomSeed = (
            kPathTracerRandomPerRay *
            coherencyCx->render.kickoffSampleRecursionCount *
            coherencyCx->render.passId *
            coherencyCx->render.icpt.groupPerCoherencyTile.value +
            coherencyTileGroupId
        );
    }

    // the pixel pos
    return kickoffTilePixelPos + coherencyCx->render.kickoffTilePos;
}


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
    sample_context_t sampleCxArray[1];// CS499R_MAX_GROUP_SIZE
    sample_context_t * const sampleCx = sampleCxArray;

#if defined(_CL_PROGRAM_CPT)
    uint32x2_t const pixelPos = kernel_pixel_pos_tiled_cpt(coherencyCx, sampleCx);
#else // defined(_CL_PROGRAM_CPT)
    uint32x2_t const pixelPos = kernel_pixel_pos_tiled_icpt(coherencyCx, sampleCx);
#endif // defined(_CL_PROGRAM_CPT)

    if (pixelPos.x >= coherencyCx->render.resolution.x || pixelPos.y >= coherencyCx->render.resolution.y)
    {
        return;
    }

#if defined(_CL_PROGRAM_PATH_TRACER)
    /*
     * Here is the path tracer's code
     */
    float32x3_t sampleColor = (float32x3_t)(0.0f, 0.0f, 0.0f);

    for (uint32_t i = 0; i < coherencyCx->render.kickoffSampleIterationCount; i++)
    {
        camera_first_ray(sampleCx, coherencyCx, pixelPos, coherencyCx->render.pixelSubpixelPos);
        scene_intersection(sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

        sampleColor += sampleCx->rayInterMeshInstance->emitColor;
        float32x3_t sampleColorMultiply = sampleCx->rayInterMeshInstance->diffuseColor;

        for (uint32_t i = 1; i < coherencyCx->render.kickoffSampleRecursionCount; i++)
        {
            path_tracer_rebound(sampleCx);
            scene_intersection(sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

            sampleColor += sampleCx->rayInterMeshInstance->emitColor * sampleColorMultiply;
            sampleColorMultiply *= sampleCx->rayInterMeshInstance->diffuseColor;
        }
    }


#elif defined(_CL_PROGRAM_DEBUG_NORMAL)
    /*
     * Here is the normal debuger ray tracer's code
     */
    camera_first_ray(sampleCx, coherencyCx, pixelPos, coherencyCx->render.pixelSubpixelPos);
    scene_intersection(sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

    __global common_mesh_instance_t const * const meshInstance = sampleCx->rayInterMeshInstance;

    float32x3_t const meshNormal = sampleCx->rayInterMeshNormal;
    float32x3_t const sceneNormal = (
        meshInstance->meshSceneMatrix.x * meshNormal.x +
        meshInstance->meshSceneMatrix.y * meshNormal.y +
        meshInstance->meshSceneMatrix.z * meshNormal.z
    );

    float32x3_t sampleColor = sceneNormal * 0.5f + 0.5f;


#endif

#ifdef _CL_DEBUG
    {
        sampleColor = sampleCx->pixelColor;
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
