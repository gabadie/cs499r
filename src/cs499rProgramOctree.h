
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

#if CS499R_CONFIG_ENABLE_OCTREE_NODE_CACHING
# if !CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP
#  error "CS499R_CONFIG_ENABLE_OCTREE_NODE_CACHING requires CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP"
# endif

    uint32_t subnodeOffsets[8];
    uint32_t primFirst;
    uint32_t primCount;
    uint8_t subnodeCount;
    uint8_t subnodeMask;

#else
    uint32_t nodeGlobalId;

#endif //CS499R_CONFIG_ENABLE_OCTREE_NODE_CACHING

    uint32_t subnodeAccessId;

} octree_stack_t;


// ----------------------------------------------------------------------------- SUBNODE ACCESS LISTS & REORDERING
#if CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING || CS499R_CONFIG_OCTREE_ACCESS_LISTS

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


#if CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_NO_ACCESS_LISTS
/*
 * This array is the sub-node access order depending on the ray direction.
 *
 * Generated by: python ../script/octree_subnode_order.py --direction-access-lists
 *
 *  uint32_t nodeSubnodeAccessList =
 *      kOctreeDirectionAccessLists[rayDirectionId];
 */
__constant uint32_t const kOctreeDirectionAccessLists[] = {
    0x01234567,
    0x10325476,
    0x23016745,
    0x32107654,
    0x45670123,
    0x54761032,
    0x67452301,
    0x76543210,
};

inline
uint32_t
octree_subnode_order(float32x3_t const rayDirection)
{
    uint32_t const directionId = octree_direction_id(rayDirection);

    return kOctreeDirectionAccessLists[directionId];
}


#elif CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_MASK_ACCESS_LISTS
/*
 * This array is the node's sub-node access list depending on the
 * ray's direction and the node's sub-nodes mask.
 *
 * Generated by: python ../scripts/octree_subnode_order.py --subnode-access-lists
 *
 *  uint32_t nodeSubnodeAccessList =
 *      kOctreeSubnodeMaskAccessLists[rayDirectionId * 256 + nodeSubnodeMask];
 */
