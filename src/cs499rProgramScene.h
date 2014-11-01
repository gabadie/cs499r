
#ifndef _CLH_CS499R_PROGRAM_SCENE
#define _CLH_CS499R_PROGRAM_SCENE

#include "cs499rProgramSampleContext.h"
#include "cs499rProgramMesh.h"


// ----------------------------------------------------------------------------- FUNCTIONS

inline
void
scene_intersection(
    sample_context_t * sampleCx,
    __global common_render_context_t const * shotCx,
    __global common_mesh_instance_t const * meshInstances,
    __global common_octree_node_t const * octreeNodes,
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
    for (uint32_t i = 1; i < shotCx->scene.meshInstanceMaxId; i++)
    {
        sampleCx->boundMeshInstance = meshInstances + i;

        mesh_instance_intersection(sampleCx, octreeNodes, primitives);
    }
}


#endif // _CLH_CS499R_PROGRAM_SCENE
