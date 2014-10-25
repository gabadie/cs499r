
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

        shot_ctx_t shotCtx;
        common_render_context_t templateCtx;

        shotInit(&shotCtx);
        shotInitTemplateCtx(&shotCtx, sceneBuffer, camera, &templateCtx);
        shotInitCircularCtx(&shotCtx, &templateCtx);
        shotKickoff(&shotCtx, sceneBuffer, outProfiling);
        shotFree(&shotCtx);
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
    RenderState::shotInit(shot_ctx_t * ctx) const
    {
        size_t warpSize;

        { // fetch OpenCL constants
            auto const rayTracer = mRenderTarget->mRayTracer;
            cl_kernel const kernel = rayTracer->mProgram[mRayAlgorithm].kernel;

            cl_int error =  clGetKernelWorkGroupInfo(
                kernel, rayTracer->mDeviceId, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
                sizeof(warpSize), &warpSize, nullptr
            );

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        bool const debugKernel = mRayAlgorithm != kRayAlgorithmPathTracer;
        ctx->pixelBorderSubdivisions = debugKernel ? 1 : mPixelBorderSubdivisions;
        ctx->samplesPerSubdivisions = debugKernel ? 1 : mSamplesPerSubdivisions;
        ctx->recursionPerSample = debugKernel ? 1 : mRecursionPerSample;

        ctx->pixelSubdivisions = ctx->pixelBorderSubdivisions * ctx->pixelBorderSubdivisions;
        ctx->pixelSampleCount = ctx->pixelSubdivisions * ctx->samplesPerSubdivisions;
        ctx->pixelRayCount = ctx->pixelSampleCount * ctx->recursionPerSample;

        ctx->kickoffSampleIterationCount = min(ctx->samplesPerSubdivisions, kMaxKickoffSampleIteration);
        ctx->kickoffInvocationCount =ctx-> samplesPerSubdivisions / ctx->kickoffSampleIterationCount;

        ctx->kickoffTileSize = sqrt(kThreadsPerTilesTarget);
        ctx->kickoffTileGlobalSize = ctx->kickoffTileSize * ctx->kickoffTileSize;

        /*
         * We multiply here between the local size and warp size so that we
         * don't overload the warp scheduler. Also, might not want to multiply
         * to much, because it will also decrease the warp coherency
         */
        ctx->kickoffTileLocalSize = warpSize * kWarpSizefactor;
        ctx->coherencyTileSize = sqrt(ceilSquarePow2(ctx->kickoffTileLocalSize));
        ctx->kickoffTileGrid = size2_t(
            (mRenderTarget->width() + ctx->kickoffTileSize - 1) / ctx->kickoffTileSize,
            (mRenderTarget->height() + ctx->kickoffTileSize - 1) / ctx->kickoffTileSize
        );
        ctx->kickoffTileCount = ctx->kickoffTileGrid.x * ctx->kickoffTileGrid.y;

        { // validation
            CS499R_ASSERT((ctx->kickoffTileGlobalSize % ctx->kickoffTileLocalSize) == 0);

            CS499R_ASSERT((ctx->kickoffTileLocalSize % ctx->coherencyTileSize) == 0);
            CS499R_ASSERT((ctx->kickoffTileLocalSize % warpSize) == 0);
            CS499R_ASSERT(ctx->kickoffTileLocalSize <= CS499R_MAX_GROUP_SIZE);
            CS499R_ASSERT((ctx->kickoffTileSize % ctx->coherencyTileSize) == 0);
        }

        { // memory allocations
            ctx->kickoffCtxCircularArray = alloc<common_render_context_t>(ctx->kickoffTileCount * kHostAheadCommandCount);
            ctx->kickoffCtxManagers = alloc<kickoff_ctx_manager_t>(ctx->kickoffTileCount);
        }
    }

    void
    RenderState::shotInitTemplateCtx(
        shot_ctx_t const * ctx,
        SceneBuffer const * sceneBuffer,
        Camera const * camera,
        common_render_context_t * templateCtx
    ) const
    {
        { // init the camera
            auto const aspectRatio = float32_t(mRenderTarget->width()) / float32_t(mRenderTarget->height());

            camera->exportToShotCamera(&templateCtx->camera, aspectRatio);
        }

        // +1 because of the anonymous mesh
        templateCtx->scene.meshInstanceMaxId = sceneBuffer->mScene->mObjectsMap.meshInstances.size() + 1;

        // init render
        templateCtx->render.resolution.x = mRenderTarget->width();
        templateCtx->render.resolution.y = mRenderTarget->height();
        templateCtx->render.subpixelPerPixelBorder = ctx->pixelBorderSubdivisions;

        templateCtx->render.kickoffSampleIterationCount = ctx->kickoffSampleIterationCount;
        templateCtx->render.kickoffSampleRecursionCount = ctx->recursionPerSample;

        { // render context's CBT init
            templateCtx->render.cbt.kickoffTileSize = ctx->kickoffTileSize;
            templateCtx->render.cbt.coherencyTileSize = ctx->coherencyTileSize;
            templateCtx->render.cbt.coherencyTilePerKickoffTileBorder = (
                ctx->kickoffTileSize / ctx->coherencyTileSize
            );
            templateCtx->render.cbt.groupPerCoherencyTile = (
                (ctx->coherencyTileSize * ctx->coherencyTileSize) /
                ctx->kickoffTileLocalSize
            );

            /*
             * Depending on the warp size, we might have cases where the local
             * size might not be a square number. But since it will always be
             * power of two, we will split the coherency tile in two.
             */
            CS499R_ASSERT(
                templateCtx->render.cbt.groupPerCoherencyTile.value == 1 ||
                templateCtx->render.cbt.groupPerCoherencyTile.value == 2
            );
        }

        { // render context's ICPT init
            size_t const kCoherencyTileSize = 16;

            templateCtx->render.icpt.kickoffTileSize = ctx->kickoffTileSize;
            templateCtx->render.icpt.coherencyTilePerKickoffTileBorder = (
                ctx->kickoffTileSize / kCoherencyTileSize
            );
            templateCtx->render.icpt.groupPerCoherencyTile = (
                (kCoherencyTileSize * kCoherencyTileSize) /
                ctx->kickoffTileLocalSize
            );
        }
    }

    void
    RenderState::shotInitCircularCtx(
        shot_ctx_t const * ctx,
        common_render_context_t const * templateCtx
    ) const
    {
        cl_int error = 0;
        cl_context const context = mRenderTarget->mRayTracer->mContext;

        // init render context circular pass 0
        for (size_t kickoffTileIdY = 0; kickoffTileIdY < ctx->kickoffTileGrid.y; kickoffTileIdY++)
        {
            for (size_t kickoffTileIdX = 0; kickoffTileIdX < ctx->kickoffTileGrid.x; kickoffTileIdX++)
            {
                size_t const kickoffTileId = kickoffTileIdX + kickoffTileIdY * ctx->kickoffTileGrid.x;
                auto const kickoffCtx = ctx->kickoffCtxCircularArray + kickoffTileId;

                memcpy(kickoffCtx, templateCtx, sizeof(*templateCtx));

                kickoffCtx->render.kickoffTilePos.x = ctx->kickoffTileSize * kickoffTileIdX;
                kickoffCtx->render.kickoffTilePos.y = ctx->kickoffTileSize * kickoffTileIdY;

                for (size_t circularPassId = 0; circularPassId < kHostAheadCommandCount; circularPassId++)
                {
                    ctx->kickoffCtxManagers[kickoffTileId].buffers[circularPassId] = clCreateBuffer(context,
                        CL_MEM_READ_ONLY,
                        sizeof(*templateCtx), nullptr,
                        &error
                    );
                }

                CS499R_ASSERT_NO_CL_ERROR(error);
            }
        }

        // clones the render context circular pass 0 into others
        for (size_t circularPassId = 1; circularPassId < kHostAheadCommandCount; circularPassId++)
        {
            auto const kickoffCtxArray = ctx->kickoffCtxCircularArray + ctx->kickoffTileCount * circularPassId;

            for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount; kickoffTileId++)
            {
                memcpy(
                    kickoffCtxArray + kickoffTileId,
                    ctx->kickoffCtxCircularArray + kickoffTileId,
                    sizeof(*templateCtx)
                );
            }
        }
    }

    void
    RenderState::shotKickoff(shot_ctx_t const * ctx, SceneBuffer const * sceneBuffer, RenderProfiling * outProfiling)
    {
        auto const rayTracer = sceneBuffer->mRayTracer;

        timestamp_t kernelStart;
        timestamp_t kernelEnd;

        cl_int error = 0;
        cl_command_queue const cmdQueue = rayTracer->mCmdQueue;
        cl_kernel const kernel = rayTracer->mProgram[mRayAlgorithm].kernel;

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
        size_t const passCount = ctx->kickoffInvocationCount * ctx->pixelSubdivisions;

        // render loops
        for (size_t invocationId = 0; invocationId < ctx->kickoffInvocationCount; invocationId++)
        {
            for (size_t subPixelY = 0; subPixelY < ctx->pixelBorderSubdivisions; subPixelY++)
            {
                for (size_t subPixelX = 0; subPixelX < ctx->pixelBorderSubdivisions; subPixelX++)
                {
                    size_t const currentCircularPassId = passId % kHostAheadCommandCount;

                    auto const kickoffCtxArray = ctx->kickoffCtxCircularArray + ctx->kickoffTileCount * currentCircularPassId;

                    // upload render context buffers for this pass
                    for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount; kickoffTileId++)
                    {
                        auto const kickoffCtxManager = ctx->kickoffCtxManagers + kickoffTileId;
                        auto const kickoffCtx = kickoffCtxArray + kickoffTileId;

                        auto const currentPassEvent = kickoffCtxManager->events + currentCircularPassId;

                        kickoffCtx->render.passId = (
                            invocationId * ctx->pixelSubdivisions +
                            subPixelY * ctx->pixelBorderSubdivisions +
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
                    for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount; kickoffTileId++)
                    {
                        auto const kickoffCtxManager = ctx->kickoffCtxManagers + kickoffTileId;
                        auto const currentPassEvent = kickoffCtxManager->events + currentCircularPassId;

                        error |= clSetKernelArg(
                            kernel, 0,
                            sizeof(cl_mem), &kickoffCtxManager->buffers[currentCircularPassId]
                        );
                        error |= clEnqueueNDRangeKernel(
                            cmdQueue, kernel,
                            1, nullptr, &ctx->kickoffTileGlobalSize, &ctx->kickoffTileLocalSize,
                            1, &currentPassEvent->bufferWriteDone,
                            &currentPassEvent->kickoffDone
                        );

                        CS499R_ASSERT_NO_CL_ERROR(error);
                    }

                    passId++;

                    if (passId >= kHostAheadCommandCount && passId != passCount)
                    {
                        size_t const nextCircularPassId = passId % kHostAheadCommandCount;

                        for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount; kickoffTileId++)
                        {
                            auto const kickoffCtxManager = ctx->kickoffCtxManagers + kickoffTileId;
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
            for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount; kickoffTileId++)
            {
                auto const kickoffCtxManager = ctx->kickoffCtxManagers + kickoffTileId;
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

            outProfiling->mCPUDuration = kernelEnd - kernelStart;
            outProfiling->mRays = mRenderTarget->width() * mRenderTarget->height() * ctx->pixelRayCount;
        }

        { // render target multiplication
            float32_t const multiplyFactor = 1.0f / float32_t(ctx->pixelSampleCount);

            multiplyRenderTarget(multiplyFactor);
        }
    }

    void
    RenderState::shotFree(shot_ctx_t const * ctx) const
    {
        for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount; kickoffTileId++)
        {
            auto const kickoffCtxManager = ctx->kickoffCtxManagers + kickoffTileId;

            for (size_t circularPassId = 0; circularPassId < kHostAheadCommandCount; circularPassId++)
            {
                cl_int error = clReleaseMemObject(kickoffCtxManager->buffers[circularPassId]);
                CS499R_ASSERT_NO_CL_ERROR(error);
            }
        }

        free(ctx->kickoffCtxCircularArray);
        free(ctx->kickoffCtxManagers);
    }

    // ------------------------------------------------------------------------- NASTY C++ WORK AROUNDS

    size_t const RenderState::kHostAheadCommandCount;
    size_t const RenderState::kMaxKickoffSampleIteration;
    size_t const RenderState::kThreadsPerTilesTarget;
    size_t const RenderState::kWarpSizefactor;


}
