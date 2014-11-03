
#include <string.h>
#include "cs499rCompiledScene.hpp"
#include "cs499rOctree.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rScene.hpp"

namespace CS499R
{

    CompiledScene::CompiledScene(Scene const * scene, RayTracer const * rayTracer)
        : mScene(scene)
        , mRayTracer(rayTracer)
    {
        memset(&mBuffer, 0, sizeof(mBuffer));

        createBuffers();
    }

    CompiledScene::~CompiledScene()
    {
        releaseBuffers();
    }

    void
    CompiledScene::createBuffers()
    {
        CompilationCtx compilationCtx;

        createOctreeNodesBuffer(compilationCtx);

        createPrimitivesBuffer(compilationCtx);

        createMeshInstancesBuffer(compilationCtx);
    }

    void
    CompiledScene::createPrimitivesBuffer(CompilationCtx & compilationCtx)
    {
        cl_int error = 0;
        cl_context context = mRayTracer->mContext;
        cl_command_queue cmdQueue = mRayTracer->mCmdQueue;

        size_t totalPrimCount = 0;

        for (auto it : mScene->mObjectsMap.meshes)
        {
            auto sceneMesh = it.second;

            compilationCtx.meshPrimitivesGlobalOffsets.insert({sceneMesh, totalPrimCount});

            totalPrimCount += sceneMesh->mPrimitiveCount;
        }

        CS499R_ASSERT(totalPrimCount != 0);

        mBuffer.primitives = clCreateBuffer(
            context, CL_MEM_READ_ONLY,
            totalPrimCount * sizeof(common_primitive_t),
            NULL,
            &error
        );

        CS499R_ASSERT_NO_CL_ERROR(error);

        size_t meshPrimOffset = 0;

        for (auto it : mScene->mObjectsMap.meshes)
        {
            auto sceneMesh = it.second;

            CS499R_ASSERT_ALIGNMENT(sceneMesh->mPrimitiveArray);

            error |= clEnqueueWriteBuffer(
                cmdQueue, mBuffer.primitives, CL_FALSE,
                meshPrimOffset * sizeof(common_primitive_t),
                sceneMesh->mPrimitiveCount * sizeof(common_primitive_t),
                sceneMesh->mPrimitiveArray,
                0, NULL, NULL
            );

            CS499R_ASSERT_NO_CL_ERROR(error);

            meshPrimOffset += sceneMesh->mPrimitiveCount;
        }
    }

