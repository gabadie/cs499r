
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
//inline
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
    uint32_t nodeStack[2 * kOctreeNodeStackSize];
    uint32_t subNodeAccessStack[2 * kOctreeNodeStackSize];
    float32x4_t nodeInfosStack[2 * kOctreeNodeStackSize];

    { // init Octrees' node stack
        nodeStack[0] = 0;
        subNodeAccessStack[0] = 0;
        nodeInfosStack[0].xyz = -(scene_octree_ray_origin(sampleCx, renderContext));
        nodeInfosStack[0].w = (scene_octree_root_half_size(renderContext));
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
    __global common_mesh_instance_t const * meshInstanceEnd;

    for(;;)
    {
#ifdef CS499R_STATS_OCTREE_LOOPS
        sampleCx->stats++;
#endif

        uint32_t const nodeStackId = nodeSceneStackSize + nodeMeshStackSize - 1;
        uint32_t const subNodeAccessId = subNodeAccessStack[nodeStackId];
        __global common_octree_node_t const * const node = rootNode + nodeStack[nodeStackId];
        uint32_t const subNodeAccessOrder = node->subNodeAccessLists[directionId];
        uint32_t const subNodeId = (subNodeAccessOrder >> (subNodeAccessId * 4)) & kOctreeSubNodeMask;

        if (subNodeAccessId != node->subNodeCount)
        {
            subNodeAccessStack[nodeStackId] = subNodeAccessId + 1;

            float32x4_t const nodeInfos = nodeInfosStack[nodeStackId];
            float32x4_t const subNodeInfos = octree_sub_node_infos(nodeInfos, subNodeId);

            if (
                !box_intersection_raw(
                    sampleCx,
                    subNodeInfos.xyz - subNodeInfos.w,
                    subNodeInfos.xyz + 3.0f * subNodeInfos.w,
                    directionInverted
                )
            )
            {
                continue;
            }

            uint32_t const nextNodeStackId = nodeStackId + 1;

            // going down
            nodeStack[nextNodeStackId] = octreeRootOffset + node->subNodeOffsets[subNodeId];
            nodeInfosStack[nextNodeStackId] = subNodeInfos;
            subNodeAccessStack[nextNodeStackId] = 0;

#ifdef CS499R_STATS_OCTREE_NODE_BROWSING
            //sampleCx->stats++;
#endif

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

            if (meshInstance < meshInstanceEnd)
            {
                /*
                 * There is still some meshes in the current octree node
                 */

                sampleCx->boundMeshInstance = meshInstance;

                mesh_instance_prepare_frame(sampleCx, meshInstance);

                // load mesh octree browsing
                octreeRootOffset = meshInstance->mesh.octreeRootGlobalId;
                octreePrimitiveOffset = meshInstance->mesh.primFirst;
                directionId = octree_direction_id(sampleCx->rayMeshDirection);
                directionInverted = sampleCx->rayMeshDirectionInverted;

                /*
                 * nextNodeStackId = nodeStackId because we replace the
                 * current mesh's root with the next mesh's root
                 */
                uint32_t const nextNodeStackId = nodeStackId;

                nodeStack[nextNodeStackId] = octreeRootOffset;
                subNodeAccessStack[nextNodeStackId] = 0;
                nodeInfosStack[nextNodeStackId].xyz = -(mesh_octree_ray_origin(sampleCx));
                nodeInfosStack[nextNodeStackId].w = (mesh_octree_root_half_size());
                nodeMeshStackSize = 1;

                continue;
            }

            /*
             * We have finished with the current octree node, we restor
             * scene octree browsing and we can go upward
             */

            // restore to scene octree browsing
            octreeRootOffset = 0;
            octreePrimitiveOffset = 0;
            directionId = sceneDirectionId;
            directionInverted = sampleCx->raySceneDirectionInverted;
        }
        else if (node->primCount != 0)
        {
            /*
             * We are about to go up in the scene octree, but this node has
             * mesh instances, so we start the iteration over them
             */
            meshInstance = meshInstances + node->primFirst;
            meshInstanceEnd = meshInstance + node->primCount;

            sampleCx->boundMeshInstance = meshInstance;

            mesh_instance_prepare_frame(sampleCx, meshInstance);

            // load mesh octree browsing
            octreeRootOffset = meshInstance->mesh.octreeRootGlobalId;
            octreePrimitiveOffset = meshInstance->mesh.primFirst;
            directionId = octree_direction_id(sampleCx->rayMeshDirection);
            directionInverted = sampleCx->rayMeshDirectionInverted;

            // set up octree stack
            uint32_t const nextNodeStackId = nodeStackId + 1;

            nodeStack[nextNodeStackId] = octreeRootOffset;
            subNodeAccessStack[nextNodeStackId] = 0;
            nodeInfosStack[nextNodeStackId].xyz = -(mesh_octree_ray_origin(sampleCx));
            nodeInfosStack[nextNodeStackId].w = (mesh_octree_root_half_size());
            nodeMeshStackSize = 1;

            continue;
        }

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
