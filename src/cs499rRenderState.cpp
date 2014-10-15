
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

        if (mRayAlgorithm == kRayAlgorithmPathTracer)
        {
            shotSceneCoherency(sceneBuffer, camera, outProfiling);
        }
        else
        {
            shotSceneDebug(sceneBuffer, camera, outProfiling);
        }
    }

    RenderState::RenderState()
    {
        mPixelBorderSubdivisions = kDefaultPixelBorderSubdivisions;
        mSamplesPerSubdivisions = kDefaultSamplesPerSubdivisions;
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

        size_t const kThreadsPerTilesTarget = 2048 * 8;

        auto const rayTracer = sceneBuffer->mRayTracer;

        timestamp_t kernelStart;
        timestamp_t kernelEnd;

        cl_int error = 0;
        cl_context const context = rayTracer->mContext;
        cl_command_queue const cmdQueue = rayTracer->mCmdQueue;
        cl_kernel const kernel = rayTracer->mProgram[mRayAlgorithm].kernel;

        size_t const coherencyTileSize = 8;
        size_t const kickoffTileSize = sqrt(kThreadsPerTilesTarget);
        size_t const kickoffTileGlobalSize = kickoffTileSize * kickoffTileSize;
        size_t const kickoffTileLocalSize = kCS499RGpuWarpSize * 2;
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
            templateCtx.render.subpixelPerPixelBorder = mPixelBorderSubdivisions;

            templateCtx.render.kickoffTileSize = kickoffTileSize;
            templateCtx.render.kickoffTileSizeLog = log2(kickoffTileSize);
            templateCtx.render.coherencyTileSize = coherencyTileSize;
            templateCtx.render.coherencyTileSizeLog = log2(coherencyTileSize);
            templateCtx.render.coherencyTilePerKickoffTileBorder = (
                kickoffTileSize / coherencyTileSize
            );
            templateCtx.render.coherencyTilePerKickoffTileBorderLog =
                log2(templateCtx.render.coherencyTilePerKickoffTileBorder);
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
        size_t const passCount = mSamplesPerSubdivisions * mPixelBorderSubdivisions * mPixelBorderSubdivisions;

        // render loops
        for (size_t sampleId = 0; sampleId < mSamplesPerSubdivisions; sampleId++)
        {
            for (size_t subPixelY = 0; subPixelY < mPixelBorderSubdivisions; subPixelY++)
            {
                for (size_t subPixelX = 0; subPixelX < mPixelBorderSubdivisions; subPixelX++)
                {
                    size_t const currentCircularPassId = passId % kHostAheadCommandCount;

                    auto const kickoffCtxArray = kickoffCtxCircularArray + kickoffTileCount * currentCircularPassId;

                    // upload render context buffers for this pass
                    for (size_t kickoffTileId = 0; kickoffTileId < kickoffTileCount; kickoffTileId++)
                    {
                        auto const kickoffCtxManager = kickoffCtxManagers + kickoffTileId;
                        auto const kickoffCtx = kickoffCtxArray + kickoffTileId;

                        auto const currentPassEvent = kickoffCtxManager->events + currentCircularPassId;

                        kickoffCtx->render.kickoffTileRandomSeedOffset = 20 * (
                            sampleId + mSamplesPerSubdivisions * (
                                subPixelY * mPixelBorderSubdivisions + subPixelX
                            )
                        ); // TODO
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

        // wait all remaining passes
        for (size_t circularPassId = 0; circularPassId < min(passId, kHostAheadCommandCount); circularPassId++)
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
            float32_t const multiplyFactor = 1.0f / float32_t(
                mSamplesPerSubdivisions * mPixelBorderSubdivisions * mPixelBorderSubdivisions
            );

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
            outProfiling->mSamples = mRenderTarget->width() * mRenderTarget->height() *
                mPixelBorderSubdivisions * mPixelBorderSubdivisions * mSamplesPerSubdivisions;
        }
    }

    void
    RenderState::shotSceneDebug(SceneBuffer const * sceneBuffer, Camera const * camera, RenderProfiling * outProfiling)
    {
        auto const rayTracer = sceneBuffer->mRayTracer;

        timestamp_t kernelStart;
        timestamp_t kernelEnd;

        cl_int error = 0;
        cl_context const context = rayTracer->mContext;
        cl_command_queue const cmdQueue = rayTracer->mCmdQueue;
        cl_kernel const kernel = rayTracer->mProgram[mRayAlgorithm].kernel;

        bool const debugKernel = mRayAlgorithm != kRayAlgorithmPathTracer;
        size_t const pixelBorderSubdivisions = debugKernel ? 1 : mPixelBorderSubdivisions;
        size_t const samplesPerSubdivisions = debugKernel ? 1 : mSamplesPerSubdivisions;

        float const aspectRatio = float(mRenderTarget->width()) / float(mRenderTarget->height());
        common_render_context_t shotContext;

        { // init shot context
            camera->exportToShotCamera(&shotContext.camera, aspectRatio);

            shotContext.render.resolution.x = mRenderTarget->width();
            shotContext.render.resolution.y = mRenderTarget->height();
            shotContext.render.subpixelPerPixelBorder = mPixelBorderSubdivisions;

            // +1 because of the anonymous mesh
            shotContext.scene.meshInstanceMaxId = sceneBuffer->mScene->mObjectsMap.meshInstances.size() + 1;
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
            error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &sceneBuffer->mBuffer.meshInstances);
            error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &sceneBuffer->mBuffer.meshOctreeNodes);
            error |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &sceneBuffer->mBuffer.primitives);
            error |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &mRenderTarget->mGpuBuffer);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        { // launch kernel
            size_t const kInvocationDims = 3;
            size_t const kThreadsPerTilesTarget = 2048 * 8;

            size_t const groupSize[kInvocationDims] = {
                pixelBorderSubdivisions,
                pixelBorderSubdivisions,
                samplesPerSubdivisions,
            };

            size_t maxWorkGroupSize = 0;
            clGetDeviceInfo(rayTracer->mDeviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWorkGroupSize), &maxWorkGroupSize, NULL);

            size2_t const globalSize(
                mRenderTarget->width() * groupSize[0],
                mRenderTarget->height() * groupSize[1]
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
            outProfiling->mSamples = mRenderTarget->width() * mRenderTarget->height() *
                pixelBorderSubdivisions * pixelBorderSubdivisions * samplesPerSubdivisions;
        }

        error = clReleaseMemObject(shotContextBuffer);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }


}
