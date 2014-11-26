
#ifndef _CLH_CS499R_PROGRAM_OCTREE
#define _CLH_CS499R_PROGRAM_OCTREE

#include "cs499rProgramConsts.h"


// ----------------------------------------------------------------------------- FUNCTIONS

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
 * Computes subnode's geometry
 */
inline
float32x4_t
octree_subnode_geometry(float32x4_t const nodeInfos, uint32_t const subnodeId)
{
    return (
        nodeInfos.w * kOctreeSubnodeInfoOffset[subnodeId] +
        nodeInfos
    );
}

/*
 * Octree stack's structure
 */
typedef
struct octree_stack_s
{
    float32x4_t nodeGeometry;
    uint32_t nodeGlobalId;
    uint32_t subnodeAccessId;
} octree_stack_t;


// ----------------------------------------------------------------------------- SUBNODE ACCESS LISTS & REORDERING
#if CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING || CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS

inline
uint32_t
octree_direction_id(float32x3_t const rayDirection)
{
    return (
        (uint32_t)(rayDirection.x >= 0.0f) |
        ((uint32_t)(rayDirection.y >= 0.0f) << 1) |
        ((uint32_t)(rayDirection.z >= 0.0f) << 2)
    );
}

#if !CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
/*
 * This array is the sub-node access order depending on the ray direction.
 */
__constant uint32_t const kOctreeSubnodeAccessOrder[] = {
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
octree_subnode_order(float32x3_t const rayDirection)
{
    uint32_t const directionId = octree_direction_id(rayDirection);

    return kOctreeSubnodeAccessOrder[directionId];
}
#endif //!CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS

/*
 * kOctreeSubnodeIdStride is the subnode access id's stride in the access order
 *      Could be set to 3 (2^3 == 8), but since uint32_t ouly need to store 8
 *      distincts values, it is set to 4 (4 * 8 == 32) so that the
 *      multiplycation can be replaced by a shift left.
 */
#define kOctreeSubnodeIdStride 4
#define kOctreeSubnodeMask 0x7

/*
 * Gets the subnode's id at position i from the access order
 */
#define octree_subnode_access(subnodeAccessOrder, i) \
        (((subnodeAccessOrder) >> ((i) * kOctreeSubnodeIdStride)) & kOctreeSubnodeMask)

#endif //CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING || CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS


#endif // _CLH_CS499R_PROGRAM_OCTREE
