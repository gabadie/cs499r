
#include "cs499rBenchmark.hpp"
#include "cs499rCamera.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rRenderProfiling.hpp"
#include "cs499rRenderState.hpp"
#include "cs499rRenderTarget.hpp"
#include "cs499rScene.hpp"
#include "cs499rSceneBuffer.hpp"


namespace CS499R
{

    void
    RenderState::shotScene(SceneBuffer const * sceneBuffer, Camera const * camera, RenderProfiling * outProfiling)
    {
        CS499R_ASSERT(sceneBuffer != nullptr);
        CS499R_ASSERT(camera != nullptr);
        CS499R_ASSERT(validateParams());
        CS499R_ASSERT(mRenderTarget != nullptr);
        CS499R_ASSERT(mRenderTarget->mRayTracer == sceneBuffer->mRayTracer);

        auto rayTracer = sceneBuffer->mRayTracer;

        timestamp_t kernelStart;
        timestamp_t kernelEnd;

        cl_int error = 0;
        cl_context context = rayTracer->mContext;
        cl_command_queue cmdQueue = rayTracer->mCmdQueue;
        cl_kernel kernel = rayTracer->mKernel.dispatch;

        float const aspectRatio = float(mRenderTarget->width()) / float(mRenderTarget->height());
        common_shot_context_t shotContext;

        { // init shot context
            camera->exportToShotCamera(&shotContext.camera, aspectRatio);

            shotContext.render.x = mRenderTarget->width();
            shotContext.render.y = mRenderTarget->height();
            shotContext.render.z = mPixelBorderSubdivisions;

            shotContext.triangleCount = sceneBuffer->mScene->mTriangles.size();
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
            error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &sceneBuffer->mBuffer.triangles);
            error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &mRenderTarget->mGpuBuffer);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        { // launch kernel
            size_t const kInvocationDims = 3;
            size_t const kThreadsPerTilesTarget = 2048;

            size_t const groupSize[kInvocationDims] = {
                mPixelBorderSubdivisions,
                mPixelBorderSubdivisions,
                mSamplesPerSubdivisions,
            };

            size_t maxWorkGroupSize = 0;
            clGetDeviceInfo(rayTracer->mDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, NULL);

            size2_t const globalSize(
                mRenderTarget->width() * mPixelBorderSubdivisions,
                mRenderTarget->height() * mPixelBorderSubdivisions
            );

            size_t const groupThreads = groupSize[0] * groupSize[1] * groupSize[2];
            size_t const groupPerTileBorder = (size_t) ceil(sqrt(
                float32_t(kThreadsPerTilesTarget) / float32_t(groupThreads)
            ));

            size_t const tileSize[kInvocationDims] = {
                groupSize[0] * groupPerTileBorder,
                groupSize[1] * groupPerTileBorder,
                groupSize[2]
            };

            size2_t const tileCount = (globalSize + (tileSize[0] - 1)) / tileSize[0];
            size2_t tileCoord;

            CS499R_ASSERT(groupSize[0] == groupSize[1]);
            CS499R_ASSERT(tileSize[0] == tileSize[1]);
            CS499R_ASSERT(groupThreads <= maxWorkGroupSize);

            if (outProfiling)
            {
                clFinish(cmdQueue);
                kernelStart = timestamp();
            }

            for (tileCoord.x = 0; tileCoord.x < tileCount.x; tileCoord.x++)
            {
                for (tileCoord.y = 0; tileCoord.y < tileCount.y; tileCoord.y++)
                {
                    size_t tileOffset[kInvocationDims] = {
                        tileSize[0] * tileCoord.x,
                        tileSize[1] * tileCoord.y,
                        0
                    };

                    error = clEnqueueNDRangeKernel(
                        cmdQueue, kernel,
                        kInvocationDims, tileOffset, tileSize, groupSize,
                        0, NULL, NULL
                    );

                    CS499R_ASSERT_NO_CL_ERROR(error);
                }
            }

            if (outProfiling)
            {
                clFinish(cmdQueue);
                kernelEnd = timestamp();
            }
        }

        if (outProfiling)
        {
            outProfiling->mCPUDuration = kernelEnd - kernelStart;
            outProfiling->mRays = mRenderTarget->width() * mRenderTarget->height() *
                mPixelBorderSubdivisions * mPixelBorderSubdivisions * mSamplesPerSubdivisions;
        }

        error = clReleaseMemObject(shotContextBuffer);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    RenderState::RenderState()
    {
        mPixelBorderSubdivisions = kDefaultPixelBorderSubdivisions;
        mSamplesPerSubdivisions = kDefaultSamplesPerSubdivisions;
        mRenderTarget = nullptr;

        CS499R_ASSERT(validateParams());
    }

    RenderState::~RenderState()
    {

    }

    bool
    RenderState::validateParams() const
    {
        CS499R_ASSERT(isPow2(mPixelBorderSubdivisions));
        CS499R_ASSERT(isPow2(mSamplesPerSubdivisions));

        return true;
    }

}
