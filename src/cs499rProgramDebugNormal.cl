
//#define _CL_DEBUG
#include "cs499rProgramCommon.h"


// ----------------------------------------------------------------------------- FUNCTIONS

__kernel
void
kernel_debug_normal(
    __global common_shot_context_t const * shotCx,
    __global common_mesh_instance_t const * meshInstances,
    __global common_mesh_octree_node_t const * meshOctreeNodes,
    __global common_primitive_t const * primitives,
    __global float32_t * renderTarget
)
{
    uint32x2_t const pixelCoord = (uint32x2_t)(
        get_global_id(0),
        get_global_id(1)
    );

    if (pixelCoord.x >= shotCx->render.resolution.x || pixelCoord.y >= shotCx->render.resolution.y)
    {
        return;
    }

    uint32x2_t const pixelSubpixelCoord = (uint32x2_t)(0, 0);

    sample_context_t sampleCx;

    camera_first_ray(&sampleCx, shotCx, pixelCoord, pixelSubpixelCoord);
    scene_intersection(&sampleCx, shotCx, meshInstances, meshOctreeNodes, primitives);

    __global common_mesh_instance_t const * const meshInstance = sampleCx.rayInterMeshInstance;

    float32x3_t const meshNormal = sampleCx.rayInterMeshNormal;
    float32x3_t const sceneNormal = (
        meshInstance->meshSceneMatrix.x * meshNormal.x +
        meshInstance->meshSceneMatrix.y * meshNormal.y +
        meshInstance->meshSceneMatrix.z * meshNormal.z
    );

    float32x3_t pixelColor = sceneNormal * 0.5f + 0.5f;

#ifdef _CL_DEBUG
    {
        pixelColor = sampleCx.pixelColor;
    }
#endif

    uint32_t const pixelId = pixelCoord.x + pixelCoord.y * shotCx->render.resolution.x;

    __global float32_t * pixelTarget = renderTarget + pixelId * 3;

    pixelTarget[0] = pixelColor.x;
    pixelTarget[1] = pixelColor.y;
    pixelTarget[2] = pixelColor.z;
}