__constant uint32_t const kOctreeSubnodeMaskAccessLists[256 * 8] = {
    // ray direction 0
    0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000002, 0x00000002, 0x00000012, 0x00000012,
    0x00000003, 0x00000003, 0x00000013, 0x00000013, 0x00000023, 0x00000023, 0x00000123, 0x00000123,
    0x00000004, 0x00000004, 0x00000014, 0x00000014, 0x00000024, 0x00000024, 0x00000124, 0x00000124,
    0x00000034, 0x00000034, 0x00000134, 0x00000134, 0x00000234, 0x00000234, 0x00001234, 0x00001234,
    0x00000005, 0x00000005, 0x00000015, 0x00000015, 0x00000025, 0x00000025, 0x00000125, 0x00000125,
    0x00000035, 0x00000035, 0x00000135, 0x00000135, 0x00000235, 0x00000235, 0x00001235, 0x00001235,
    0x00000045, 0x00000045, 0x00000145, 0x00000145, 0x00000245, 0x00000245, 0x00001245, 0x00001245,
    0x00000345, 0x00000345, 0x00001345, 0x00001345, 0x00002345, 0x00002345, 0x00012345, 0x00012345,
    0x00000006, 0x00000006, 0x00000016, 0x00000016, 0x00000026, 0x00000026, 0x00000126, 0x00000126,
    0x00000036, 0x00000036, 0x00000136, 0x00000136, 0x00000236, 0x00000236, 0x00001236, 0x00001236,
    0x00000046, 0x00000046, 0x00000146, 0x00000146, 0x00000246, 0x00000246, 0x00001246, 0x00001246,
    0x00000346, 0x00000346, 0x00001346, 0x00001346, 0x00002346, 0x00002346, 0x00012346, 0x00012346,
    0x00000056, 0x00000056, 0x00000156, 0x00000156, 0x00000256, 0x00000256, 0x00001256, 0x00001256,
    0x00000356, 0x00000356, 0x00001356, 0x00001356, 0x00002356, 0x00002356, 0x00012356, 0x00012356,
    0x00000456, 0x00000456, 0x00001456, 0x00001456, 0x00002456, 0x00002456, 0x00012456, 0x00012456,
    0x00003456, 0x00003456, 0x00013456, 0x00013456, 0x00023456, 0x00023456, 0x00123456, 0x00123456,
    0x00000007, 0x00000007, 0x00000017, 0x00000017, 0x00000027, 0x00000027, 0x00000127, 0x00000127,
    0x00000037, 0x00000037, 0x00000137, 0x00000137, 0x00000237, 0x00000237, 0x00001237, 0x00001237,
    0x00000047, 0x00000047, 0x00000147, 0x00000147, 0x00000247, 0x00000247, 0x00001247, 0x00001247,
    0x00000347, 0x00000347, 0x00001347, 0x00001347, 0x00002347, 0x00002347, 0x00012347, 0x00012347,
    0x00000057, 0x00000057, 0x00000157, 0x00000157, 0x00000257, 0x00000257, 0x00001257, 0x00001257,
    0x00000357, 0x00000357, 0x00001357, 0x00001357, 0x00002357, 0x00002357, 0x00012357, 0x00012357,
    0x00000457, 0x00000457, 0x00001457, 0x00001457, 0x00002457, 0x00002457, 0x00012457, 0x00012457,
    0x00003457, 0x00003457, 0x00013457, 0x00013457, 0x00023457, 0x00023457, 0x00123457, 0x00123457,
    0x00000067, 0x00000067, 0x00000167, 0x00000167, 0x00000267, 0x00000267, 0x00001267, 0x00001267,
    0x00000367, 0x00000367, 0x00001367, 0x00001367, 0x00002367, 0x00002367, 0x00012367, 0x00012367,
    0x00000467, 0x00000467, 0x00001467, 0x00001467, 0x00002467, 0x00002467, 0x00012467, 0x00012467,
    0x00003467, 0x00003467, 0x00013467, 0x00013467, 0x00023467, 0x00023467, 0x00123467, 0x00123467,
    0x00000567, 0x00000567, 0x00001567, 0x00001567, 0x00002567, 0x00002567, 0x00012567, 0x00012567,
    0x00003567, 0x00003567, 0x00013567, 0x00013567, 0x00023567, 0x00023567, 0x00123567, 0x00123567,
    0x00004567, 0x00004567, 0x00014567, 0x00014567, 0x00024567, 0x00024567, 0x00124567, 0x00124567,
    0x00034567, 0x00034567, 0x00134567, 0x00134567, 0x00234567, 0x00234567, 0x01234567, 0x01234567,
    // ray direction 1
    0x00000000, 0x00000000, 0x00000001, 0x00000010, 0x00000002, 0x00000002, 0x00000012, 0x00000102,
    0x00000003, 0x00000003, 0x00000013, 0x00000103, 0x00000032, 0x00000032, 0x00000132, 0x00001032,
    0x00000004, 0x00000004, 0x00000014, 0x00000104, 0x00000024, 0x00000024, 0x00000124, 0x00001024,
    0x00000034, 0x00000034, 0x00000134, 0x00001034, 0x00000324, 0x00000324, 0x00001324, 0x00010324,
    0x00000005, 0x00000005, 0x00000015, 0x00000105, 0x00000025, 0x00000025, 0x00000125, 0x00001025,
    0x00000035, 0x00000035, 0x00000135, 0x00001035, 0x00000325, 0x00000325, 0x00001325, 0x00010325,
    0x00000054, 0x00000054, 0x00000154, 0x00001054, 0x00000254, 0x00000254, 0x00001254, 0x00010254,
    0x00000354, 0x00000354, 0x00001354, 0x00010354, 0x00003254, 0x00003254, 0x00013254, 0x00103254,
    0x00000006, 0x00000006, 0x00000016, 0x00000106, 0x00000026, 0x00000026, 0x00000126, 0x00001026,
    0x00000036, 0x00000036, 0x00000136, 0x00001036, 0x00000326, 0x00000326, 0x00001326, 0x00010326,
    0x00000046, 0x00000046, 0x00000146, 0x00001046, 0x00000246, 0x00000246, 0x00001246, 0x00010246,
    0x00000346, 0x00000346, 0x00001346, 0x00010346, 0x00003246, 0x00003246, 0x00013246, 0x00103246,
    0x00000056, 0x00000056, 0x00000156, 0x00001056, 0x00000256, 0x00000256, 0x00001256, 0x00010256,
    0x00000356, 0x00000356, 0x00001356, 0x00010356, 0x00003256, 0x00003256, 0x00013256, 0x00103256,
    0x00000546, 0x00000546, 0x00001546, 0x00010546, 0x00002546, 0x00002546, 0x00012546, 0x00102546,
    0x00003546, 0x00003546, 0x00013546, 0x00103546, 0x00032546, 0x00032546, 0x00132546, 0x01032546,
    0x00000007, 0x00000007, 0x00000017, 0x00000107, 0x00000027, 0x00000027, 0x00000127, 0x00001027,
    0x00000037, 0x00000037, 0x00000137, 0x00001037, 0x00000327, 0x00000327, 0x00001327, 0x00010327,
    0x00000047, 0x00000047, 0x00000147, 0x00001047, 0x00000247, 0x00000247, 0x00001247, 0x00010247,
    0x00000347, 0x00000347, 0x00001347, 0x00010347, 0x00003247, 0x00003247, 0x00013247, 0x00103247,
    0x00000057, 0x00000057, 0x00000157, 0x00001057, 0x00000257, 0x00000257, 0x00001257, 0x00010257,
    0x00000357, 0x00000357, 0x00001357, 0x00010357, 0x00003257, 0x00003257, 0x00013257, 0x00103257,
    0x00000547, 0x00000547, 0x00001547, 0x00010547, 0x00002547, 0x00002547, 0x00012547, 0x00102547,
    0x00003547, 0x00003547, 0x00013547, 0x00103547, 0x00032547, 0x00032547, 0x00132547, 0x01032547,
    0x00000076, 0x00000076, 0x00000176, 0x00001076, 0x00000276, 0x00000276, 0x00001276, 0x00010276,
    0x00000376, 0x00000376, 0x00001376, 0x00010376, 0x00003276, 0x00003276, 0x00013276, 0x00103276,
    0x00000476, 0x00000476, 0x00001476, 0x00010476, 0x00002476, 0x00002476, 0x00012476, 0x00102476,
    0x00003476, 0x00003476, 0x00013476, 0x00103476, 0x00032476, 0x00032476, 0x00132476, 0x01032476,
    0x00000576, 0x00000576, 0x00001576, 0x00010576, 0x00002576, 0x00002576, 0x00012576, 0x00102576,
    0x00003576, 0x00003576, 0x00013576, 0x00103576, 0x00032576, 0x00032576, 0x00132576, 0x01032576,
    0x00005476, 0x00005476, 0x00015476, 0x00105476, 0x00025476, 0x00025476, 0x00125476, 0x01025476,
    0x00035476, 0x00035476, 0x00135476, 0x01035476, 0x00325476, 0x00325476, 0x01325476, 0x10325476,
    // ray direction 2
    0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000002, 0x00000020, 0x00000021, 0x00000201,
    0x00000003, 0x00000030, 0x00000031, 0x00000301, 0x00000023, 0x00000230, 0x00000231, 0x00002301,
    0x00000004, 0x00000004, 0x00000014, 0x00000014, 0x00000024, 0x00000204, 0x00000214, 0x00002014,
    0x00000034, 0x00000304, 0x00000314, 0x00003014, 0x00000234, 0x00002304, 0x00002314, 0x00023014,
    0x00000005, 0x00000005, 0x00000015, 0x00000015, 0x00000025, 0x00000205, 0x00000215, 0x00002015,
    0x00000035, 0x00000305, 0x00000315, 0x00003015, 0x00000235, 0x00002305, 0x00002315, 0x00023015,
    0x00000045, 0x00000045, 0x00000145, 0x00000145, 0x00000245, 0x00002045, 0x00002145, 0x00020145,
    0x00000345, 0x00003045, 0x00003145, 0x00030145, 0x00002345, 0x00023045, 0x00023145, 0x00230145,
    0x00000006, 0x00000006, 0x00000016, 0x00000016, 0x00000026, 0x00000206, 0x00000216, 0x00002016,
    0x00000036, 0x00000306, 0x00000316, 0x00003016, 0x00000236, 0x00002306, 0x00002316, 0x00023016,
    0x00000064, 0x00000064, 0x00000164, 0x00000164, 0x00000264, 0x00002064, 0x00002164, 0x00020164,
    0x00000364, 0x00003064, 0x00003164, 0x00030164, 0x00002364, 0x00023064, 0x00023164, 0x00230164,
    0x00000065, 0x00000065, 0x00000165, 0x00000165, 0x00000265, 0x00002065, 0x00002165, 0x00020165,
    0x00000365, 0x00003065, 0x00003165, 0x00030165, 0x00002365, 0x00023065, 0x00023165, 0x00230165,
    0x00000645, 0x00000645, 0x00001645, 0x00001645, 0x00002645, 0x00020645, 0x00021645, 0x00201645,
    0x00003645, 0x00030645, 0x00031645, 0x00301645, 0x00023645, 0x00230645, 0x00231645, 0x02301645,
    0x00000007, 0x00000007, 0x00000017, 0x00000017, 0x00000027, 0x00000207, 0x00000217, 0x00002017,
    0x00000037, 0x00000307, 0x00000317, 0x00003017, 0x00000237, 0x00002307, 0x00002317, 0x00023017,
    0x00000074, 0x00000074, 0x00000174, 0x00000174, 0x00000274, 0x00002074, 0x00002174, 0x00020174,
    0x00000374, 0x00003074, 0x00003174, 0x00030174, 0x00002374, 0x00023074, 0x00023174, 0x00230174,
    0x00000075, 0x00000075, 0x00000175, 0x00000175, 0x00000275, 0x00002075, 0x00002175, 0x00020175,
    0x00000375, 0x00003075, 0x00003175, 0x00030175, 0x00002375, 0x00023075, 0x00023175, 0x00230175,
    0x00000745, 0x00000745, 0x00001745, 0x00001745, 0x00002745, 0x00020745, 0x00021745, 0x00201745,
    0x00003745, 0x00030745, 0x00031745, 0x00301745, 0x00023745, 0x00230745, 0x00231745, 0x02301745,
    0x00000067, 0x00000067, 0x00000167, 0x00000167, 0x00000267, 0x00002067, 0x00002167, 0x00020167,
    0x00000367, 0x00003067, 0x00003167, 0x00030167, 0x00002367, 0x00023067, 0x00023167, 0x00230167,
    0x00000674, 0x00000674, 0x00001674, 0x00001674, 0x00002674, 0x00020674, 0x00021674, 0x00201674,
    0x00003674, 0x00030674, 0x00031674, 0x00301674, 0x00023674, 0x00230674, 0x00231674, 0x02301674,
    0x00000675, 0x00000675, 0x00001675, 0x00001675, 0x00002675, 0x00020675, 0x00021675, 0x00201675,
    0x00003675, 0x00030675, 0x00031675, 0x00301675, 0x00023675, 0x00230675, 0x00231675, 0x02301675,
    0x00006745, 0x00006745, 0x00016745, 0x00016745, 0x00026745, 0x00206745, 0x00216745, 0x02016745,
    0x00036745, 0x00306745, 0x00316745, 0x03016745, 0x00236745, 0x02306745, 0x02316745, 0x23016745,
    // ray direction 3
    0x00000000, 0x00000000, 0x00000001, 0x00000010, 0x00000002, 0x00000020, 0x00000021, 0x00000210,
    0x00000003, 0x00000030, 0x00000031, 0x00000310, 0x00000032, 0x00000320, 0x00000321, 0x00003210,
    0x00000004, 0x00000004, 0x00000014, 0x00000104, 0x00000024, 0x00000204, 0x00000214, 0x00002104,
    0x00000034, 0x00000304, 0x00000314, 0x00003104, 0x00000324, 0x00003204, 0x00003214, 0x00032104,
    0x00000005, 0x00000005, 0x00000015, 0x00000105, 0x00000025, 0x00000205, 0x00000215, 0x00002105,
    0x00000035, 0x00000305, 0x00000315, 0x00003105, 0x00000325, 0x00003205, 0x00003215, 0x00032105,
    0x00000054, 0x00000054, 0x00000154, 0x00001054, 0x00000254, 0x00002054, 0x00002154, 0x00021054,
    0x00000354, 0x00003054, 0x00003154, 0x00031054, 0x00003254, 0x00032054, 0x00032154, 0x00321054,
    0x00000006, 0x00000006, 0x00000016, 0x00000106, 0x00000026, 0x00000206, 0x00000216, 0x00002106,
    0x00000036, 0x00000306, 0x00000316, 0x00003106, 0x00000326, 0x00003206, 0x00003216, 0x00032106,
    0x00000064, 0x00000064, 0x00000164, 0x00001064, 0x00000264, 0x00002064, 0x00002164, 0x00021064,
    0x00000364, 0x00003064, 0x00003164, 0x00031064, 0x00003264, 0x00032064, 0x00032164, 0x00321064,
    0x00000065, 0x00000065, 0x00000165, 0x00001065, 0x00000265, 0x00002065, 0x00002165, 0x00021065,
    0x00000365, 0x00003065, 0x00003165, 0x00031065, 0x00003265, 0x00032065, 0x00032165, 0x00321065,
    0x00000654, 0x00000654, 0x00001654, 0x00010654, 0x00002654, 0x00020654, 0x00021654, 0x00210654,
    0x00003654, 0x00030654, 0x00031654, 0x00310654, 0x00032654, 0x00320654, 0x00321654, 0x03210654,
    0x00000007, 0x00000007, 0x00000017, 0x00000107, 0x00000027, 0x00000207, 0x00000217, 0x00002107,
    0x00000037, 0x00000307, 0x00000317, 0x00003107, 0x00000327, 0x00003207, 0x00003217, 0x00032107,
    0x00000074, 0x00000074, 0x00000174, 0x00001074, 0x00000274, 0x00002074, 0x00002174, 0x00021074,
    0x00000374, 0x00003074, 0x00003174, 0x00031074, 0x00003274, 0x00032074, 0x00032174, 0x00321074,
    0x00000075, 0x00000075, 0x00000175, 0x00001075, 0x00000275, 0x00002075, 0x00002175, 0x00021075,
    0x00000375, 0x00003075, 0x00003175, 0x00031075, 0x00003275, 0x00032075, 0x00032175, 0x00321075,
    0x00000754, 0x00000754, 0x00001754, 0x00010754, 0x00002754, 0x00020754, 0x00021754, 0x00210754,
    0x00003754, 0x00030754, 0x00031754, 0x00310754, 0x00032754, 0x00320754, 0x00321754, 0x03210754,
    0x00000076, 0x00000076, 0x00000176, 0x00001076, 0x00000276, 0x00002076, 0x00002176, 0x00021076,
    0x00000376, 0x00003076, 0x00003176, 0x00031076, 0x00003276, 0x00032076, 0x00032176, 0x00321076,
    0x00000764, 0x00000764, 0x00001764, 0x00010764, 0x00002764, 0x00020764, 0x00021764, 0x00210764,
    0x00003764, 0x00030764, 0x00031764, 0x00310764, 0x00032764, 0x00320764, 0x00321764, 0x03210764,
    0x00000765, 0x00000765, 0x00001765, 0x00010765, 0x00002765, 0x00020765, 0x00021765, 0x00210765,
    0x00003765, 0x00030765, 0x00031765, 0x00310765, 0x00032765, 0x00320765, 0x00321765, 0x03210765,
    0x00007654, 0x00007654, 0x00017654, 0x00107654, 0x00027654, 0x00207654, 0x00217654, 0x02107654,
    0x00037654, 0x00307654, 0x00317654, 0x03107654, 0x00327654, 0x03207654, 0x03217654, 0x32107654,
    // ray direction 4
    0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000002, 0x00000002, 0x00000012, 0x00000012,
    0x00000003, 0x00000003, 0x00000013, 0x00000013, 0x00000023, 0x00000023, 0x00000123, 0x00000123,
    0x00000004, 0x00000040, 0x00000041, 0x00000401, 0x00000042, 0x00000402, 0x00000412, 0x00004012,
    0x00000043, 0x00000403, 0x00000413, 0x00004013, 0x00000423, 0x00004023, 0x00004123, 0x00040123,
    0x00000005, 0x00000050, 0x00000051, 0x00000501, 0x00000052, 0x00000502, 0x00000512, 0x00005012,
    0x00000053, 0x00000503, 0x00000513, 0x00005013, 0x00000523, 0x00005023, 0x00005123, 0x00050123,
    0x00000045, 0x00000450, 0x00000451, 0x00004501, 0x00000452, 0x00004502, 0x00004512, 0x00045012,
    0x00000453, 0x00004503, 0x00004513, 0x00045013, 0x00004523, 0x00045023, 0x00045123, 0x00450123,
    0x00000006, 0x00000060, 0x00000061, 0x00000601, 0x00000062, 0x00000602, 0x00000612, 0x00006012,
    0x00000063, 0x00000603, 0x00000613, 0x00006013, 0x00000623, 0x00006023, 0x00006123, 0x00060123,
    0x00000046, 0x00000460, 0x00000461, 0x00004601, 0x00000462, 0x00004602, 0x00004612, 0x00046012,
    0x00000463, 0x00004603, 0x00004613, 0x00046013, 0x00004623, 0x00046023, 0x00046123, 0x00460123,
    0x00000056, 0x00000560, 0x00000561, 0x00005601, 0x00000562, 0x00005602, 0x00005612, 0x00056012,
    0x00000563, 0x00005603, 0x00005613, 0x00056013, 0x00005623, 0x00056023, 0x00056123, 0x00560123,
    0x00000456, 0x00004560, 0x00004561, 0x00045601, 0x00004562, 0x00045602, 0x00045612, 0x00456012,
    0x00004563, 0x00045603, 0x00045613, 0x00456013, 0x00045623, 0x00456023, 0x00456123, 0x04560123,
    0x00000007, 0x00000070, 0x00000071, 0x00000701, 0x00000072, 0x00000702, 0x00000712, 0x00007012,
    0x00000073, 0x00000703, 0x00000713, 0x00007013, 0x00000723, 0x00007023, 0x00007123, 0x00070123,
    0x00000047, 0x00000470, 0x00000471, 0x00004701, 0x00000472, 0x00004702, 0x00004712, 0x00047012,
    0x00000473, 0x00004703, 0x00004713, 0x00047013, 0x00004723, 0x00047023, 0x00047123, 0x00470123,
    0x00000057, 0x00000570, 0x00000571, 0x00005701, 0x00000572, 0x00005702, 0x00005712, 0x00057012,
    0x00000573, 0x00005703, 0x00005713, 0x00057013, 0x00005723, 0x00057023, 0x00057123, 0x00570123,
    0x00000457, 0x00004570, 0x00004571, 0x00045701, 0x00004572, 0x00045702, 0x00045712, 0x00457012,
    0x00004573, 0x00045703, 0x00045713, 0x00457013, 0x00045723, 0x00457023, 0x00457123, 0x04570123,
    0x00000067, 0x00000670, 0x00000671, 0x00006701, 0x00000672, 0x00006702, 0x00006712, 0x00067012,
    0x00000673, 0x00006703, 0x00006713, 0x00067013, 0x00006723, 0x00067023, 0x00067123, 0x00670123,
    0x00000467, 0x00004670, 0x00004671, 0x00046701, 0x00004672, 0x00046702, 0x00046712, 0x00467012,
    0x00004673, 0x00046703, 0x00046713, 0x00467013, 0x00046723, 0x00467023, 0x00467123, 0x04670123,
    0x00000567, 0x00005670, 0x00005671, 0x00056701, 0x00005672, 0x00056702, 0x00056712, 0x00567012,
    0x00005673, 0x00056703, 0x00056713, 0x00567013, 0x00056723, 0x00567023, 0x00567123, 0x05670123,
    0x00004567, 0x00045670, 0x00045671, 0x00456701, 0x00045672, 0x00456702, 0x00456712, 0x04567012,
    0x00045673, 0x00456703, 0x00456713, 0x04567013, 0x00456723, 0x04567023, 0x04567123, 0x45670123,
    // ray direction 5
    0x00000000, 0x00000000, 0x00000001, 0x00000010, 0x00000002, 0x00000002, 0x00000012, 0x00000102,
    0x00000003, 0x00000003, 0x00000013, 0x00000103, 0x00000032, 0x00000032, 0x00000132, 0x00001032,
    0x00000004, 0x00000040, 0x00000041, 0x00000410, 0x00000042, 0x00000402, 0x00000412, 0x00004102,
    0x00000043, 0x00000403, 0x00000413, 0x00004103, 0x00000432, 0x00004032, 0x00004132, 0x00041032,
    0x00000005, 0x00000050, 0x00000051, 0x00000510, 0x00000052, 0x00000502, 0x00000512, 0x00005102,
    0x00000053, 0x00000503, 0x00000513, 0x00005103, 0x00000532, 0x00005032, 0x00005132, 0x00051032,
    0x00000054, 0x00000540, 0x00000541, 0x00005410, 0x00000542, 0x00005402, 0x00005412, 0x00054102,
    0x00000543, 0x00005403, 0x00005413, 0x00054103, 0x00005432, 0x00054032, 0x00054132, 0x00541032,
    0x00000006, 0x00000060, 0x00000061, 0x00000610, 0x00000062, 0x00000602, 0x00000612, 0x00006102,
    0x00000063, 0x00000603, 0x00000613, 0x00006103, 0x00000632, 0x00006032, 0x00006132, 0x00061032,
    0x00000046, 0x00000460, 0x00000461, 0x00004610, 0x00000462, 0x00004602, 0x00004612, 0x00046102,
    0x00000463, 0x00004603, 0x00004613, 0x00046103, 0x00004632, 0x00046032, 0x00046132, 0x00461032,
    0x00000056, 0x00000560, 0x00000561, 0x00005610, 0x00000562, 0x00005602, 0x00005612, 0x00056102,
    0x00000563, 0x00005603, 0x00005613, 0x00056103, 0x00005632, 0x00056032, 0x00056132, 0x00561032,
    0x00000546, 0x00005460, 0x00005461, 0x00054610, 0x00005462, 0x00054602, 0x00054612, 0x00546102,
    0x00005463, 0x00054603, 0x00054613, 0x00546103, 0x00054632, 0x00546032, 0x00546132, 0x05461032,
    0x00000007, 0x00000070, 0x00000071, 0x00000710, 0x00000072, 0x00000702, 0x00000712, 0x00007102,
    0x00000073, 0x00000703, 0x00000713, 0x00007103, 0x00000732, 0x00007032, 0x00007132, 0x00071032,
    0x00000047, 0x00000470, 0x00000471, 0x00004710, 0x00000472, 0x00004702, 0x00004712, 0x00047102,
    0x00000473, 0x00004703, 0x00004713, 0x00047103, 0x00004732, 0x00047032, 0x00047132, 0x00471032,
    0x00000057, 0x00000570, 0x00000571, 0x00005710, 0x00000572, 0x00005702, 0x00005712, 0x00057102,
    0x00000573, 0x00005703, 0x00005713, 0x00057103, 0x00005732, 0x00057032, 0x00057132, 0x00571032,
    0x00000547, 0x00005470, 0x00005471, 0x00054710, 0x00005472, 0x00054702, 0x00054712, 0x00547102,
    0x00005473, 0x00054703, 0x00054713, 0x00547103, 0x00054732, 0x00547032, 0x00547132, 0x05471032,
    0x00000076, 0x00000760, 0x00000761, 0x00007610, 0x00000762, 0x00007602, 0x00007612, 0x00076102,
    0x00000763, 0x00007603, 0x00007613, 0x00076103, 0x00007632, 0x00076032, 0x00076132, 0x00761032,
    0x00000476, 0x00004760, 0x00004761, 0x00047610, 0x00004762, 0x00047602, 0x00047612, 0x00476102,
    0x00004763, 0x00047603, 0x00047613, 0x00476103, 0x00047632, 0x00476032, 0x00476132, 0x04761032,
    0x00000576, 0x00005760, 0x00005761, 0x00057610, 0x00005762, 0x00057602, 0x00057612, 0x00576102,
    0x00005763, 0x00057603, 0x00057613, 0x00576103, 0x00057632, 0x00576032, 0x00576132, 0x05761032,
    0x00005476, 0x00054760, 0x00054761, 0x00547610, 0x00054762, 0x00547602, 0x00547612, 0x05476102,
    0x00054763, 0x00547603, 0x00547613, 0x05476103, 0x00547632, 0x05476032, 0x05476132, 0x54761032,
    // ray direction 6
    0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000002, 0x00000020, 0x00000021, 0x00000201,
    0x00000003, 0x00000030, 0x00000031, 0x00000301, 0x00000023, 0x00000230, 0x00000231, 0x00002301,
    0x00000004, 0x00000040, 0x00000041, 0x00000401, 0x00000042, 0x00000420, 0x00000421, 0x00004201,
    0x00000043, 0x00000430, 0x00000431, 0x00004301, 0x00000423, 0x00004230, 0x00004231, 0x00042301,
    0x00000005, 0x00000050, 0x00000051, 0x00000501, 0x00000052, 0x00000520, 0x00000521, 0x00005201,
    0x00000053, 0x00000530, 0x00000531, 0x00005301, 0x00000523, 0x00005230, 0x00005231, 0x00052301,
    0x00000045, 0x00000450, 0x00000451, 0x00004501, 0x00000452, 0x00004520, 0x00004521, 0x00045201,
    0x00000453, 0x00004530, 0x00004531, 0x00045301, 0x00004523, 0x00045230, 0x00045231, 0x00452301,
    0x00000006, 0x00000060, 0x00000061, 0x00000601, 0x00000062, 0x00000620, 0x00000621, 0x00006201,
    0x00000063, 0x00000630, 0x00000631, 0x00006301, 0x00000623, 0x00006230, 0x00006231, 0x00062301,
    0x00000064, 0x00000640, 0x00000641, 0x00006401, 0x00000642, 0x00006420, 0x00006421, 0x00064201,
    0x00000643, 0x00006430, 0x00006431, 0x00064301, 0x00006423, 0x00064230, 0x00064231, 0x00642301,
    0x00000065, 0x00000650, 0x00000651, 0x00006501, 0x00000652, 0x00006520, 0x00006521, 0x00065201,
    0x00000653, 0x00006530, 0x00006531, 0x00065301, 0x00006523, 0x00065230, 0x00065231, 0x00652301,
    0x00000645, 0x00006450, 0x00006451, 0x00064501, 0x00006452, 0x00064520, 0x00064521, 0x00645201,
    0x00006453, 0x00064530, 0x00064531, 0x00645301, 0x00064523, 0x00645230, 0x00645231, 0x06452301,
    0x00000007, 0x00000070, 0x00000071, 0x00000701, 0x00000072, 0x00000720, 0x00000721, 0x00007201,
    0x00000073, 0x00000730, 0x00000731, 0x00007301, 0x00000723, 0x00007230, 0x00007231, 0x00072301,
    0x00000074, 0x00000740, 0x00000741, 0x00007401, 0x00000742, 0x00007420, 0x00007421, 0x00074201,
    0x00000743, 0x00007430, 0x00007431, 0x00074301, 0x00007423, 0x00074230, 0x00074231, 0x00742301,
    0x00000075, 0x00000750, 0x00000751, 0x00007501, 0x00000752, 0x00007520, 0x00007521, 0x00075201,
    0x00000753, 0x00007530, 0x00007531, 0x00075301, 0x00007523, 0x00075230, 0x00075231, 0x00752301,
    0x00000745, 0x00007450, 0x00007451, 0x00074501, 0x00007452, 0x00074520, 0x00074521, 0x00745201,
    0x00007453, 0x00074530, 0x00074531, 0x00745301, 0x00074523, 0x00745230, 0x00745231, 0x07452301,
    0x00000067, 0x00000670, 0x00000671, 0x00006701, 0x00000672, 0x00006720, 0x00006721, 0x00067201,
    0x00000673, 0x00006730, 0x00006731, 0x00067301, 0x00006723, 0x00067230, 0x00067231, 0x00672301,
    0x00000674, 0x00006740, 0x00006741, 0x00067401, 0x00006742, 0x00067420, 0x00067421, 0x00674201,
    0x00006743, 0x00067430, 0x00067431, 0x00674301, 0x00067423, 0x00674230, 0x00674231, 0x06742301,
    0x00000675, 0x00006750, 0x00006751, 0x00067501, 0x00006752, 0x00067520, 0x00067521, 0x00675201,
    0x00006753, 0x00067530, 0x00067531, 0x00675301, 0x00067523, 0x00675230, 0x00675231, 0x06752301,
    0x00006745, 0x00067450, 0x00067451, 0x00674501, 0x00067452, 0x00674520, 0x00674521, 0x06745201,
    0x00067453, 0x00674530, 0x00674531, 0x06745301, 0x00674523, 0x06745230, 0x06745231, 0x67452301,
    // ray direction 7
    0x00000000, 0x00000000, 0x00000001, 0x00000010, 0x00000002, 0x00000020, 0x00000021, 0x00000210,
    0x00000003, 0x00000030, 0x00000031, 0x00000310, 0x00000032, 0x00000320, 0x00000321, 0x00003210,
    0x00000004, 0x00000040, 0x00000041, 0x00000410, 0x00000042, 0x00000420, 0x00000421, 0x00004210,
    0x00000043, 0x00000430, 0x00000431, 0x00004310, 0x00000432, 0x00004320, 0x00004321, 0x00043210,
    0x00000005, 0x00000050, 0x00000051, 0x00000510, 0x00000052, 0x00000520, 0x00000521, 0x00005210,
    0x00000053, 0x00000530, 0x00000531, 0x00005310, 0x00000532, 0x00005320, 0x00005321, 0x00053210,
    0x00000054, 0x00000540, 0x00000541, 0x00005410, 0x00000542, 0x00005420, 0x00005421, 0x00054210,
    0x00000543, 0x00005430, 0x00005431, 0x00054310, 0x00005432, 0x00054320, 0x00054321, 0x00543210,
    0x00000006, 0x00000060, 0x00000061, 0x00000610, 0x00000062, 0x00000620, 0x00000621, 0x00006210,
    0x00000063, 0x00000630, 0x00000631, 0x00006310, 0x00000632, 0x00006320, 0x00006321, 0x00063210,
    0x00000064, 0x00000640, 0x00000641, 0x00006410, 0x00000642, 0x00006420, 0x00006421, 0x00064210,
    0x00000643, 0x00006430, 0x00006431, 0x00064310, 0x00006432, 0x00064320, 0x00064321, 0x00643210,
    0x00000065, 0x00000650, 0x00000651, 0x00006510, 0x00000652, 0x00006520, 0x00006521, 0x00065210,
    0x00000653, 0x00006530, 0x00006531, 0x00065310, 0x00006532, 0x00065320, 0x00065321, 0x00653210,
    0x00000654, 0x00006540, 0x00006541, 0x00065410, 0x00006542, 0x00065420, 0x00065421, 0x00654210,
    0x00006543, 0x00065430, 0x00065431, 0x00654310, 0x00065432, 0x00654320, 0x00654321, 0x06543210,
    0x00000007, 0x00000070, 0x00000071, 0x00000710, 0x00000072, 0x00000720, 0x00000721, 0x00007210,
    0x00000073, 0x00000730, 0x00000731, 0x00007310, 0x00000732, 0x00007320, 0x00007321, 0x00073210,
    0x00000074, 0x00000740, 0x00000741, 0x00007410, 0x00000742, 0x00007420, 0x00007421, 0x00074210,
    0x00000743, 0x00007430, 0x00007431, 0x00074310, 0x00007432, 0x00074320, 0x00074321, 0x00743210,
    0x00000075, 0x00000750, 0x00000751, 0x00007510, 0x00000752, 0x00007520, 0x00007521, 0x00075210,
    0x00000753, 0x00007530, 0x00007531, 0x00075310, 0x00007532, 0x00075320, 0x00075321, 0x00753210,
    0x00000754, 0x00007540, 0x00007541, 0x00075410, 0x00007542, 0x00075420, 0x00075421, 0x00754210,
    0x00007543, 0x00075430, 0x00075431, 0x00754310, 0x00075432, 0x00754320, 0x00754321, 0x07543210,
    0x00000076, 0x00000760, 0x00000761, 0x00007610, 0x00000762, 0x00007620, 0x00007621, 0x00076210,
    0x00000763, 0x00007630, 0x00007631, 0x00076310, 0x00007632, 0x00076320, 0x00076321, 0x00763210,
    0x00000764, 0x00007640, 0x00007641, 0x00076410, 0x00007642, 0x00076420, 0x00076421, 0x00764210,
    0x00007643, 0x00076430, 0x00076431, 0x00764310, 0x00076432, 0x00764320, 0x00764321, 0x07643210,
    0x00000765, 0x00007650, 0x00007651, 0x00076510, 0x00007652, 0x00076520, 0x00076521, 0x00765210,
    0x00007653, 0x00076530, 0x00076531, 0x00765310, 0x00076532, 0x00765320, 0x00765321, 0x07653210,
    0x00007654, 0x00076540, 0x00076541, 0x00765410, 0x00076542, 0x00765420, 0x00765421, 0x07654210,
    0x00076543, 0x00765430, 0x00765431, 0x07654310, 0x00765432, 0x07654320, 0x07654321, 0x76543210,
};

/*
 * Gets the octree node's subnode access id from the ray's direction id and the
 * node's subnode mask
 */
#define octree_subnode_mask_access_list(rayDirectionId, nodeSubnodeMask) \
    kOctreeSubnodeMaskAccessLists[(rayDirectionId) * 256 + (nodeSubnodeMask)]


#endif //CS499R_CONFIG_OCTREE_ACCESS_LISTS

#endif //CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING || CS499R_CONFIG_OCTREE_ACCESS_LISTS

// ----------------------------------------------------------------------------- NODE CACHING
#if CS499R_CONFIG_ENABLE_OCTREE_NODE_CACHING

inline
void
octree_node_cache(__private octree_stack_t * node_stack, __global common_octree_node_t const * node)
{
    for (uint32_t i = 0; i < 8; i++)
    {
        node_stack->subnodeOffsets[i] = node->subnodeOffsets[i];
    }

    node_stack->primFirst = node->primFirst;
    node_stack->primCount = node->primCount;
    node_stack->subnodeCount = node->subnodeCount;
    node_stack->subnodeMask = node->subnodeMask;
}


#endif //CS499R_CONFIG_ENABLE_OCTREE_NODE_CACHING

#endif // _CLH_CS499R_PROGRAM_OCTREE
