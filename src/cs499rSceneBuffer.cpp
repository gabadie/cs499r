
#include <string.h>
#include "cs499rCamera.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rRenderTarget.hpp"
#include "cs499rScene.hpp"
#include "cs499rSceneBuffer.hpp"

namespace CS499R
{

    void
    SceneBuffer::render(RenderTarget * target, Camera const * camera) const
    {
        cl_int error = 0;
        cl_context context = mRayTracer->mContext;
        cl_command_queue cmdQueue = mRayTracer->mCmdQueue;
        cl_kernel kernel = mRayTracer->mKernel.dispatch;

        float const aspectRatio = float(target->width()) / float(target->height());
        common_shot_context_t shotContext;

        { // init shot context
            camera->exportToShotCamera(&shotContext.camera, aspectRatio);

            shotContext.triangleCount = mScene->mTriangles.size();
            shotContext.renderWidth = target->width();
            shotContext.renderHeight = target->height();
        }

        cl_mem shotContextBuffer = clCreateBuffer(context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(shotContext), &shotContext,
            &error
        );

        CS499R_ASSERT_NO_CL_ERROR(error);
        CS499R_ASSERT(shotContextBuffer != 0);

        { // kernel arguments
            error |= clSetKernelArg(kernel, 0, sizeof(cl_mem), &shotContextBuffer);
            error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mBuffer.triangles);
            error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &target->mGpuBuffer);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        { // launch kernel
            size_t const kRegionSize = 1;
            size_t const kInvocationDims = 2;

            size_t groupSize[kInvocationDims] = {
                kRegionSize,
                kRegionSize
            };
            size_t globalSize[kInvocationDims] = {
                ((target->width() + groupSize[0] - 1) / groupSize[0]) * groupSize[0],
                ((target->height() + groupSize[1] - 1) / groupSize[1]) * groupSize[1],
            };

            error |= clEnqueueNDRangeKernel(cmdQueue, kernel, kInvocationDims, NULL, globalSize, groupSize, 0, NULL, NULL);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        error |= clReleaseMemObject(shotContextBuffer);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

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
