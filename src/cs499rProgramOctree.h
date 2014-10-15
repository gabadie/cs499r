
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


#endif // _CLH_CS499R_PROGRAM_OCTREE
