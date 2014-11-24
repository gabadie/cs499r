
#ifndef _CLH_CS499R_PROGRAM_SCENE
#define _CLH_CS499R_PROGRAM_SCENE

#include "cs499rProgramSampleContext.h"
#include "cs499rProgramMesh.h"


// ----------------------------------------------------------------------------- MACROS

#define scene_octree_ray_origin(sampleCx,renderContext) \
    (sampleCx->raySceneOrigin + renderContext->scene.octreeOffset)

#define scene_octree_root_half_size(renderContext) \
    renderContext->scene.octreeRootHalfSize



// ----------------------------------------------------------------------------- OCTREE TEMPLATE
#if CS499R_CONFIG_ENABLE_SCENE_OCTREE && !CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP

#define octree_tmplt_intersection(sampleCx, rootNode) \
    scene_octree_intersection( \
        sampleCx, \
        __global common_render_context_t const * const renderContext, \
        __global common_mesh_instance_t const * const meshInstances, \
        __global common_primitive_t const * const primitives, \
        rootNode \
    )

#define octree_tmplt_ray_origin(sampleCx) \
    scene_octree_ray_origin(sampleCx,renderContext)

#define octree_tmplt_ray_direction_inverted(sampleCx) \
    sampleCx->raySceneDirectionInverted

#define octree_tmplt_root_half_size() \
    scene_octree_root_half_size(renderContext)

#define octree_tmplt_prim_intersection(sampleCx, primId) \
    { \
        sampleCx->boundMeshInstance = meshInstances + (primId); \
        mesh_instance_intersection(sampleCx, rootNode, primitives); \
    }

#include "cs499rProgramOctree.tmplt.h"

#endif //CS499R_CONFIG_ENABLE_SCENE_OCTREE


// ----------------------------------------------------------------------------- FUNCTIONS

#if CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP

# if !CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
#  error "CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP requires CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS"
# endif

# if !CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
#  error "CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP requires CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING"
# endif


#include "cs499rProgramOctree.h"

/*
 * This compute the scene's octree intersection and meshes' octree intersection
 * in the same loop.
 */
