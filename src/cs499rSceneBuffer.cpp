
#include <string.h>
#include "cs499rRayTracer.hpp"
#include "cs499rScene.hpp"
#include "cs499rSceneBuffer.hpp"

namespace CS499R
{

    SceneBuffer::SceneBuffer(Scene const * scene, RayTracer const * rayTracer)
        : mScene(scene)
        , mRayTracer(rayTracer)
    {
        memset(&mBuffer, 0, sizeof(mBuffer));

        createBuffers();
    }

    SceneBuffer::~SceneBuffer()
    {
        releaseBuffers();
    }

    void
    SceneBuffer::createBuffers()
    {
        cl_int error = 0;
        cl_context context = mRayTracer->mContext;
        cl_command_queue cmdQueue = mRayTracer->mCmdQueue;

        std::map<SceneMesh *, size_t> meshOffsetMap;

        { // upload triangles
            mBuffer.triangles = clCreateBuffer(
                context,
                CL_MEM_READ_ONLY,
                sizeof(common_triangle_t) * mScene->mTriangles.size(),
                NULL,
                &error
            );

            CS499R_ASSERT(error == 0);
            CS499R_ASSERT(mBuffer.triangles != nullptr);

            error |= clEnqueueWriteBuffer(
                cmdQueue,
                mBuffer.triangles,
                CL_FALSE, 0,
                sizeof(common_triangle_t) * mScene->mTriangles.size(),
                &mScene->mTriangles[0],
                0, NULL, NULL
            );
        }

        { // upload primitives
            size_t totalPrimCount = 0;

            for (auto it : mScene->mObjectsMap.meshes)
            {
                auto sceneMesh = it.second;

                meshOffsetMap.insert({sceneMesh, totalPrimCount});

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

                error |= clEnqueueWriteBuffer(
                    cmdQueue, mBuffer.triangles, CL_FALSE,
                    meshPrimOffset * sizeof(common_primitive_t),
                    sceneMesh->mPrimitiveCount * sizeof(common_primitive_t),
                    sceneMesh->mPrimitiveArray,
                    0, NULL, NULL
                );

                CS499R_ASSERT_NO_CL_ERROR(error);

                meshPrimOffset += sceneMesh->mPrimitiveCount;
            }
        }

        { // upload instances
            size_t const instanceCount = mScene->mObjectsMap.meshInstances.size();
            size_t instanceId = 0;
            auto instanceArray = new common_mesh_instance_t[instanceCount];

            for (auto it : mScene->mObjectsMap.meshInstances)
            {
                auto sceneMeshInstance = it.second;
                auto sceneMesh = it.second->mSceneMesh;
                auto sceneMeshPrimFirst = meshOffsetMap.find(sceneMesh)->second;
                auto commonMesh = instanceArray + instanceId;

                commonMesh->meshSceneMatrix.c[0] = sceneMeshInstance->mMeshSceneMatrix.x;
                commonMesh->meshSceneMatrix.c[1] = sceneMeshInstance->mMeshSceneMatrix.y;
                commonMesh->meshSceneMatrix.c[2] = sceneMeshInstance->mMeshSceneMatrix.z;
                commonMesh->meshSceneMatrix.c[3] = sceneMeshInstance->mScenePosition;

                auto sceneMeshMatrix = transpose(sceneMeshInstance->mMeshSceneMatrix);

                commonMesh->sceneMeshMatrix.c[0] = sceneMeshMatrix.x;
                commonMesh->sceneMeshMatrix.c[1] = sceneMeshMatrix.y;
                commonMesh->sceneMeshMatrix.c[2] = sceneMeshMatrix.z;
                commonMesh->sceneMeshMatrix.c[3] = -dot(sceneMeshMatrix, sceneMeshInstance->mScenePosition);

                commonMesh->diffuseColor = sceneMeshInstance->mColorDiffuse;
                commonMesh->emitColor = sceneMeshInstance->mColorEmit;
                commonMesh->mesh.primFirst = sceneMeshPrimFirst;
                commonMesh->mesh.primCount = sceneMesh->mPrimitiveCount;
            }

            mBuffer.primitives = clCreateBuffer(
                context, CL_MEM_READ_ONLY,
                instanceCount * sizeof(common_mesh_instance_t),
                NULL,
                &error
            );

            delete [] instanceArray;

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        CS499R_ASSERT(error == 0);
    }

    void
    SceneBuffer::releaseBuffers()
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
