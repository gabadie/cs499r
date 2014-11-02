
#include <string.h>
#include "cs499rCompiledScene.hpp"
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

        createPrimitivesBuffer(compilationCtx);

        createMeshOctreeNodesBuffer(compilationCtx);

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
        cl_int error = 0;
        cl_context context = mRayTracer->mContext;

        size_t const instanceCount = mScene->mObjectsMap.meshInstances.size();
        auto const instanceArray = alloc<common_mesh_instance_t>(instanceCount + 1);

        { // init the anonymous mesh instance
            SceneMeshInstance::exportAnonymousToCommonMeshInstance(instanceArray + 0);
        }

        size_t instanceId = 1;

        for (auto it : mScene->mObjectsMap.meshInstances)
        {
            auto const sceneMeshInstance = it.second;
            auto const commonMesh = instanceArray + instanceId;

            sceneMeshInstance->exportToCommonMeshInstance(compilationCtx, commonMesh);

            instanceId++;
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
    CompiledScene::createMeshOctreeNodesBuffer(CompilationCtx & compilationCtx)
    {
        cl_int error = 0;

        { // alloc mBuffer.meshOctreeNodes
            cl_context const context = mRayTracer->mContext;
            size_t totalMeshOctreeNodeCount = 0;

            for (auto it : mScene->mObjectsMap.meshes)
            {
                auto sceneMesh = it.second;

                compilationCtx.meshOctreeRootGlobalId.insert({sceneMesh, totalMeshOctreeNodeCount});

                totalMeshOctreeNodeCount += sceneMesh->mOctreeNodeCount;
            }

            CS499R_ASSERT(totalMeshOctreeNodeCount != 0);

            mBuffer.meshOctreeNodes = clCreateBuffer(
                context, CL_MEM_READ_ONLY,
                totalMeshOctreeNodeCount * sizeof(common_octree_node_t),
                NULL,
                &error
            );

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        { // upload mBuffer.meshOctreeNodes's content
            cl_command_queue const cmdQueue = mRayTracer->mCmdQueue;
            size_t meshOctreeNodeOffset = 0;

            for (auto it : mScene->mObjectsMap.meshes)
            {
                auto sceneMesh = it.second;

                CS499R_ASSERT_ALIGNMENT(sceneMesh->mOctreeNodeArray);

                error |= clEnqueueWriteBuffer(
                    cmdQueue, mBuffer.meshOctreeNodes, CL_FALSE,
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
