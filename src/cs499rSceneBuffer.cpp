
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
