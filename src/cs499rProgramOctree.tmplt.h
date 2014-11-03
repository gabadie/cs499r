
#include "cs499rProgramOctree.h"


#ifndef __CS499R_OPENCL_FILE

#define octree_tmplt_intersection(sampleCx, rootNode) \
    mesh_octree_intersection( \
        sampleCx, \
        rootNode \
    )

#define octree_tmplt_ray_origin(sampleCx) ((float32x3_t)(0.0f))
#define octree_tmplt_ray_direction_inverted(sampleCx) ((float32x3_t)(1.0f))
#define octree_tmplt_root_half_size() 1.0f
#define octree_tmplt_prim_intersection(sampleCx, primId)

#endif


inline
void
octree_tmplt_intersection(
    sample_context_t * const sampleCx,
    __global common_octree_node_t const * const rootNode
)
{
    uint32_t nodeStackSize = 1;
    uint32_t nodeStack[kOctreeNodeStackSize];
    uint32_t subNodeAccessStack[kOctreeNodeStackSize];
    float32x4_t nodeInfosStack[kOctreeNodeStackSize];

    {
        nodeStack[0] = 0;
        subNodeAccessStack[0] = 0;
        nodeInfosStack[0].xyz = -(octree_tmplt_ray_origin(sampleCx));
        nodeInfosStack[0].w = (octree_tmplt_root_half_size());
    }

#if CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
    uint32_t const directionId = octree_direction_id(sampleCx->rayMeshDirection);

# if !CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
#  error "CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS requires CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING"
# endif

#elif CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
    uint32_t const subNodeAccessOrder = octree_sub_node_order(sampleCx->rayMeshDirection);

#endif

    while (1)
    {
#ifdef CS499R_STATS_MESH_OCTREE_LOOPS
        sampleCx->stats++;
#endif

        uint32_t const subNodeAccessId = subNodeAccessStack[nodeStackSize - 1];

        __global common_octree_node_t const * const node = rootNode + nodeStack[nodeStackSize - 1];

#if CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
        uint32_t const subNodeAccessOrder = node->subNodeAccessLists[directionId];
        //uint32_t const subNodeAccessOrder = kOctreeSubNodeAccessOrder[directionId];
#endif

#if CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
        uint32_t const subNodeId = (subNodeAccessOrder >> (subNodeAccessId * 4)) & kOctreeSubNodeMask;

#else
        uint32_t const subNodeId = subNodeAccessId;

#endif

#if CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
        if (subNodeAccessId == node->subNodeCount)

#else
        if (subNodeAccessId == kOctreeNodeSubdivisonCount)

#endif
        {
            uint32_t const primEnd = node->primFirst + node->primCount;

            for (uint32_t primId = node->primFirst; primId < primEnd; primId++)
            {
                octree_tmplt_prim_intersection(sampleCx, primId);
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

#if 1 // TODO: !CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS causes a GPU abort
        if (node->subNodeOffsets[subNodeId] == 0)
        {
            continue;
        }
#endif

        float32x4_t const nodeInfos = nodeInfosStack[nodeStackSize - 1];
        float32x4_t const subNodeInfos = octree_sub_node_infos(nodeInfos, subNodeId);

        if (
            !box_intersection_raw(
                sampleCx,
                subNodeInfos.xyz - subNodeInfos.w,
                subNodeInfos.xyz + 3.0f * subNodeInfos.w,
                (octree_tmplt_ray_direction_inverted(sampleCx))
            )
        )
        {
            continue;
        }

        // going down
        nodeStack[nodeStackSize] = node->subNodeOffsets[subNodeId];
        nodeInfosStack[nodeStackSize] = subNodeInfos;
        subNodeAccessStack[nodeStackSize] = 0;

        nodeStackSize++;
    }
}

#undef octree_tmplt_ray_origin
#undef octree_tmplt_ray_direction_inverted
#undef octree_tmplt_intersection
#undef octree_tmplt_root_half_size
#undef octree_tmplt_prim_intersection
