
#include "cs499rProgramCommon.h"


// ----------------------------------------------------------------------------- CONSTANTES

__constant float32_t kRecursionCount = 9;
__constant float32_t kRandomPerRecursion = 2;


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
    __global common_coherency_context_t const * coherencyCx,
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

    { // set up random seed
        sampleCx.randomSeed = kickoffTileCoherencyTileId * (kRecursionCount * kRandomPerRecursion);
        sampleCx.randomSeed += coherencyCx->render.kickoffTileRandomSeedOffset;
    }

    camera_first_ray(&sampleCx, coherencyCx, pixelPos, coherencyCx->render.pixelSubpixelPos);
    scene_intersection(&sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

    float32x3_t sampleColor = sampleCx.rayInterMeshInstance->emitColor;
    float32x3_t sampleColorMultiply = sampleCx.rayInterMeshInstance->diffuseColor;

    for (uint32_t i = 0; i < kRecursionCount; i++)
    {
        path_tracer_rebound(&sampleCx);
        scene_intersection(&sampleCx, coherencyCx, meshInstances, meshOctreeNodes, primitives);

        sampleColor += sampleCx.rayInterMeshInstance->emitColor * sampleColorMultiply;
        sampleColorMultiply *= sampleCx.rayInterMeshInstance->diffuseColor;
    }

    { // save sample color
        uint32_t const pixelId = pixelPos.x + pixelPos.y * coherencyCx->render.resolution.x;

        renderTarget[pixelId * 3 + 0] += sampleColor.x;
        renderTarget[pixelId * 3 + 1] += sampleColor.y;
        renderTarget[pixelId * 3 + 2] += sampleColor.z;
    }
}
