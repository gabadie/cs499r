
#ifndef _H_CS499R_COMMON_STRUCTS
#define _H_CS499R_COMMON_STRUCTS

#include "cs499rMath.hpp"

#ifndef __CS499R_OPENCL_FILE
/*
 * We are not preprocessing code for OpenCL, so we add the CS499R namespace
 */
namespace CS499R
{
#endif // __CS499R_OPENCL_FILE

    typedef
    struct __attribute__((aligned(16)))
    common_mat3x4_s
    {
        __attribute__((aligned(16))) float32x3_t x;
        __attribute__((aligned(16))) float32x3_t y;
        __attribute__((aligned(16))) float32x3_t z;
        __attribute__((aligned(16))) float32x3_t w;
    } common_mat3x4_t;

    typedef
    struct __attribute__((aligned(16)))
    common_mat2x2_s
    {
        __attribute__((aligned(8))) float32x2_t x;
        __attribute__((aligned(8))) float32x2_t y;
    } common_mat2x2_t;

    typedef
    struct __attribute__((aligned(16), packed))
    common_octree_node_s
    {
        // node's subnodes' offsets
        __attribute__((aligned(4))) uint32_t subnodeOffsets[8];

        // node's first primitive within the mesh's primitives' array
        __attribute__((aligned(4))) uint32_t primFirst;

        // node's primitives count
        __attribute__((aligned(4))) uint32_t primCount;

#if CS499R_CONFIG_OCTREE_ACCESS_LISTS != CS499R_CONFIG_OCTREE_NO_ACCESS_LISTS
# if CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_NODE_ACCESS_LISTS
        // sub-node access lists
        __attribute__((aligned(4))) uint32_t subnodeAccessLists[8];

# elif CS499R_CONFIG_OCTREE_ACCESS_LISTS == CS499R_CONFIG_OCTREE_MASK_ACCESS_LISTS
        // node's subnodes mask
        __attribute__((aligned(4))) uint8_t subnodeMask;

# endif //CS499R_CONFIG_OCTREE_ACCESS_LISTS

        // node's subnode count
        __attribute__((aligned(4))) uint8_t subnodeCount;
#endif // CS499R_CONFIG_OCTREE_ACCESS_LISTS != CS499R_CONFIG_OCTREE_NO_ACCESS_LISTS
    } common_octree_node_t;

    typedef
    struct __attribute__((aligned(16), packed))
    common_mesh_s
    {
        // mesh's first primitive
        uint32_t primFirst;

        // mesh's triangles count
        uint32_t primCount;

        // mesh's octree root's global id
        uint32_t octreeRootGlobalId;

        // mesh's octree node count
        uint32_t octreeNodeCount;

        // mesh's vertexs' upper bound
        //  w contains the mesh's octree root's half size
        float32x4_t vertexUpperBound;
    } common_mesh_t;

    typedef
    struct __attribute__((aligned(16)))
    common_mesh_instance_s
    {
        // matrix from the mesh space to the scene space
        common_mat3x4_t meshSceneMatrix;

        // matrix from the scene space to the mesh space
        common_mat3x4_t sceneMeshMatrix;

        // the mesh instance's colors
        __attribute__((aligned(16))) float32x3_t diffuseColor;
        __attribute__((aligned(16))) float32x3_t emitColor;

        // the mesh's informations
        common_mesh_t mesh;
    } common_mesh_instance_t;

    typedef
    struct __attribute__((aligned(16), packed))
    common_primitive_s
    {
        // primitive's origin vertex
        __attribute__((aligned(16))) float32x4_t v0;

        // primitive's edges
        __attribute__((aligned(16))) float32x4_t e0;
        __attribute__((aligned(16))) float32x4_t e1;

        // primitive precomputed normal
        // float32x3_t normal = float32x3_t(v0.w, e0.w, e1.w)
        //                    = normalize(cross(e0, e1))

        // precomputed matrix calculated from v0, e1
        // to compute the uv coordinates from (dot(AI,e0), dot(AI,e1))
        common_mat2x2_t uvMatrix;
    } common_primitive_t;

    typedef struct common_camera_s
    {
        __attribute__((aligned(16))) float32x3_t shotPosition;
        __attribute__((aligned(16))) float32x3_t shotBasisU;
        __attribute__((aligned(16))) float32x3_t shotBasisV;
        __attribute__((aligned(16))) float32x3_t focusPosition;
        __attribute__((aligned(16))) float32x3_t focusBasisU;
        __attribute__((aligned(16))) float32x3_t focusBasisV;
    } common_camera_t;

