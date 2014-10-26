
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
    __global common_mesh_octree_node_t const * meshOctreeNodes,
    __global common_primitive_t const * primitives,
    __global float32_t * renderTarget
)
{
    sample_context_t sampleCxArray[1];// CS499R_MAX_GROUP_SIZE
    sample_context_t * const sampleCx = sampleCxArray;

#if CS499R_CONFIG_PIXELPOS == CS499R_CONFIG_PIXELPOS_CPT
    uint32x2_t const pixelPos = kernel_pixel_pos_cpt(coherencyCx, sampleCx);

#elif CS499R_CONFIG_PIXELPOS == CS499R_CONFIG_PIXELPOS_ICPT
    uint32x2_t const pixelPos = kernel_pixel_pos_icpt(coherencyCx, sampleCx);

#elif CS499R_CONFIG_PIXELPOS == CS499R_CONFIG_PIXELPOS_DUMMY
# error "invalid CS499R_CONFIG_PIXELPOS_DUMMY not implemented yet"

#else
# error "invalid CS499R_CONFIG_PIXELPOS"

#endif // CS499R_CONFIG_PIXELPOS

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