    void
    CompiledScene::createMeshInstancesBuffer(CompilationCtx const & compilationCtx)
    {
        CS499R_ASSERT(compilationCtx.meshInstanceOrderList);

        cl_int error = 0;
        cl_context context = mRayTracer->mContext;

        size_t const instanceCount = mScene->mObjectsMap.meshInstances.size();
        auto const instanceArray = alloc<common_mesh_instance_t>(instanceCount + 1);

        { // init the anonymous mesh instance
            SceneMeshInstance::exportAnonymousToCommonMeshInstance(instanceArray + 0);
        }

        for (size_t instanceId = 0; instanceId < instanceCount; instanceId++)
        {
            auto const sceneMeshInstance = compilationCtx.meshInstanceOrderList[instanceId];
            auto const commonMesh = instanceArray + (instanceId + 1);

            sceneMeshInstance->exportToCommonMeshInstance(compilationCtx, commonMesh);
        }

        mBuffer.meshInstances = clCreateBuffer(
            context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            (instanceCount + 1) * sizeof(instanceArray[0]),
            instanceArray,
            &error
        );

        free(instanceArray);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    void
    CompiledScene::createOctreeNodesBuffer(CompilationCtx & compilationCtx)
    {
        cl_int error = 0;
        cl_command_queue const cmdQueue = mRayTracer->mCmdQueue;

        // create scene's octree
        float32x3_t lowerBound;
        float32x3_t upperBound;

        mScene->computeBoundingBox(&lowerBound, &upperBound);

        Octree<SceneMeshInstance *> octree(lowerBound, upperBound);

        for (auto it : mScene->mObjectsMap.meshInstances)
        {
            auto const meshInstance = it.second;

            float32x3_t meshInstanceLowerBound;
            float32x3_t meshInstanceUpperBound;

            meshInstance->computeBoundingBox(
                &meshInstanceLowerBound,
                &meshInstanceUpperBound
            );

            octree.insert(
                meshInstance,
                (meshInstanceLowerBound + meshInstanceUpperBound) * 0.5f,
                max(meshInstanceUpperBound - meshInstanceLowerBound)
            );
        }

        octree.optimize();

        size_t const totalSceneOctreeNodeCount = octree.nodeCount();

        CS499R_ASSERT(totalSceneOctreeNodeCount != 0);

        { // alloc mBuffer.octreeNodes
            cl_context const context = mRayTracer->mContext;
            size_t totalMeshOctreeNodeCount = 0;

            for (auto it : mScene->mObjectsMap.meshes)
            {
                auto sceneMesh = it.second;

                compilationCtx.meshOctreeRootGlobalId.insert({
                    sceneMesh,
                    totalMeshOctreeNodeCount + totalSceneOctreeNodeCount
                });

                totalMeshOctreeNodeCount += sceneMesh->mOctreeNodeCount;
            }

            CS499R_ASSERT(totalMeshOctreeNodeCount != 0);

            mBuffer.octreeNodes = clCreateBuffer(
                context, CL_MEM_READ_ONLY,
                (totalSceneOctreeNodeCount + totalMeshOctreeNodeCount) * sizeof(common_octree_node_t),
                NULL,
                &error
            );

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        { // upload scene's octree nodes to mBuffer.octreeNodes
            auto const sceneOctreeCommonNodes = alloc<common_octree_node_t>(totalSceneOctreeNodeCount);
            compilationCtx.meshInstanceOrderList = alloc<SceneMeshInstance *>(mScene->mObjectsMap.meshInstances.size());

            CS499R_ASSERT_ALIGNMENT(sceneOctreeCommonNodes);

            octree.exportToCommonOctreeNodeArray(
                compilationCtx.meshInstanceOrderList,
                sceneOctreeCommonNodes
            );

            error = clEnqueueWriteBuffer(
                cmdQueue, mBuffer.octreeNodes, CL_FALSE,
                0,
                totalSceneOctreeNodeCount * sizeof(common_octree_node_t),
                sceneOctreeCommonNodes,
                0, NULL, NULL
            );

            CS499R_ASSERT_NO_CL_ERROR(error);

            free(sceneOctreeCommonNodes);
        }

        { // upload meshes' octree nodes to mBuffer.octreeNodes
            size_t meshOctreeNodeOffset = totalSceneOctreeNodeCount;

            for (auto it : mScene->mObjectsMap.meshes)
            {
                auto sceneMesh = it.second;

                CS499R_ASSERT_ALIGNMENT(sceneMesh->mOctreeNodeArray);

                error = clEnqueueWriteBuffer(
                    cmdQueue, mBuffer.octreeNodes, CL_FALSE,
                    meshOctreeNodeOffset * sizeof(common_octree_node_t),
                    sceneMesh->mOctreeNodeCount * sizeof(common_octree_node_t),
                    sceneMesh->mOctreeNodeArray,
                    0, NULL, NULL
                );

                CS499R_ASSERT_NO_CL_ERROR(error);

                meshOctreeNodeOffset += sceneMesh->mOctreeNodeCount;
            }
        }
    }

    void
    CompiledScene::releaseBuffers()
    {
        size_t const bufferArraySize = sizeof(mBuffer) / sizeof(cl_mem);
        cl_mem * bufferArray = (cl_mem *) &mBuffer;

        for (size_t i = 0; i < bufferArraySize; i++)
        {
            CS499R_ASSERT(bufferArray[i] != nullptr);

            clReleaseMemObject(bufferArray[i]);

            bufferArray[i] = 0;
        }
    }

}
