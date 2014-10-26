
#ifndef _CLH_CS499R_PROGRAM_MESH
#define _CLH_CS499R_PROGRAM_MESH

#include "cs499rProgramConsts.h"
#include "cs499rProgramSampleContext.h"
#include "cs499rProgramIntersection.h"
#include "cs499rProgramOctree.h"


// ----------------------------------------------------------------------------- FUNCTIONS

inline
uint32_t
mesh_boundingbox_intersection(
    sample_context_t * const sampleCx,
    __global common_mesh_instance_t const * const meshInstance
)
{
    float32x3_t const OE = -sampleCx->rayMeshOrigin;
    float32x3_t const OG = OE + meshInstance->mesh.vertexUpperBound.xyz;

    return box_intersection(sampleCx, OE, OG);
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

#if CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
    uint32_t const subNodeAccessOrder = octree_sub_node_order(sampleCx->rayMeshDirection);
#endif

    while (1)
    {
        uint32_t const subNodeAccessId = subNodeAccessStack[nodeStackSize - 1];

#if CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
        uint32_t const subNodeId = (subNodeAccessOrder >> (subNodeAccessId * 4)) & kOctreeSubNodeMask;

#else
        uint32_t const subNodeId = subNodeAccessId;

#endif

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
        float32x4_t const subNodeInfos = octree_sub_node_infos(nodeInfos, subNodeId);

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
mesh_instance_prepare_frame(
    sample_context_t * const sampleCx,
    __global common_mesh_instance_t const * const meshInstance
)
{
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

    mesh_instance_prepare_frame(sampleCx, meshInstance);

#if CS499R_CONFIG_ENABLE_MESH_BOUNDING_BOX
    if (!mesh_boundingbox_intersection(sampleCx, meshInstance))
    {
        return;
    }
#endif

    __global common_primitive_t const * const meshPrimitives = primitives + meshInstance->mesh.primFirst;
    __global common_mesh_octree_node_t const * const meshRootNode = meshOctreeNodes + meshInstance->mesh.octreeRootGlobalId;

#if CS499R_CONFIG_ENABLE_MESH_OCTREE
    mesh_octree_intersection(
        sampleCx,
        meshInstance,
        meshPrimitives,
        meshRootNode
    );

#else
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

#endif // CS499R_CONFIG_ENABLE_MESH_OCTREE
}


#endif // _CLH_CS499R_PROGRAM_MESH
