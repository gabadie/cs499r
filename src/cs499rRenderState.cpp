
#include "cs499rBenchmark.hpp"
#include "cs499rCamera.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rRenderAbstractTracker.hpp"
#include "cs499rRenderShotCtx.hpp"
#include "cs499rRenderState.hpp"
#include "cs499rRenderTarget.hpp"
#include "cs499rScene.hpp"
#include "cs499rSceneBuffer.hpp"


namespace CS499R
{
    void
    RenderState::downscale(RenderTarget const * srcRenderTarget)
    {
        CS499R_ASSERT(mRenderTarget != nullptr);
        CS499R_ASSERT(srcRenderTarget != nullptr);
        CS499R_ASSERT(mRenderTarget != srcRenderTarget);

        downscale(
            mRenderTarget,
            srcRenderTarget,
            size2_t(0),
            srcRenderTarget->resolution(),
            size2_t(0),
            mRenderTarget->resolution()
        );
    }

    void
    RenderState::shotScene(SceneBuffer const * sceneBuffer, Camera const * camera, RenderAbstractTracker * renderTracker)
    {
        CS499R_ASSERT(sceneBuffer != nullptr);
        CS499R_ASSERT(camera != nullptr);
        CS499R_ASSERT(renderTracker != nullptr);
        CS499R_ASSERT(validateParams());
        CS499R_ASSERT(mRenderTarget != nullptr);
        CS499R_ASSERT(mRenderTarget->mRayTracer == sceneBuffer->mRayTracer);

        RenderShotCtx shotCtx;
        common_render_context_t templateCtx;

        shotInit(&shotCtx);

        renderTracker->eventShotStart(&shotCtx);

        shotInitTemplateCtx(&shotCtx, sceneBuffer, camera, &templateCtx);
        shotInitCircularCtx(&shotCtx, &templateCtx);
        shotKickoff(&shotCtx, sceneBuffer, renderTracker);
        shotFree(&shotCtx);

        renderTracker->eventShotEnd();
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
    RenderState::downscale(
        RenderTarget * const destRenderTarget,
        RenderTarget const * const srcRenderTarget,
        size2_t const srcPos,
        size2_t const srcSize,
        size2_t const destPos,
        size2_t const destSize,
        float32_t const multiplyFactor,
        cl_uint const eventWaitListSize,
        cl_event const * const eventWaitList,
        cl_event * const event
    )
    {
        CS499R_ASSERT(destRenderTarget != nullptr);
        CS499R_ASSERT(srcRenderTarget != nullptr);
        CS499R_ASSERT(destRenderTarget != srcRenderTarget);
        CS499R_ASSERT((srcSize.x % destSize.x) == 0);
        CS499R_ASSERT((srcSize.y % destSize.y) == 0);
        CS499R_ASSERT((srcSize.x / destSize.x) == (srcSize.y / destSize.y));

        common_downscale_context_t ctx;

        ctx.srcResolution.x = srcRenderTarget->width();
        ctx.srcResolution.y = srcRenderTarget->height();
        ctx.srcTilePos.x = srcPos.x;
        ctx.srcTilePos.y = srcPos.y;
        ctx.srcTileSize = srcSize.x;

        ctx.destResolution.x = destRenderTarget->width();
        ctx.destResolution.y = destRenderTarget->height();
        ctx.destTilePos.x = destPos.x;
        ctx.destTilePos.y = destPos.y;
        ctx.destTileSize = destSize.x;
        ctx.downscaleFactor = srcSize.x / destSize.x;

        ctx.multiplyFactor = multiplyFactor / float32_t(ctx.downscaleFactor.value * ctx.downscaleFactor.value);

        auto const rayTracer = mRenderTarget->mRayTracer;

        cl_kernel const kernel = rayTracer->mProgram[RayTracer::kProgramTragetDownscale].kernel;
        cl_int error = 0;

        { // kernel arguments
            error |= clSetKernelArg(kernel, 0, sizeof(ctx), &ctx);
            error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &srcRenderTarget->mGpuBuffer);
            error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &destRenderTarget->mGpuBuffer);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        size_t const globalCount = destSize.x * destSize.y;

        error = clEnqueueNDRangeKernel(
            rayTracer->mCmdQueue, kernel,
            1, nullptr, &globalCount, nullptr,
            eventWaitListSize, eventWaitList,
            event
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
    RenderState::shotInit(RenderShotCtx * ctx) const
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

        ctx->renderResolution.x = mRenderTarget->width();
        ctx->renderResolution.y = mRenderTarget->height();

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
            (ctx->renderResolution.x + ctx->kickoffTileSize - 1) / ctx->kickoffTileSize,
            (ctx->renderResolution.y + ctx->kickoffTileSize - 1) / ctx->kickoffTileSize
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
            ctx->kickoffCtxManagers = alloc<RenderShotCtx::kickoff_ctx_manager_t>(ctx->kickoffTileCount);
        }
    }

