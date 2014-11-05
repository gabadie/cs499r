
#ifndef _CLH_CS499R_PROGRAM_MESH
#define _CLH_CS499R_PROGRAM_MESH

#include "cs499rProgramConsts.h"
#include "cs499rProgramSampleContext.h"
#include "cs499rProgramIntersection.h"


// ----------------------------------------------------------------------------- MACROS

#define mesh_octree_ray_origin(sampleCx) \
    sampleCx->rayMeshOrigin

#define mesh_octree_root_half_size() \
    meshInstance->mesh.vertexUpperBound.w



// ----------------------------------------------------------------------------- OCTREE TEMPLATE

#define octree_tmplt_intersection(sampleCx, rootNode) \
    mesh_octree_intersection( \
        sampleCx, \
        __global common_mesh_instance_t const * const meshInstance, \
        __global common_primitive_t const * const meshPrimitives, \
        rootNode \
    )

#define octree_tmplt_ray_origin(sampleCx) \
    mesh_octree_ray_origin(sampleCx)

#define octree_tmplt_ray_direction_inverted(sampleCx) \
    sampleCx->rayMeshDirectionInverted

#define octree_tmplt_root_half_size() \
    mesh_octree_root_half_size()

#define octree_tmplt_prim_intersection(sampleCx, primId) \
    primitive_intersection(sampleCx, meshPrimitives + (primId))

#include "cs499rProgramOctree.tmplt.h"


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
    __global common_octree_node_t const * octreeNodes,
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
    __global common_octree_node_t const * const meshRootNode = octreeNodes + meshInstance->mesh.octreeRootGlobalId;

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
