
#ifndef _CLH_CS499R_PROGRAM_SCENE
#define _CLH_CS499R_PROGRAM_SCENE

#include "cs499rProgramSampleContext.h"
#include "cs499rProgramMesh.h"


// ----------------------------------------------------------------------------- OCTREE TEMPLATE
#if CS499R_CONFIG_ENABLE_SCENE_OCTREE

#define octree_tmplt_intersection(sampleCx, rootNode) \
    scene_octree_intersection( \
        sampleCx, \
        __global common_render_context_t const * const renderContext, \
        __global common_mesh_instance_t const * const meshInstances, \
        __global common_primitive_t const * const primitives, \
        rootNode \
    )

#define octree_tmplt_ray_origin(sampleCx) \
    (sampleCx->raySceneOrigin + renderContext->scene.octreeOffset)

#define octree_tmplt_ray_direction_inverted(sampleCx) \
    sampleCx->raySceneDirectionInverted

#define octree_tmplt_root_half_size() \
    renderContext->scene.octreeRootHalfSize

#define octree_tmplt_prim_intersection(sampleCx, primId) \
    { \
        sampleCx->boundMeshInstance = meshInstances + (primId); \
        mesh_instance_intersection(sampleCx, rootNode, primitives); \
    }

#include "cs499rProgramOctree.tmplt.h"

#endif //CS499R_CONFIG_ENABLE_SCENE_OCTREE


// ----------------------------------------------------------------------------- FUNCTIONS

inline
void
scene_intersection(
    sample_context_t * const sampleCx,
    __global common_render_context_t const * const renderContext,
    __global common_mesh_instance_t const * const meshInstances,
    __global common_octree_node_t const * const octreeNodes,
    __global common_primitive_t const * const primitives
)
{
    sampleCx->rayInterDistance = INFINITY;

    /*
     * We sets up the intersection mesh as the anonymous mesh first
     */
    sampleCx->rayInterMeshInstance = meshInstances;

#if CS499R_CONFIG_ENABLE_SCENE_OCTREE
    scene_octree_intersection(
        sampleCx,
        renderContext,
        meshInstances,
        primitives,
        octreeNodes
    );
#else
    /*
     * i = 1 because we skip over the anonymous mesh instance
     */
    for (uint32_t i = 1; i < renderContext->scene.meshInstanceMaxId; i++)
    {
        sampleCx->boundMeshInstance = meshInstances + i;

        mesh_instance_intersection(sampleCx, octreeNodes, primitives);
    }
#endif
}


#endif // _CLH_CS499R_PROGRAM_SCENE
