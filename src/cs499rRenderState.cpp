
#include "cs499rBenchmark.hpp"
#include "cs499rCamera.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rRenderProfiling.hpp"
#include "cs499rRenderState.hpp"
#include "cs499rRenderTarget.hpp"
#include "cs499rScene.hpp"
#include "cs499rSceneBuffer.hpp"

#include <iostream>
#define show(x) \
    std::cout << #x << " = " << x << "\n"

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

        shotSceneCoherency(sceneBuffer, camera, outProfiling);
    }

    RenderState::RenderState()
    {
        mPixelBorderSubdivisions = kDefaultPixelBorderSubdivisions;
        mSamplesPerSubdivisions = kDefaultSamplesPerSubdivisions;
        mRecursionPerSample = kDefaultRecursionPerSample;
        mRayAlgorithm = kRayAlgorithmPathTracer;
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
        CS499R_ASSERT(mRecursionPerSample >= 1);
        CS499R_ASSERT(mRayAlgorithm < kRayAlgorithmCount);

        return true;
    }

    void
    RenderState::clearRenderTarget()
    {
        uint8_t const pattern = 0;

        auto const rayTracer = mRenderTarget->mRayTracer;

        cl_int error = clEnqueueFillBuffer(
            rayTracer->mCmdQueue, mRenderTarget->mGpuBuffer,
            &pattern, sizeof(pattern),
            0, mRenderTarget->width() * mRenderTarget->height() * RenderTarget::kChanelCount * sizeof(float32_t),
            0, nullptr, nullptr
        );

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    void
    RenderState::multiplyRenderTarget(float32_t multiplyFactor)
    {
        cl_int error = 0;

        auto const rayTracer = mRenderTarget->mRayTracer;

        cl_kernel const kernel = rayTracer->mProgram[RayTracer::kProgramTragetMultiply].kernel;

        { // kernel arguments
            error |= clSetKernelArg(kernel, 0, sizeof(float32_t), &multiplyFactor);
            error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mRenderTarget->mGpuBuffer);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        size_t const pixelCount = mRenderTarget->width() * mRenderTarget->height();

        error = clEnqueueNDRangeKernel(
            rayTracer->mCmdQueue, kernel,
            1, nullptr, &pixelCount, nullptr,
            0, nullptr,
            nullptr
        );
        CS499R_ASSERT_NO_CL_ERROR(error);

        clFinish(rayTracer->mCmdQueue);
    }

    void
    RenderState::shotSceneCoherency(SceneBuffer const * sceneBuffer, Camera const * camera, RenderProfiling * outProfiling)
    {
        size_t const kHostAheadCommandCount = 10;
        size_t const kMaxKickoffSampleIteration = 32;
        size_t const kPathTracerRandomPerRay = 2;
        size_t const kThreadsPerTilesTarget = 2048 * 8;

        struct kickoff_events_t
        {
            cl_event bufferWriteDone;
            cl_event kickoffDone;
        };

        struct kickoff_ctx_manager_t
        {
            cl_mem buffers[kHostAheadCommandCount];
            kickoff_events_t events[kHostAheadCommandCount];
        };

        auto const rayTracer = sceneBuffer->mRayTracer;

        timestamp_t kernelStart;
        timestamp_t kernelEnd;

        cl_int error = 0;
        cl_context const context = rayTracer->mContext;
        cl_command_queue const cmdQueue = rayTracer->mCmdQueue;
        cl_kernel const kernel = rayTracer->mProgram[mRayAlgorithm].kernel;

        size_t warpSize = 1;

        error =  clGetKernelWorkGroupInfo(
            kernel, rayTracer->mDeviceId, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
            sizeof(warpSize), &warpSize, nullptr
        );
        CS499R_ASSERT_NO_CL_ERROR(error);

        bool const debugKernel = mRayAlgorithm != kRayAlgorithmPathTracer;
        size_t const pixelBorderSubdivisions = debugKernel ? 1 : mPixelBorderSubdivisions;
        size_t const samplesPerSubdivisions = debugKernel ? 1 : mSamplesPerSubdivisions;
        size_t const recursionPerSample = debugKernel ? 1 : mRecursionPerSample;

        size_t const pixelSubdivisions = pixelBorderSubdivisions * pixelBorderSubdivisions;
        size_t const pixelSampleCount = pixelSubdivisions * samplesPerSubdivisions;
        size_t const pixelRayCount = pixelSampleCount * recursionPerSample;

        size_t const kickoffSampleIterationCount = min(samplesPerSubdivisions, kMaxKickoffSampleIteration);
        size_t const kickoffInvocationCount = samplesPerSubdivisions / kickoffSampleIterationCount;

        size_t const coherencyTileSize = 8;
        size_t const kickoffTileSize = sqrt(kThreadsPerTilesTarget);
        size_t const kickoffTileGlobalSize = kickoffTileSize * kickoffTileSize;
        size_t const kickoffTileLocalSize = ceilSquarePow2(warpSize);
        size2_t const kickoffTileGrid = size2_t(
            (mRenderTarget->width() + kickoffTileSize - 1) / kickoffTileSize,
            (mRenderTarget->height() + kickoffTileSize - 1) / kickoffTileSize
        );
        size_t const kickoffTileCount = kickoffTileGrid.x * kickoffTileGrid.y;

        { // validation
            CS499R_ASSERT((kickoffTileGlobalSize % kickoffTileLocalSize) == 0);

            CS499R_ASSERT((kickoffTileLocalSize % coherencyTileSize) == 0);
            CS499R_ASSERT((kickoffTileSize % coherencyTileSize) == 0);
        }

        common_render_context_t templateCtx;

        { // init render context template
            auto const aspectRatio = float32_t(mRenderTarget->width()) / float32_t(mRenderTarget->height());

            camera->exportToShotCamera(&templateCtx.camera, aspectRatio);

            // +1 because of the anonymous mesh
            templateCtx.scene.meshInstanceMaxId = sceneBuffer->mScene->mObjectsMap.meshInstances.size() + 1;

            templateCtx.render.resolution.x = mRenderTarget->width();
            templateCtx.render.resolution.y = mRenderTarget->height();
            templateCtx.render.subpixelPerPixelBorder = pixelBorderSubdivisions;

            templateCtx.render.warpSize = warpSize;

            templateCtx.render.kickoffSampleIterationCount = kickoffSampleIterationCount;
            templateCtx.render.kickoffSampleRecursionCount = recursionPerSample - 1;
        }

        { // render context's CBT init
            templateCtx.render.cbt.kickoffTileSize = kickoffTileSize;
            templateCtx.render.cbt.coherencyTileSize = coherencyTileSize;
            templateCtx.render.cbt.coherencyTilePerKickoffTileBorder = (
                kickoffTileSize / coherencyTileSize
            );
        }

        auto const kickoffCtxCircularArray = alloc<common_render_context_t>(kickoffTileCount * kHostAheadCommandCount);
        auto const kickoffCtxManagers = alloc<kickoff_ctx_manager_t>(kickoffTileCount);

        // init render context circular pass 0
        for (size_t kickoffTileIdY = 0; kickoffTileIdY < kickoffTileGrid.y; kickoffTileIdY++)
        {
            for (size_t kickoffTileIdX = 0; kickoffTileIdX < kickoffTileGrid.x; kickoffTileIdX++)
            {
                size_t const kickoffTileId = kickoffTileIdX + kickoffTileIdY * kickoffTileGrid.x;
                auto const kickoffCtx = kickoffCtxCircularArray + kickoffTileId;

                memcpy(kickoffCtx, &templateCtx, sizeof(templateCtx));

                kickoffCtx->render.kickoffTilePos.x = kickoffTileSize * kickoffTileIdX;
                kickoffCtx->render.kickoffTilePos.y = kickoffTileSize * kickoffTileIdY;

                for (size_t circularPassId = 0; circularPassId < kHostAheadCommandCount; circularPassId++)
                {
                    kickoffCtxManagers[kickoffTileId].buffers[circularPassId] = clCreateBuffer(context,
                        CL_MEM_READ_ONLY,
                        sizeof(templateCtx), nullptr,
                        &error
                    );
                }

                CS499R_ASSERT_NO_CL_ERROR(error);
            }
        }

        // clones the render context circular pass 0 into others
        for (size_t circularPassId = 1; circularPassId < kHostAheadCommandCount; circularPassId++)
        {
            auto const kickoffCtxArray = kickoffCtxCircularArray + kickoffTileCount * circularPassId;

            for (size_t kickoffTileId = 0; kickoffTileId < kickoffTileCount; kickoffTileId++)
            {
                memcpy(
                    kickoffCtxArray + kickoffTileId,
                    kickoffCtxCircularArray + kickoffTileId,
                    sizeof(templateCtx)
                );
            }
        }

        { // kernel arguments
            error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &sceneBuffer->mBuffer.meshInstances);
            error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &sceneBuffer->mBuffer.meshOctreeNodes);
            error |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &sceneBuffer->mBuffer.primitives);
            error |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &mRenderTarget->mGpuBuffer);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        { // clears render target
            clearRenderTarget();
        }

        if (outProfiling)
        {
            clFinish(cmdQueue);
            kernelStart = timestamp();
        }

        size_t passId = 0;
        size_t const passCount = kickoffInvocationCount * pixelSubdivisions;

        // render loops
        for (size_t invocationId = 0; invocationId < kickoffInvocationCount; invocationId++)
        {
            for (size_t subPixelY = 0; subPixelY < pixelBorderSubdivisions; subPixelY++)
            {
                for (size_t subPixelX = 0; subPixelX < pixelBorderSubdivisions; subPixelX++)
                {
                    size_t const currentCircularPassId = passId % kHostAheadCommandCount;

                    auto const kickoffCtxArray = kickoffCtxCircularArray + kickoffTileCount * currentCircularPassId;

                    // upload render context buffers for this pass
                    for (size_t kickoffTileId = 0; kickoffTileId < kickoffTileCount; kickoffTileId++)
                    {
                        auto const kickoffCtxManager = kickoffCtxManagers + kickoffTileId;
                        auto const kickoffCtx = kickoffCtxArray + kickoffTileId;

                        auto const currentPassEvent = kickoffCtxManager->events + currentCircularPassId;

                        kickoffCtx->render.kickoffTileRandomSeedOffset = kPathTracerRandomPerRay * recursionPerSample * (
                            invocationId * pixelSubdivisions +
                            subPixelY * pixelBorderSubdivisions +
                            subPixelX
                        );
                        kickoffCtx->render.pixelSubpixelPos.x = subPixelX;
                        kickoffCtx->render.pixelSubpixelPos.y = subPixelY;

                        error = clEnqueueWriteBuffer(
                            cmdQueue, kickoffCtxManager->buffers[currentCircularPassId], CL_FALSE,
                            0, sizeof(common_render_context_t), kickoffCtx,
                            0, nullptr,
                            &currentPassEvent->bufferWriteDone
                        );

                        CS499R_ASSERT_NO_CL_ERROR(error);
                    }

                    // kickoff this pass
                    for (size_t kickoffTileId = 0; kickoffTileId < kickoffTileCount; kickoffTileId++)
                    {
                        auto const kickoffCtxManager = kickoffCtxManagers + kickoffTileId;
                        auto const currentPassEvent = kickoffCtxManager->events + currentCircularPassId;

                        error |= clSetKernelArg(
                            kernel, 0,
                            sizeof(cl_mem), &kickoffCtxManager->buffers[currentCircularPassId]
                        );
                        error |= clEnqueueNDRangeKernel(
                            cmdQueue, kernel,
                            1, nullptr, &kickoffTileGlobalSize, &kickoffTileLocalSize,
                            1, &currentPassEvent->bufferWriteDone,
                            &currentPassEvent->kickoffDone
                        );

                        CS499R_ASSERT_NO_CL_ERROR(error);
                    }

                    passId++;

                    if (passId >= kHostAheadCommandCount && passId != passCount)
                    {
                        size_t const nextCircularPassId = passId % kHostAheadCommandCount;

                        for (size_t kickoffTileId = 0; kickoffTileId < kickoffTileCount; kickoffTileId++)
                        {
                            auto const kickoffCtxManager = kickoffCtxManagers + kickoffTileId;
                            auto const nextPassEvent = kickoffCtxManager->events + nextCircularPassId;

                            error |= clWaitForEvents(1, &nextPassEvent->bufferWriteDone);
                            error |= clWaitForEvents(1, &nextPassEvent->kickoffDone);
                            error |= clReleaseEvent(nextPassEvent->bufferWriteDone);
                            error |= clReleaseEvent(nextPassEvent->kickoffDone);

                            CS499R_ASSERT_NO_CL_ERROR(error);
                        }
                    }
                }
            }
        }

        CS499R_ASSERT(passId == passCount);

        // wait all remaining passes
        for (size_t circularPassId = 0; circularPassId < min(passCount, kHostAheadCommandCount); circularPassId++)
        {
            for (size_t kickoffTileId = 0; kickoffTileId < kickoffTileCount; kickoffTileId++)
            {
                auto const kickoffCtxManager = kickoffCtxManagers + kickoffTileId;
                auto const circularPassEvent = kickoffCtxManager->events + circularPassId;

                error |= clWaitForEvents(1, &circularPassEvent->bufferWriteDone);
                error |= clWaitForEvents(1, &circularPassEvent->kickoffDone);
                error |= clReleaseEvent(circularPassEvent->bufferWriteDone);
                error |= clReleaseEvent(circularPassEvent->kickoffDone);

                CS499R_ASSERT_NO_CL_ERROR(error);
            }
        }

        if (outProfiling)
        {
            kernelEnd = timestamp();
        }

        { // render target multiplication
            float32_t const multiplyFactor = 1.0f / float32_t(pixelSampleCount);

            multiplyRenderTarget(multiplyFactor);
        }

        { // memory releases
            for (size_t kickoffTileId = 0; kickoffTileId < kickoffTileCount; kickoffTileId++)
            {
                auto const kickoffCtxManager = kickoffCtxManagers + kickoffTileId;

                for (size_t circularPassId = 0; circularPassId < kHostAheadCommandCount; circularPassId++)
                {
                    error = clReleaseMemObject(kickoffCtxManager->buffers[circularPassId]);
                    CS499R_ASSERT_NO_CL_ERROR(error);
                }
            }

            free(kickoffCtxCircularArray);
            free(kickoffCtxManagers);
        }

        if (outProfiling)
        {
            outProfiling->mCPUDuration = kernelEnd - kernelStart;
            outProfiling->mRays = mRenderTarget->width() * mRenderTarget->height() * pixelRayCount;
        }
    }

}