inline
void
scene_octree_one_loop_intersection(
    sample_context_t * const sampleCx,
    __global common_render_context_t const * const renderContext,
    __global common_mesh_instance_t const * const meshInstances,
    __global common_primitive_t const * const primitives,
    __global common_octree_node_t const * const rootNode
)
{
    /*
     * Octrees' node stack
     */
    uint32_t nodeSceneStackSize = 1;
    uint32_t nodeMeshStackSize = 0;

    octree_stack_t stack[2 * kOctreeNodeStackSize];

    { // init Octrees' node stack
        stack[0].nodeGeometry.xyz = -(scene_octree_ray_origin(sampleCx, renderContext));
        stack[0].nodeGeometry.w = (scene_octree_root_half_size(renderContext));
        stack[0].nodeGlobalId = 0;
        stack[0].subnodeAccessId = 0;
    }

    // the scene's direction id
    uint32_t const sceneDirectionId = octree_direction_id(sampleCx->raySceneDirection);

    /*
     * The current octree information
     */
    uint32_t octreeRootOffset = 0; // the current octree's root it
    uint32_t octreePrimitiveOffset = 0;
    uint32_t directionId = sceneDirectionId;
    float32x3_t directionInverted = sampleCx->raySceneDirectionInverted;

    /*
     * The mesh iterations variables
     */
    __global common_mesh_instance_t const * meshInstance = meshInstances;
    __global common_mesh_instance_t const * meshInstanceEnd = meshInstance;

    for(;;)
    {
        sample_stats_name(sampleCx,OCTREE_LOOPS,++);

        octree_stack_t * const stackRaw = stack + nodeSceneStackSize + nodeMeshStackSize - 1;
        uint32_t const subnodeAccessId = stackRaw->subnodeAccessId;
        __global common_octree_node_t const * const node = rootNode + stackRaw->nodeGlobalId;

        if (subnodeAccessId != node->subnodeCount)
        {
            stackRaw->subnodeAccessId = subnodeAccessId + 1;

            uint32_t const subnodeAccessOrder = node->subnodeAccessLists[directionId];
            uint32_t const subnodeId = (subnodeAccessOrder >> (subnodeAccessId * 4)) & kOctreeSubNodeMask;
            float32x4_t const subnodeGeometry = octree_subnode_geometry(stackRaw->nodeGeometry, subnodeId);

            if (
                !box_intersection_raw(
                    sampleCx,
                    subnodeGeometry.xyz - subnodeGeometry.w,
                    subnodeGeometry.xyz + 3.0f * subnodeGeometry.w,
                    directionInverted
                )
            )
            {
                continue;
            }

            octree_stack_t * const nextStackRaw = stackRaw + 1;

            // going down
            nextStackRaw->nodeGeometry = subnodeGeometry;
            nextStackRaw->nodeGlobalId = octreeRootOffset + node->subnodeOffsets[subnodeId];
            nextStackRaw->subnodeAccessId = 0;

            sample_stats_name(sampleCx,OCTREE_NODE_BROWSING,++);

            if (nodeMeshStackSize != 0)
            {
                nodeMeshStackSize++;
            }
            else
            {
                nodeSceneStackSize++;
            }

            continue;
        }

        /*
         * going up
         */
        if (nodeMeshStackSize != 0)
        { // we are browsing in a mesh octree
            // processing mesh triangles
            uint32_t const primStart = octreePrimitiveOffset + node->primFirst;
            uint32_t const primEnd = primStart + node->primCount;

            for (uint32_t primId = primStart; primId < primEnd; primId++)
            {
                primitive_intersection(sampleCx, primitives + primId);
            }

            nodeMeshStackSize--;

            if (nodeMeshStackSize != 0)
            {
                /*
                 * We have not finished with that mesh
                 */
                continue;
            }

            /*
             * We have finished with that mesh
             */
            meshInstance++;
        }
        else
        {
            /*
             * We are about to go up in the scene octree, but this node has
             * mesh instances, so we start the iteration over them
             */
            meshInstance = meshInstances + node->primFirst;
            meshInstanceEnd = meshInstance + node->primCount;
        }

        // assert(nodeMeshStackSize == 0)

#if CS499R_CONFIG_ENABLE_MESH_BOUNDING_BOX
        if (meshInstance < meshInstanceEnd)
        {
            do
            {
                mesh_instance_prepare_frame(sampleCx, meshInstance);

                if (mesh_boundingbox_intersection(
                    sampleCx,
                    meshInstance
                ))
                {
                    break;
                }

                meshInstance++;
            }
            while (meshInstance < meshInstanceEnd);
#else
        {
#endif //CS499R_CONFIG_ENABLE_MESH_BOUNDING_BOX

            if (meshInstance < meshInstanceEnd)
            {
                sampleCx->boundMeshInstance = meshInstance;

#if !CS499R_CONFIG_ENABLE_MESH_BOUNDING_BOX
                mesh_instance_prepare_frame(sampleCx, meshInstance);
#endif

                // load mesh octree browsing
                octreeRootOffset = meshInstance->mesh.octreeRootGlobalId;
                octreePrimitiveOffset = meshInstance->mesh.primFirst;
                directionId = octree_direction_id(sampleCx->rayMeshDirection);
                directionInverted = sampleCx->rayMeshDirectionInverted;

                // set up octree stack
                octree_stack_t * const nextStackRaw = stack + nodeSceneStackSize;

                nextStackRaw->nodeGlobalId = octreeRootOffset;
                nextStackRaw->subnodeAccessId = 0;
                nextStackRaw->nodeGeometry.xyz = -(mesh_octree_ray_origin(sampleCx));
                nextStackRaw->nodeGeometry.w = (mesh_octree_root_half_size());
                nodeMeshStackSize = 1;

                sample_stats_name(sampleCx,OCTREE_MESH_BROWSING,++);

                continue;
            }
        }

        if (meshInstance == meshInstanceEnd)
        {
            /*
             * We have finished with the current octree node, we restor
             * scene octree browsing and we can go upward
             */

            // restore to scene octree browsing
            octreeRootOffset = 0;
            directionId = sceneDirectionId;
            directionInverted = sampleCx->raySceneDirectionInverted;
        }

        // assert(nodeMeshStackSize == 0)

        /*
         * We are going up in the scene octree right away
         */
        nodeSceneStackSize--;

        if (nodeSceneStackSize == 0)
        {
            // scene processing is done
            return;
        }
    }
}

#endif //CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP

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

#if CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP
    scene_octree_one_loop_intersection(
        sampleCx,
        renderContext,
        meshInstances,
        primitives,
        octreeNodes
    );

#elif CS499R_CONFIG_ENABLE_SCENE_OCTREE
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
