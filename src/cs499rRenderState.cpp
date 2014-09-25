
#include "cs499rCamera.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rRenderState.hpp"
#include "cs499rRenderTarget.hpp"
#include "cs499rScene.hpp"
#include "cs499rSceneBuffer.hpp"


namespace CS499R
{

    void
    RenderState::shotScene(SceneBuffer const * sceneBuffer, Camera const * camera)
    {
        CS499R_ASSERT(sceneBuffer != nullptr);
        CS499R_ASSERT(camera != nullptr);
        CS499R_ASSERT(mRenderTarget != nullptr);
        CS499R_ASSERT(mRenderTarget->mRayTracer == sceneBuffer->mRayTracer);

        auto rayTracer = sceneBuffer->mRayTracer;

        cl_int error = 0;
        cl_context context = rayTracer->mContext;
        cl_command_queue cmdQueue = rayTracer->mCmdQueue;
        cl_kernel kernel = rayTracer->mKernel.dispatch;

        float const aspectRatio = float(mRenderTarget->width()) / float(mRenderTarget->height());
        common_shot_context_t shotContext;

        { // init shot context
            camera->exportToShotCamera(&shotContext.camera, aspectRatio);

            shotContext.triangleCount = sceneBuffer->mScene->mTriangles.size();
            shotContext.renderWidth = mRenderTarget->width();
            shotContext.renderHeight = mRenderTarget->height();
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
            size_t const kWarpSize = 32;
            size_t const kRegionSize = kWarpSize;
            size_t const kGroupSize = 1;
            size_t const kInvocationDims = 2;

            size_t groupSize[kInvocationDims] = {
                kGroupSize,
                kGroupSize,
            };

            size2_t regionCount(
                ((mRenderTarget->width() + kRegionSize - 1) / kRegionSize),
                ((mRenderTarget->height() + kRegionSize - 1) / kRegionSize)
            );

            size2_t regionPos;

            for (regionPos.x = 0; regionPos.x < regionCount.x; regionPos.x++)
            {
                for (regionPos.y = 0; regionPos.y < regionCount.y; regionPos.y++)
                {
                    size_t globalOffset[kInvocationDims] = {
                        kRegionSize * regionPos.x,
                        kRegionSize * regionPos.y,
                    };

                    size_t globalSize[kInvocationDims] = {
                        kRegionSize,
                        kRegionSize,
                    };

                    error = clEnqueueNDRangeKernel(
                        cmdQueue, kernel,
                        kInvocationDims, globalOffset, globalSize, groupSize,
                        0, NULL, NULL
                    );

                    CS499R_ASSERT_NO_CL_ERROR(error);
                }
            }
        }

        error |= clReleaseMemObject(shotContextBuffer);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    RenderState::RenderState()
    {
        mPixelBorderSubdivisions = 4;
        mSamplesPerSubdivisions = 16;
        mRenderTarget = nullptr;
    }

    RenderState::~RenderState()
    {

    }

}
