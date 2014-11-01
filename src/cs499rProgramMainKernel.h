
#ifndef _CLH_CS499R_PROGRAM_MAIN_KERNEL
#define _CLH_CS499R_PROGRAM_MAIN_KERNEL

#include "cs499rProgramPrefix.h"
#include "cs499rProgramConsts.h"

#include "cs499rProgramCamera.h"
#include "cs499rProgramIntersection.h"
#include "cs499rProgramMesh.h"
#include "cs499rProgramOctree.h"
#include "cs499rProgramPathTracer.h"
#include "cs499rProgramPixelPos.h"
#include "cs499rProgramSampleContext.h"
#include "cs499rProgramScene.h"
#include "cs499rProgramUtils.h"


/*
 * Path tracer's main entry point
 */
__kernel void
kernel_main(
    __global common_render_context_t const * coherencyCx,
    __global common_mesh_instance_t const * meshInstances,
    __global common_octree_node_t const * octreeNodes,
    __global common_primitive_t const * primitives,
    __global float32_t * renderTarget
)
{
    sample_context_t sampleCxArray[1];// CS499R_MAX_GROUP_SIZE
    sample_context_t * const sampleCx = sampleCxArray;

    uint32x2_t const virtualPixelPos = kernel_pixel_pos(coherencyCx, sampleCx);

    if (
        virtualPixelPos.x >= coherencyCx->render.virtualTargetResolution.x ||
        virtualPixelPos.y >= coherencyCx->render.virtualTargetResolution.y
    )
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
        camera_first_ray(sampleCx, coherencyCx, virtualPixelPos, coherencyCx->render.pixelSubpixelPos);
        scene_intersection(sampleCx, coherencyCx, meshInstances, octreeNodes, primitives);

        sampleColor += sampleCx->rayInterMeshInstance->emitColor;
        float32x3_t sampleColorMultiply = sampleCx->rayInterMeshInstance->diffuseColor;

        for (uint32_t i = 1; i < coherencyCx->render.kickoffSampleRecursionCount; i++)
        {
            path_tracer_rebound(sampleCx);
            scene_intersection(sampleCx, coherencyCx, meshInstances, octreeNodes, primitives);

            sampleColor += sampleCx->rayInterMeshInstance->emitColor * sampleColorMultiply;
            sampleColorMultiply *= sampleCx->rayInterMeshInstance->diffuseColor;
        }
    }


#elif defined(_CL_PROGRAM_DEBUG_NORMAL)
    /*
     * Here is the normal debuger ray tracer's code
     */
    camera_first_ray(sampleCx, coherencyCx, virtualPixelPos, coherencyCx->render.pixelSubpixelPos);
    scene_intersection(sampleCx, coherencyCx, meshInstances, octreeNodes, primitives);

    __global common_mesh_instance_t const * const meshInstance = sampleCx->rayInterMeshInstance;

    float32x3_t const meshNormal = sampleCx->rayInterMeshNormal;
    float32x3_t const sceneNormal = (
        meshInstance->meshSceneMatrix.x * meshNormal.x +
        meshInstance->meshSceneMatrix.y * meshNormal.y +
        meshInstance->meshSceneMatrix.z * meshNormal.z
    );

    float32x3_t sampleColor = sceneNormal * 0.5f + 0.5f;


#elif defined(_CL_PROGRAM_RAY_STATS)
    /*
     * Here is the ray stats code
     */
# ifndef CS499R_CONFIG_RAY_STATS_FACTOR
#  error "need #define CS499R_CONFIG_RAY_STATS_FACTOR"
# endif

    sampleCx->stats = 0;

    camera_first_ray(sampleCx, coherencyCx, virtualPixelPos, coherencyCx->render.pixelSubpixelPos);
    scene_intersection(sampleCx, coherencyCx, meshInstances, octreeNodes, primitives);

    __global common_mesh_instance_t const * const meshInstance = sampleCx->rayInterMeshInstance;

    float32x3_t const meshNormal = sampleCx->rayInterMeshNormal;
    float32x3_t const sceneNormal = (
        meshInstance->meshSceneMatrix.x * meshNormal.x +
        meshInstance->meshSceneMatrix.y * meshNormal.y +
        meshInstance->meshSceneMatrix.z * meshNormal.z
    );

    float32x3_t sampleNormal = clamp(sceneNormal * 0.25f + 0.25f, 0.0f, 0.5f);
    float32_t sampleNormalBW = 0.33f * (sampleNormal.x + sampleNormal.y + sampleNormal.z);

    float32_t statValue = (
        CS499R_CONFIG_RAY_STATS_FACTOR *
        (float32_t)(sampleCx->stats)
    );
    float32x3_t sampleColor = (float32x3_t)(
        sampleNormalBW + statValue,
        sampleNormalBW,
        sampleNormalBW
    );


#endif

#ifdef _CL_DEBUG
    {
        sampleColor = sampleCx->pixelColor;
    }
#endif // _CL_DEBUG

    { // save sample color
        uint32x2_t const targetVirtualPixelPos = virtualPixelPos - coherencyCx->render.targetVirtualOffset;
        uint32_t const targetVirtualPixelId = (
            targetVirtualPixelPos.x +
            targetVirtualPixelPos.y * coherencyCx->render.targetResolution.x
        );

        renderTarget[targetVirtualPixelId * 3 + 0] += sampleColor.x;
        renderTarget[targetVirtualPixelId * 3 + 1] += sampleColor.y;
        renderTarget[targetVirtualPixelId * 3 + 2] += sampleColor.z;
    }
}


#endif
