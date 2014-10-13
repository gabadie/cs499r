
#ifndef _CLH_CS499R_PROGRAM_COMMON
#define _CLH_CS499R_PROGRAM_COMMON

#include "cs499rProgramPrefix.h"


// ----------------------------------------------------------------------------- CONSTANTS

__constant float32_t kEPSILONE = 0.00001f;
__constant float32_t kPi = 3.14159265359f;



// ----------------------------------------------------------------------------- STRUCTS

typedef
__private struct sample_context_s
{
    // ray's origin in the mesh space
    float32x3_t raySceneOrigin;

    // normalized ray's direction in the mesh space
    float32x3_t raySceneDirection;

    // ray's origin in the mesh space
    float32x3_t rayMeshOrigin;

    // normalized ray's direction in the mesh space
    float32x3_t rayMeshDirection;

    // ray's minimal distance found for depth test
    float32_t rayInterDistance;

    // the ray's normalized intersection normal in the mesh space
    float32x3_t rayInterMeshNormal;

    // the currently bound mesh instance
    __global common_mesh_instance_t const * rayInterMeshInstance;

    // the sample's random seed
    uint32_t randomSeed;

    // the currently bound mesh instance
    __global common_mesh_instance_t const * boundMeshInstance;

} sample_context_t;


// ----------------------------------------------------------------------------- FUNCTIONS

inline
float32_t
noise(uint32_t seed)
{
    float32_t s = sin((float32_t)seed * 12.9898f) * 43758.5453f;

    return s - floor(s);
}

inline
float32_t
random(sample_context_t * sampleCx)
{
    uint32_t seed = sampleCx->randomSeed++;

    return noise(seed);
}

void
primitive_intersection(
    sample_context_t * sampleCx,
    __global common_primitive_t const * primitive
)
{
    /*
     *      (triangle)
     *          |
     *    \     |
     *      \   |
     *        \ |             (normal)
     *          I--------------->
     *          | \
     *          |   \
     *          |     \
     *          |       \
     *          |         \
     *          A           \   (ray)
     *          |  \          \
     *          |     \         \
     *          |        \        \
     *          |           \       \       ->
     *          |              \      \ __  d
     *          |                 \    |\
     *          |                    \    \
     *          |                       \   \
     *          |                          \  \
     *          |                             \ \
     *          |                                \\
     *          H  -  -  -  -  -  -  -  -  -  -  -  O
     *          |
     *          |
     *
     *
     * Glossary:
     *      Point `O` is the origin of the ray (sampleCx->rayOrigin)
     *      Vector `d` is direction of the ray (sampleCx->rayDirection)
     *      Point `A` is the first vertex of the current triangle (primitive->v0)
     *      Point `I` is the ray intersection with the triangle
     *      Distance `OI` is the intersection distance (rayInterDistance)
     *
     *
     *
     *
     */

    float32x3_t const normal = (float32x3_t)(primitive->v0.w, primitive->e0.w, primitive->e1.w);
    float32x3_t const vAO = sampleCx->rayMeshOrigin - primitive->v0.xyz;
    float32_t const OH = dot(vAO, normal);
    float32_t const normalDotRay = dot(sampleCx->rayMeshDirection, normal);
    float32_t const rayInterDistance = - OH / normalDotRay;

    if (isless(rayInterDistance, kEPSILONE) || isgreaterequal(rayInterDistance, sampleCx->rayInterDistance))
    {
        // cull this triangle
        return;
    }

    float32x3_t const vAI = vAO + sampleCx->rayMeshDirection * rayInterDistance;
    float32x2_t const uv = (
        primitive->uvMatrix.x * dot(vAI, primitive->e0.xyz) +
        primitive->uvMatrix.y * dot(vAI, primitive->e1.xyz)
    );

    if ((uv.x < 0.0f) || (uv.y < 0.0f) || ((uv.x + uv.y) > 1.0f))
    {
        // the intersection would be outside the triangle
        return;
    }

    // there is an intersection, so we update the context

    sampleCx->rayInterDistance = rayInterDistance;
    sampleCx->rayInterMeshNormal = normal;
    sampleCx->rayInterMeshInstance = sampleCx->boundMeshInstance;
}

inline
void
mesh_instance_intersection(
    sample_context_t * sampleCx,
    __global common_mesh_octree_node_t const * meshOctreeNodes,
    __global common_primitive_t const * primitives
)
{
    __global common_mesh_instance_t const * const meshInstance = sampleCx->boundMeshInstance;

    sampleCx->rayMeshOrigin = (
        meshInstance->sceneMeshMatrix.x * sampleCx->raySceneOrigin.x +
        meshInstance->sceneMeshMatrix.y * sampleCx->raySceneOrigin.y +
        meshInstance->sceneMeshMatrix.z * sampleCx->raySceneOrigin.z +
        meshInstance->sceneMeshMatrix.w
    );

    sampleCx->rayMeshDirection = (
        meshInstance->sceneMeshMatrix.x * sampleCx->raySceneDirection.x +
        meshInstance->sceneMeshMatrix.y * sampleCx->raySceneDirection.y +
        meshInstance->sceneMeshMatrix.z * sampleCx->raySceneDirection.z
    );

    __global common_primitive_t const * const meshPrimitives = primitives + meshInstance->mesh.primFirst;
    __global common_mesh_octree_node_t const * const meshNodes = meshOctreeNodes + meshInstance->mesh.octreeRootGlobalId;
    uint32_t const nodeCount = meshInstance->mesh.octreeNodeCount;

    for (uint32_t nodeId = 0; nodeId < nodeCount; nodeId++)
    {
        __global common_mesh_octree_node_t const * const node = meshNodes + nodeId;
        uint32_t const primEnd = node->primFirst + node->primCount;

        for (uint32_t primId = node->primFirst; primId < primEnd; primId++)
        {
            primitive_intersection(sampleCx, meshPrimitives + primId);
        }
    }
}

inline
void
scene_intersection(
    sample_context_t * sampleCx,
    __global common_shot_context_t const * shotCx,
    __global common_mesh_instance_t const * meshInstances,
    __global common_mesh_octree_node_t const * meshOctreeNodes,
    __global common_primitive_t const * primitives
)
{
    sampleCx->rayInterDistance = INFINITY;

    /*
     * We sets up the intersection mesh as the anonymous mesh first
     */
    sampleCx->rayInterMeshInstance = meshInstances;

    /*
     * i = 1 because we skip over the anonymous mesh instance
     */
    for (uint32_t i = 1; i < shotCx->meshInstanceMaxId; i++)
    {
        sampleCx->boundMeshInstance = meshInstances + i;

        mesh_instance_intersection(sampleCx, meshOctreeNodes, primitives);
    }
}

inline
void
camera_first_ray(
    sample_context_t * const sampleCx,
    __global common_shot_context_t const * shotCx,
    uint32x2_t const pixelCoord,
    uint32x2_t const pixelSubpixelCoord
)
{
    float32x2_t const subpixelCoord = (float32x2_t)(pixelCoord.x, pixelCoord.y) +
            (1.0f + 2.0f * (float32x2_t)(pixelSubpixelCoord.x, pixelSubpixelCoord.y)) *
            (0.5f / (float32_t) shotCx->render.z);

    float32x2_t areaCoord;
    areaCoord.x = subpixelCoord.x / ((float32_t)shotCx->render.x);
    areaCoord.y = subpixelCoord.y / ((float32_t)shotCx->render.y);
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

#endif // _CLH_CS499R_PROGRAM_COMMON
