
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
    uint32_t subnodeAccessStack[kOctreeNodeStackSize];
    float32x4_t nodeInfosStack[kOctreeNodeStackSize];

    {
        nodeStack[0] = 0;
        subnodeAccessStack[0] = 0;
        nodeInfosStack[0].xyz = -(octree_tmplt_ray_origin(sampleCx));
        nodeInfosStack[0].w = (octree_tmplt_root_half_size());
    }

#if CS499R_CONFIG_OCTREE_ACCESS_LISTS != CS499R_CONFIG_OCTREE_NO_ACCESS_LISTS
    uint32_t const directionId = octree_direction_id(sampleCx->rayMeshDirection);

# if !CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
#  error "CS499R_CONFIG_OCTREE_ACCESS_LISTS requires CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING"
# endif

#elif CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
    uint32_t const subnodeAccessOrder = octree_subnode_order(sampleCx->rayMeshDirection);

#endif

    while (1)
    {
        sample_stats_name(sampleCx,OCTREE_LOOPS,++);

        uint32_t const subnodeAccessId = subnodeAccessStack[nodeStackSize - 1];

        __global common_octree_node_t const * const node = rootNode + nodeStack[nodeStackSize - 1];

#if CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_NODE_ACCESS_LISTS
        uint32_t const subnodeAccessOrder = node->subnodeAccessLists[directionId];

#elif CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_MASK_ACCESS_LISTS
        uint32_t const subnodeAccessOrder = octree_subnode_mask_access_list(
            directionId,
            node->subnodeMask
        );

#endif

#if CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
        uint32_t const subnodeId = octree_subnode_access(subnodeAccessOrder, subnodeAccessId);

#else
        uint32_t const subnodeId = subnodeAccessId;

#endif

#if CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_NO_ACCESS_LISTS
        if (subnodeAccessId == kOctreeNodeSubdivisonCount)

#else
        if (subnodeAccessId == node->subnodeCount)

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

        subnodeAccessStack[nodeStackSize - 1] = subnodeAccessId + 1;

#if 1 // TODO: !CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_NO_ACCESS_LISTS causes a GPU abort
        if (node->subnodeOffsets[subnodeId] == 0)
        {
            continue;
        }
#endif

        float32x4_t const nodeInfos = nodeInfosStack[nodeStackSize - 1];
        float32x4_t const subnodeInfos = octree_subnode_geometry(nodeInfos, subnodeId);

        if (
            !box_intersection_raw(
                sampleCx,
                subnodeInfos.xyz - subnodeInfos.w,
                subnodeInfos.xyz + 3.0f * subnodeInfos.w,
                (octree_tmplt_ray_direction_inverted(sampleCx))
            )
        )
        {
            continue;
        }

        // going down
        nodeStack[nodeStackSize] = node->subnodeOffsets[subnodeId];
        nodeInfosStack[nodeStackSize] = subnodeInfos;
        subnodeAccessStack[nodeStackSize] = 0;

        nodeStackSize++;

        sample_stats_name(sampleCx,OCTREE_NODE_BROWSING,++);
    }
}

#undef octree_tmplt_ray_origin
#undef octree_tmplt_ray_direction_inverted
#undef octree_tmplt_intersection
#undef octree_tmplt_root_half_size
#undef octree_tmplt_prim_intersection
