
#ifndef _CLH_CS499R_PROGRAM_COMMON
#define _CLH_CS499R_PROGRAM_COMMON

#include "cs499rProgramPrefix.h"


// ----------------------------------------------------------------------------- CONSTANTS

__constant float32_t kEPSILONE = 0.00001f;
__constant float32_t kPi = 3.14159265359f;

__constant uint32_t const kOctreeNodeSubdivisonCount = 8;
__constant uint32_t const kOctreeSubNodeMask = 0x7;
__constant uint32_t const kOctreeNodeStackSize = 32;



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

    // normalized ray's direction inverted
    float32x3_t rayMeshDirectionInverted;

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

#ifdef _CL_DEBUG
    float32x3_t pixelColor;
#endif

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
 *      Point `O` is the origin of the ray (sampleCx->rayMeshOrigin)
 *      Vector `d` is direction of the ray (sampleCx->rayMeshDirection)
 *      Point `A` is the first vertex of the current triangle (primitive->v0)
 *      Point `I` is the ray intersection with the triangle
 *      Distance `OI` is the intersection distance (rayInterDistance)
 */
void
primitive_intersection(
    sample_context_t * sampleCx,
    __global common_primitive_t const * primitive
)
{
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

/*
 * Detects if there is an intersection between the box and the context's ray.
 *
 *                 \
 *                  \
 *                   \
 *                    \
 *          H---------------G
 *          |           \   |
 *          |            \  |
 *          |             \ |
 *          |              \|
 *          |               I
 *          |               |\
 *          |               | \
 *          E---------------F  \
 *                              \
 *                               \
 *                                \    ->
 *                                 \__ d
 *                                 /\
 *                                   \
 *                                    O
 *  y
 *  ^
 *  |
 *  |
 *  z----> x
 *
 *
 * Glossary:
 *      Point `O` is the origin of the ray (sampleCx->rayMeshOrigin)
 *      Vector `d` is direction of the ray (sampleCx->rayMeshDirection)
 *      Point `I` is the nearest ray intersection with the box
 */
inline
uint32_t
box_intersection(sample_context_t const * const sampleCx, float32x3_t const OE, float32x3_t const OG)
{
    float32x3_t const t0 = OE * sampleCx->rayMeshDirectionInverted;
    float32x3_t const t1 = OG * sampleCx->rayMeshDirectionInverted;
    float32x3_t const tMin = min(t0, t1);
    float32x3_t const tMax = max(t0, t1);

    return (
        (tMin.x < tMax.y) &&
        (tMin.y < tMax.x) &&
        (max(tMin.x, tMin.y) < tMax.z) &&
        (tMin.z < min(tMax.x, tMax.y)) &&
        (max(tMax.x, max(tMax.y, tMax.z)) > 0.0f) &&
        (min(tMin.x, min(tMin.y, tMin.z)) < sampleCx->rayInterDistance)
    );
}

/*
 * This array is used to compute the the subnode infos by multiplying to the
 * subnode's size:
 *      xyz: the subnode's lower bound coordinates
 *      w: the subnode's half size
 */
__constant float32x4_t const kOctreeSubnodeInfoOffset[] = {
    (float32x4_t)(0.0f, 0.0f, 0.0f, -0.5f),
    (float32x4_t)(1.0f, 0.0f, 0.0f, -0.5f),
    (float32x4_t)(0.0f, 1.0f, 0.0f, -0.5f),
    (float32x4_t)(1.0f, 1.0f, 0.0f, -0.5f),
    (float32x4_t)(0.0f, 0.0f, 1.0f, -0.5f),
    (float32x4_t)(1.0f, 0.0f, 1.0f, -0.5f),
    (float32x4_t)(0.0f, 1.0f, 1.0f, -0.5f),
    (float32x4_t)(1.0f, 1.0f, 1.0f, -0.5f)
};

/*
 * This array is the sub-node access order depending on the ray direction.
 */
__constant uint32_t const kOctreeSubNodeAccessOrder[] = {
    0x01234567,
    0x10325476,
    0x23016745,
    0x32107654,
    0x45670123,
    0x54761032,
    0x67452301,
    0x76543210
};

inline
uint32_t
octree_sub_node_order(float32x3_t rayDirection)
{
    uint32_t const directctionId = (
        (uint32_t)(rayDirection.x >= 0.0f) |
        ((uint32_t)(rayDirection.y >= 0.0f) << 1) |
        ((uint32_t)(rayDirection.z >= 0.0f) << 2)
    );

    return kOctreeSubNodeAccessOrder[directctionId];
}

inline
void
mesh_octree_intersection(
    sample_context_t * const sampleCx,
    __global common_mesh_instance_t const * const meshInstance,
    __global common_primitive_t const * const meshPrimitives,
    __global common_mesh_octree_node_t const * const rootNode
)
{
    uint32_t nodeStackSize = 1;
    uint32_t nodeStack[kOctreeNodeStackSize];
    uint32_t subNodeAccessStack[kOctreeNodeStackSize];
    float32x4_t nodeInfosStack[kOctreeNodeStackSize];

    {
        nodeStack[0] = 0;
        subNodeAccessStack[0] = 0;
        nodeInfosStack[0].xyz = -sampleCx->rayMeshOrigin;
        nodeInfosStack[0].w = meshInstance->mesh.vertexUpperBound.w;
    }

//#define _CL_NO_MESH_OCTREE_SUBNODE_REORDERING
#ifndef _CL_NO_MESH_OCTREE_SUBNODE_REORDERING
    uint32_t const subNodeAccessOrder = octree_sub_node_order(sampleCx->rayMeshDirection);
#endif // _CL_NO_MESH_OCTREE_SUBNODE_REORDERING

    while (1)
    {
        uint32_t const subNodeAccessId = subNodeAccessStack[nodeStackSize - 1];

#ifndef _CL_NO_MESH_OCTREE_SUBNODE_REORDERING
        uint32_t const subNodeId = (subNodeAccessOrder >> (subNodeAccessId * 4)) & kOctreeSubNodeMask;
#else // _CL_NO_MESH_OCTREE_SUBNODE_REORDERING
        uint32_t const subNodeId = subNodeAccessId;
#endif // _CL_NO_MESH_OCTREE_SUBNODE_REORDERING

        __global common_mesh_octree_node_t const * const node = rootNode + nodeStack[nodeStackSize - 1];

        if (subNodeAccessId == kOctreeNodeSubdivisonCount)
        {
            uint32_t const primEnd = node->primFirst + node->primCount;

            for (uint32_t primId = node->primFirst; primId < primEnd; primId++)
            {
                primitive_intersection(sampleCx, meshPrimitives + primId);
            }

            if (nodeStackSize == 1)
            {
                return;
            }

            // going up
            nodeStackSize--;
            continue;
        }

        subNodeAccessStack[nodeStackSize - 1] = subNodeAccessId + 1;

        if (node->subNodeOffsets[subNodeId] == 0)
        {
            continue;
        }

        float32x4_t const nodeInfos = nodeInfosStack[nodeStackSize - 1];
        float32x4_t const subNodeInfos = (
            nodeInfos.w * kOctreeSubnodeInfoOffset[subNodeId] +
            nodeInfos
        );

        if (!box_intersection(sampleCx, subNodeInfos.xyz - subNodeInfos.w, subNodeInfos.xyz + 3.0f * subNodeInfos.w))
        {
            continue;
        }

        // going down
        nodeStack[nodeStackSize] = node->subNodeOffsets[subNodeId];
        subNodeAccessStack[nodeStackSize] = 0;
        nodeInfosStack[nodeStackSize] = subNodeInfos;

        nodeStackSize++;
    }
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

    sampleCx->rayMeshDirectionInverted = 1.0f / sampleCx->rayMeshDirection;

#ifndef _CL_NO_BOUNDING_BOX_CHECKING
    {
        float32x3_t const OE = -sampleCx->rayMeshOrigin;
        float32x3_t const OG = OE + meshInstance->mesh.vertexUpperBound.xyz;

        if (!box_intersection(sampleCx, OE, OG))
        {
            /*
             * The ray has no intersection with this mesh instance's bounding
             * box, so we skip it right away.
             */
            return;
        }
    }
#endif // _CL_NO_BOUNDING_BOX_CHECKING

    __global common_primitive_t const * const meshPrimitives = primitives + meshInstance->mesh.primFirst;
    __global common_mesh_octree_node_t const * const meshRootNode = meshOctreeNodes + meshInstance->mesh.octreeRootGlobalId;

#ifdef _CL_NO_OCTREE_RAY_TARCING
    uint32_t const nodeCount = meshInstance->mesh.octreeNodeCount;

    for (uint32_t nodeId = 0; nodeId < nodeCount; nodeId++)
    {
        __global common_mesh_octree_node_t const * const node = meshRootNode + nodeId;
        uint32_t const primEnd = node->primFirst + node->primCount;

        for (uint32_t primId = node->primFirst; primId < primEnd; primId++)
        {
            primitive_intersection(sampleCx, meshPrimitives + primId);
        }
    }
#else // _CL_NO_OCTREE_RAY_TARCING
    mesh_octree_intersection(
        sampleCx,
        meshInstance,
        meshPrimitives,
        meshRootNode
    );
#endif // _CL_NO_OCTREE_RAY_TARCING
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