    typedef
    struct __attribute__((aligned(16), packed)) common_scene_s
    {
        // the max id of mesh instances in the scene
        __attribute__((aligned(16))) uint32_t meshInstanceMaxId;

        // the half size of the scene octree's root node
        __attribute__((aligned(16))) float32_t octreeRootHalfSize;

        // the scene octree's offset
        __attribute__((aligned(16))) float32x3_t octreeOffset;
    } common_scene_t;

    typedef
    struct __attribute__((aligned(8), packed)) common_pow_s
    {
        uint32_t value;
        uint32_t log;

#ifdef __cplusplus
        inline
        void
        operator = (uint32_t x)
        {
            value = x;
            log = ::log2(x);
        }
#endif
    } common_pow_t;

    typedef
    struct __attribute__((aligned(8), packed)) common_cpt_s
    {
        // coherency and kickoff tiles' constants
        __attribute__((aligned(16))) common_pow_t coherencyTileSize;
        __attribute__((aligned(16))) common_pow_t coherencyTilePerKickoffTileBorder;
        __attribute__((aligned(16))) common_pow_t groupPerCoherencyTile;
    } common_cpt_t;

    typedef
    struct __attribute__((aligned(8), packed)) common_icpt_s
    {
        // interleaved coherency and kickoff tiles' constants
        __attribute__((aligned(16))) common_pow_t coherencyTilePerKickoffTileBorder;
        __attribute__((aligned(16))) common_pow_t groupPerCoherencyTile;
    } common_icpt_t;

    typedef
    struct __attribute__((aligned(16), packed)) common_coherency_render_s
    {
        // the current virtual render's resolution (no matter if we are
        // rendering in a super tile or directly into the final target)
        __attribute__((aligned(16))) uint32x2_t virtualTargetResolution;

        // the render target's resolution
        __attribute__((aligned(16))) uint32x2_t targetResolution;

        // the render target's virtual offset in the virtual target (varying per super tile)
        __attribute__((aligned(16))) uint32x2_t targetVirtualOffset;

        // the render's subpixel count per pixel border
        __attribute__((aligned(16))) uint32_t subpixelPerPixelBorder;

        // the tile infos
        __attribute__((aligned(16))) uint32x2_t kickoffTilePos;
        __attribute__((aligned(16))) common_pow_t kickoffTileSize;

        // the kickoff's random seed offset
        __attribute__((aligned(16))) uint32_t kickoffSampleIterationCount;
        __attribute__((aligned(16))) uint32_t kickoffSampleRecursionCount;

        // the kickoff's sample group id
        __attribute__((aligned(16))) uint32_t passId;

        // the kickoff's sample group id
        __attribute__((aligned(16))) uint32_t passCount;

        // the subpixel pos in the pixel
        __attribute__((aligned(16))) uint32x2_t pixelSubpixelPos;

        // CBT algorithm input
        __attribute__((aligned(16))) common_cpt_t cpt;

        // ICBT
        __attribute__((aligned(16))) common_icpt_t icpt;
    } common_render_t;

    typedef
    struct __attribute__((aligned(16), packed)) common_coherency_context_s
    {
        // the shot's camera
        __attribute__((aligned(16))) common_camera_t camera;

        // the shot's scene
        __attribute__((aligned(16))) common_scene_t scene;

        // the shot's render info
        __attribute__((aligned(16))) common_render_t render;
    } common_render_context_t;

    typedef
    struct __attribute__((aligned(16), packed)) common_downscale_context_s
    {
        // the src render's resolution
        __attribute__((aligned(16))) uint32x2_t srcResolution;

        // the src tile infos
        __attribute__((aligned(16))) uint32x2_t srcTilePos;
        __attribute__((aligned(16))) common_pow_t srcTileSize;

        // the dest render's resolution
        __attribute__((aligned(16))) uint32x2_t destResolution;

        // the dest tile infos
        __attribute__((aligned(16))) uint32x2_t destTilePos;
        __attribute__((aligned(16))) common_pow_t destTileSize;

        // invocations parameters
        __attribute__((aligned(16))) common_pow_t downscaleFactor;

        // invocations parameters
        __attribute__((aligned(16))) float32_t multiplyFactor;
    } common_downscale_context_t;

#ifndef __CS499R_OPENCL_FILE
} // namespace CS499R
#endif // __CS499R_OPENCL_FILE


#endif // _H_CS499R_COMMON_STRUCTS