    void
    RenderState::shotInitTemplateCtx(
        RenderShotCtx const * ctx,
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

        templateCtx->render.passCount = ctx->pixelSubdivisions * ctx->kickoffInvocationCount;

        templateCtx->render.kickoffTileSize = ctx->kickoffTileSize;
        templateCtx->render.kickoffSampleIterationCount = ctx->kickoffSampleIterationCount;
        templateCtx->render.kickoffSampleRecursionCount = ctx->recursionPerSample;

        { // render context's CBT init
            templateCtx->render.cpt.coherencyTileSize = ctx->coherencyTileSize;
            templateCtx->render.cpt.coherencyTilePerKickoffTileBorder = (
                ctx->kickoffTileSize / ctx->coherencyTileSize
            );
            templateCtx->render.cpt.groupPerCoherencyTile = (
                (ctx->coherencyTileSize * ctx->coherencyTileSize) /
                ctx->kickoffTileLocalSize
            );

            /*
             * Depending on the warp size, we might have cases where the local
             * size might not be a square number. But since it will always be
             * power of two, we will split the coherency tile in two.
             */
            CS499R_ASSERT(
                templateCtx->render.cpt.groupPerCoherencyTile.value == 1 ||
                templateCtx->render.cpt.groupPerCoherencyTile.value == 2
            );
        }

        { // render context's ICPT init
            size_t const kCoherencyTileSize = 16;

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
        RenderShotCtx const * ctx,
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
    RenderState::shotKickoff(RenderShotCtx const * ctx, SceneBuffer const * sceneBuffer, RenderAbstractTracker * renderTracker)
    {
        auto const rayTracer = sceneBuffer->mRayTracer;

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

        size_t passId = 0;
        size_t const passCount = ctx->kickoffInvocationCount * ctx->pixelSubdivisions;

        size_t progressId = 1;
        size_t const progressCount = passCount;

        renderTracker->eventShotProgress(0, progressCount);

        ShotIteration it;

        // render loops
        for (it.invocationId = 0; it.invocationId < ctx->kickoffInvocationCount; it.invocationId++)
        {
            for (it.subPixel.y = 0; it.subPixel.y < ctx->pixelBorderSubdivisions; it.subPixel.y++)
            {
                for (it.subPixel.x = 0; it.subPixel.x < ctx->pixelBorderSubdivisions; it.subPixel.x++)
                {
                    size_t const currentCircularPassId = passId % kHostAheadCommandCount;

                    auto const kickoffCtxArray = ctx->kickoffCtxCircularArray + ctx->kickoffTileCount * currentCircularPassId;

                    // upload render context buffers for this pass
                    for (it.kickoffTileId = 0; it.kickoffTileId < ctx->kickoffTileCount; it.kickoffTileId++)
                    {
                        auto const kickoffCtxManager = ctx->kickoffCtxManagers + it.kickoffTileId;
                        auto const kickoffCtx = kickoffCtxArray + it.kickoffTileId;

                        auto const currentPassEvent = kickoffCtxManager->events + currentCircularPassId;

                        shotUpdateKickoffRenderCtx(
                            ctx,
                            &it,
                            kickoffCtx,
                            cmdQueue,
                            kickoffCtxManager->buffers[currentCircularPassId],
                            0, nullptr,
                            &currentPassEvent->bufferWriteDone
                        );
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

                        renderTracker->eventShotProgress(progressId, progressCount);
                        progressId++;
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

            renderTracker->eventShotProgress(progressId, progressCount);
            progressId++;
        }

        { // render target multiplication
            float32_t const multiplyFactor = 1.0f / float32_t(ctx->pixelSampleCount);

            multiplyRenderTarget(multiplyFactor);
        }
    }

    void
    RenderState::shotUpdateKickoffRenderCtx(
        RenderShotCtx const * ctx,
        ShotIteration const * shotIteration,
        common_render_context_t * kickoffCtx,
        cl_command_queue cmdQueue,
        cl_mem renderCtxBuffer,
        cl_uint eventWaitListSize,
        cl_event const * eventWaitList,
        cl_event * event
    ) const
    {
        kickoffCtx->render.passId = (
            shotIteration->invocationId * ctx->pixelSubdivisions +
            shotIteration->subPixel.y * ctx->pixelBorderSubdivisions +
            shotIteration->subPixel.x
        );

        kickoffCtx->render.pixelSubpixelPos.x = shotIteration->subPixel.x;
        kickoffCtx->render.pixelSubpixelPos.y = shotIteration->subPixel.y;

        cl_int error = clEnqueueWriteBuffer(
            cmdQueue, renderCtxBuffer, CL_FALSE,
            0, sizeof(common_render_context_t), kickoffCtx,
            eventWaitListSize, eventWaitList,
            event
        );

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    void
    RenderState::shotFree(RenderShotCtx const * ctx) const
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

    size_t const RenderState::kMaxKickoffSampleIteration;
    size_t const RenderState::kThreadsPerTilesTarget;
    size_t const RenderState::kWarpSizefactor;


}
