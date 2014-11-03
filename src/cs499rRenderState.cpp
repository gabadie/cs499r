
#include "cs499rBenchmark.hpp"
#include "cs499rCamera.hpp"
#include "cs499rCompiledScene.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rRenderAbstractTracker.hpp"
#include "cs499rRenderShotCtx.hpp"
#include "cs499rRenderState.hpp"
#include "cs499rRenderTarget.hpp"
#include "cs499rScene.hpp"

#include <iostream>

#define show(x) \
    std::cout << #x << " = " << x << std::endl


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
    RenderState::shotScene(CompiledScene const * compiledScene, Camera const * camera, RenderAbstractTracker * renderTracker)
    {
        CS499R_ASSERT(compiledScene != nullptr);
        CS499R_ASSERT(camera != nullptr);
        CS499R_ASSERT(renderTracker != nullptr);
        CS499R_ASSERT(validateParams());
        CS499R_ASSERT(mRenderTarget != nullptr);
        CS499R_ASSERT(mRenderTarget->mRayTracer == compiledScene->mRayTracer);

        RenderShotCtx shotCtx;

        shotInit(&shotCtx);

        renderTracker->eventShotStart(&shotCtx);

        { // init
            common_render_context_t templateCtx;

            shotInitTemplateCtx(&shotCtx, compiledScene, camera, &templateCtx);
            shotInitKickoffEntries(&shotCtx, &templateCtx);
            shotAllocKickoffEntriesBuffer(&shotCtx);
        }

        // kick off
        shotKickoff(&shotCtx, compiledScene, renderTracker);

        // free
        shotFreeKickoffEntriesBuffer(&shotCtx);

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
    RenderState::clear(RenderTarget * destRenderTarget)
    {
        uint8_t const pattern = 0;

        auto const rayTracer = destRenderTarget->mRayTracer;

        cl_int error = clEnqueueFillBuffer(
            rayTracer->mCmdQueue, destRenderTarget->mGpuBuffer,
            &pattern, sizeof(pattern),
            0, destRenderTarget->width() * destRenderTarget->height() * RenderTarget::kChanelCount * sizeof(float32_t),
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
        CS499R_ASSERT((srcPos.x + srcSize.x) <= srcRenderTarget->width());
        CS499R_ASSERT((srcPos.y + srcSize.y) <= srcRenderTarget->height());
        CS499R_ASSERT((destPos.x + destSize.x) <= destRenderTarget->width());
        CS499R_ASSERT((destPos.y + destSize.y) <= destRenderTarget->height());

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

        ctx->renderTargetResolution = mRenderTarget->resolution();

        bool const debugKernel = mRayAlgorithm != kRayAlgorithmPathTracer;
        ctx->pixelBorderSubdivisions = debugKernel ? 1 : mPixelBorderSubdivisions;
        ctx->samplesPerSubdivisions = debugKernel ? 1 : mSamplesPerSubdivisions;
        ctx->recursionPerSample = debugKernel ? 1 : mRecursionPerSample;

        ctx->kickoffTileSize = sqrt(kThreadsPerTilesTarget);

        /*
         * We multiply here between the local size and warp size so that we
         * don't overload the warp scheduler. Also, might not want to multiply
         * to much, because it will also decrease the warp coherency
         */
        ctx->kickoffTileLocalSize = warpSize * kWarpSizefactor;
        ctx->kickoffTileGrid = size2_t(
            (ctx->virtualTargetResolution().x + ctx->kickoffTileSize - 1) / ctx->kickoffTileSize,
            (ctx->virtualTargetResolution().y + ctx->kickoffTileSize - 1) / ctx->kickoffTileSize
        );

        { // validation
            CS499R_ASSERT((ctx->kickoffTileGlobalSize() % ctx->kickoffTileLocalSize) == 0);

            CS499R_ASSERT((ctx->kickoffTileLocalSize % warpSize) == 0);
            CS499R_ASSERT(ctx->kickoffTileLocalSize <= CS499R_MAX_GROUP_SIZE);
        }

        { // memory allocations
            ctx->allocKickoffEntries();
        }
    }

    void
    RenderState::shotInitTemplateCtx(
        RenderShotCtx const * ctx,
        CompiledScene const * compiledScene,
        Camera const * camera,
        common_render_context_t * templateCtx
    ) const
    {
        { // init the camera
            auto const aspectRatio = float32_t(mRenderTarget->width()) / float32_t(mRenderTarget->height());

            camera->exportToShotCamera(&templateCtx->camera, aspectRatio);
        }

        // +1 because of the anonymous mesh
        templateCtx->scene.meshInstanceMaxId = compiledScene->mScene->mObjectsMap.meshInstances.size() + 1;
        templateCtx->scene.octreeOffset = compiledScene->mInfo.sceneOctreeOffset;
        templateCtx->scene.octreeRootHalfSize = compiledScene->mInfo.sceneOctreeRootHalfSize;

        // init render
        if (ctx->superTiling())
        {
            templateCtx->render.targetResolution = ctx->virtualPixelPerSuperTileBorder();
        }
        else
        {
            templateCtx->render.targetResolution.x = mRenderTarget->width();
            templateCtx->render.targetResolution.y = mRenderTarget->height();
        }
        templateCtx->render.virtualTargetResolution.x = ctx->virtualTargetResolution().x;
        templateCtx->render.virtualTargetResolution.y = ctx->virtualTargetResolution().y;
        templateCtx->render.subpixelPerPixelBorder = ctx->virtualPixelBorderSubdivisions();

        templateCtx->render.passCount = ctx->pixelSubdivisions() * ctx->kickoffInvocationCount();

        templateCtx->render.kickoffTileSize = ctx->kickoffTileSize;
        templateCtx->render.kickoffSampleIterationCount = ctx->kickoffSampleIterationCount();
        templateCtx->render.kickoffSampleRecursionCount = ctx->recursionPerSample;

        { // render context's CBT init
            size_t const coherencyTileSize = sqrt(ceilSquarePow2(ctx->kickoffTileLocalSize));

            templateCtx->render.cpt.coherencyTileSize = coherencyTileSize;
            templateCtx->render.cpt.coherencyTilePerKickoffTileBorder = (
                ctx->kickoffTileSize / coherencyTileSize
            );
            templateCtx->render.cpt.groupPerCoherencyTile = (
                (coherencyTileSize * coherencyTileSize) /
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
    RenderState::shotInitKickoffEntries(
        RenderShotCtx const * ctx,
        common_render_context_t const * templateCtx
    ) const
    {
        // init render context circular pass 0
        for (size_t kickoffTileIdY = 0; kickoffTileIdY < ctx->kickoffTileGrid.y; kickoffTileIdY++)
        {
            for (size_t kickoffTileIdX = 0; kickoffTileIdX < ctx->kickoffTileGrid.x; kickoffTileIdX++)
            {
                size_t const kickoffTileId = kickoffTileIdX + kickoffTileIdY * ctx->kickoffTileGrid.x;

                auto const kickoffEntry = ctx->kickoffEntry(0, kickoffTileId);

                memcpy(&kickoffEntry->ctx, templateCtx, sizeof(*templateCtx));

                kickoffEntry->ctx.render.kickoffTilePos.x = ctx->kickoffTileSize * kickoffTileIdX;
                kickoffEntry->ctx.render.kickoffTilePos.y = ctx->kickoffTileSize * kickoffTileIdY;
            }
        }

        // clones the render context circular pass 0 into others
        for (size_t circularPassId = 1; circularPassId < kHostAheadCommandCount; circularPassId++)
        {
            for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount(); kickoffTileId++)
            {
                auto const kickoffEntrySrc = ctx->kickoffEntry(0, kickoffTileId);
                auto const kickoffEntryDest = ctx->kickoffEntry(circularPassId, kickoffTileId);

                memcpy(
                    &kickoffEntryDest->ctx,
                    &kickoffEntrySrc->ctx,
                    sizeof(kickoffEntryDest->ctx)
                );
            }
        }
    }

    void
    RenderState::shotAllocKickoffEntriesBuffer(
        RenderShotCtx const * ctx
    ) const
    {
        cl_int error = 0;
        cl_context const context = mRenderTarget->mRayTracer->mContext;

        for (size_t circularPassId = 0; circularPassId < kHostAheadCommandCount; circularPassId++)
        {
            for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount(); kickoffTileId++)
            {
                auto const kickoffEntry = ctx->kickoffEntry(circularPassId, kickoffTileId);

                kickoffEntry->ctxBuffer = clCreateBuffer(context,
                    CL_MEM_READ_ONLY,
                    sizeof(kickoffEntry->ctx), nullptr,
                    &error
                );

                CS499R_ASSERT_NO_CL_ERROR(error);
            }
        }
    }

    void
    RenderState::shotKickoff(RenderShotCtx const * ctx, CompiledScene const * compiledScene, RenderAbstractTracker * renderTracker)
    {
        auto const rayTracer = compiledScene->mRayTracer;

        cl_int error = 0;
        cl_command_queue const cmdQueue = rayTracer->mCmdQueue;
        cl_kernel const kernel = rayTracer->mProgram[mRayAlgorithm].kernel;

        RenderTarget * const superTileTarget = (ctx->superTiling()
                ? new RenderTarget(rayTracer, ctx->virtualPixelPerSuperTileBorder())
                : nullptr
        );

        { // kernel arguments
            error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &compiledScene->mBuffer.meshInstances);
            error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &compiledScene->mBuffer.octreeNodes);
            error |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &compiledScene->mBuffer.primitives);

            if (superTileTarget)
            {
                error |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &superTileTarget->mGpuBuffer);
            }
            else
            {
                error |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &mRenderTarget->mGpuBuffer);
            }

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        if (superTileTarget == nullptr)
        {
            /*
             * We clears the render final target because without super tiling,
             * each passes will add into the target's bufffer.
             */
            clear(mRenderTarget);
            clFinish(cmdQueue);
        }

        float32_t const finalMultiplicationFactor = 1.0f / float32_t(ctx->virtualPixelSampleCount());

        size2_t const superTileGrid = ctx->superTiling() ? ctx->superTileGrid() : 1;
        size_t const passCount = ctx->kickoffInvocationCount() * ctx->virtualPixelSubdivisions();
        size_t const progressCount = passCount * superTileGrid.x * superTileGrid.y;

        renderTracker->eventShotProgress(0, progressCount);

        ShotIteration it;
        size_t progressId = 1;

        // render loops
        for (it.superTilePos.x = 0; it.superTilePos.x < superTileGrid.x; it.superTilePos.x++) {
        for (it.superTilePos.y = 0; it.superTilePos.y < superTileGrid.y; it.superTilePos.y++)
        {
            size_t passId = 0;

            if (superTileTarget != nullptr)
            {
                /*
                 * We clears the super tile's render target because,
                 * each passes will add into it.
                 */
                clear(superTileTarget);
                clFinish(cmdQueue);
            }

            size2_t const kickoffTileStart = kMaxKickoffTilePerSuperTileBorder * it.superTilePos;
            size2_t const kickoffTileEnd = min(kickoffTileStart + kMaxKickoffTilePerSuperTileBorder, ctx->kickoffTileGrid);

            for (it.invocationId = 0; it.invocationId < ctx->kickoffInvocationCount(); it.invocationId++) {
            for (it.subPixel.y = 0; it.subPixel.y < ctx->virtualPixelBorderSubdivisions(); it.subPixel.y++) {
            for (it.subPixel.x = 0; it.subPixel.x < ctx->virtualPixelBorderSubdivisions(); it.subPixel.x++)
            {
                size_t const currentCircularPassId = passId % kHostAheadCommandCount;

                // upload render context buffers for this pass
                for (it.kickoffTilePos.x = kickoffTileStart.x; it.kickoffTilePos.x < kickoffTileEnd.x; it.kickoffTilePos.x++) {
                for (it.kickoffTilePos.y = kickoffTileStart.y; it.kickoffTilePos.y < kickoffTileEnd.y; it.kickoffTilePos.y++)
                {
                    it.kickoffTileId = it.kickoffTilePos.x + it.kickoffTilePos.y * ctx->kickoffTileGrid.x;

                    auto const kickoffEntry = ctx->kickoffEntry(currentCircularPassId, it.kickoffTileId);

                    shotUpdateKickoffRenderCtx(
                        ctx,
                        &it,
                        &kickoffEntry->ctx,
                        cmdQueue,
                        kickoffEntry->ctxBuffer,
                        0, nullptr,
                        &kickoffEntry->bufferWriteDone
                    );
                }} // for (it.kickoffTilePos)

                // kickoff this pass
                for (it.kickoffTilePos.x = kickoffTileStart.x; it.kickoffTilePos.x < kickoffTileEnd.x; it.kickoffTilePos.x++) {
                for (it.kickoffTilePos.y = kickoffTileStart.y; it.kickoffTilePos.y < kickoffTileEnd.y; it.kickoffTilePos.y++)
                {
                    it.kickoffTileId = it.kickoffTilePos.x + it.kickoffTilePos.y * ctx->kickoffTileGrid.x;

                    auto const kickoffEntry = ctx->kickoffEntry(currentCircularPassId, it.kickoffTileId);

                    shotKickoffRenderCtx(
                        ctx,
                        cmdQueue, kernel,
                        kickoffEntry->ctxBuffer,
                        1, &kickoffEntry->bufferWriteDone,
                        &kickoffEntry->kickoffDone
                    );
                }} // for (it.kickoffTilePos)

                passId++;

                if (passId >= kHostAheadCommandCount && passId != passCount)
                {
                    size_t const nextCircularPassId = passId % kHostAheadCommandCount;

                    for (it.kickoffTilePos.x = kickoffTileStart.x; it.kickoffTilePos.x < kickoffTileEnd.x; it.kickoffTilePos.x++) {
                    for (it.kickoffTilePos.y = kickoffTileStart.y; it.kickoffTilePos.y < kickoffTileEnd.y; it.kickoffTilePos.y++)
                    {
                        it.kickoffTileId = it.kickoffTilePos.x + it.kickoffTilePos.y * ctx->kickoffTileGrid.x;

                        auto const entry = ctx->kickoffEntry(nextCircularPassId, it.kickoffTileId);

                        shotWaitKickoffEntry(entry);
                    }} // for (it.kickoffTilePos)

                    renderTracker->eventShotProgress(progressId, progressCount);
                    progressId++;
                }
            }}}

            CS499R_ASSERT(passId == passCount);

            // wait all remaining passes
            for (size_t circularPassId = 0; circularPassId < min(passCount, kHostAheadCommandCount); circularPassId++)
            {
                for (it.kickoffTilePos.x = kickoffTileStart.x; it.kickoffTilePos.x < kickoffTileEnd.x; it.kickoffTilePos.x++) {
                for (it.kickoffTilePos.y = kickoffTileStart.y; it.kickoffTilePos.y < kickoffTileEnd.y; it.kickoffTilePos.y++)
                {
                    it.kickoffTileId = it.kickoffTilePos.x + it.kickoffTilePos.y * ctx->kickoffTileGrid.x;

                    auto const entry = ctx->kickoffEntry(circularPassId, it.kickoffTileId);

                    shotWaitKickoffEntry(entry);
                }} // for (it.kickoffTilePos)

                renderTracker->eventShotProgress(progressId, progressCount);
                progressId++;
            }

            if (superTileTarget != nullptr)
            {
                /*
                 * We have rendered into the super tile's target so we need to
                 * downscale it to the final target
                 */
                clFinish(cmdQueue);
                downscale(
                    mRenderTarget,
                    superTileTarget,
                    0,
                    superTileTarget->resolution(),
                    it.superTilePos * ctx->pixelPerSuperTileBorder(),
                    ctx->pixelPerSuperTileBorder(),
                    finalMultiplicationFactor,
                    0,
                    nullptr,
                    nullptr
                );
                clFinish(cmdQueue);
            }
        }} // for (it.superTilePos)

        CS499R_ASSERT(progressId == progressCount + 1);

        if (superTileTarget == nullptr)
        { // render target multiplication
            multiplyRenderTarget(finalMultiplicationFactor);
        }
        else
        {
            delete superTileTarget;
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
        kickoffCtx->render.targetVirtualOffset.x = shotIteration->superTilePos.x * ctx->virtualPixelPerSuperTileBorder();
        kickoffCtx->render.targetVirtualOffset.y = shotIteration->superTilePos.y * ctx->virtualPixelPerSuperTileBorder();

        kickoffCtx->render.passId = (
            shotIteration->invocationId * ctx->pixelSubdivisions() +
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
    RenderState::shotKickoffRenderCtx(
        RenderShotCtx const * ctx,
        cl_command_queue cmdQueue,
        cl_kernel kernel,
        cl_mem renderCtxBuffer,
        cl_uint eventWaitListSize,
        cl_event const * eventWaitList ,
        cl_event * event
    ) const
    {
        cl_int error = 0;

        size_t const globalSize = ctx->kickoffTileGlobalSize();

        error |= clSetKernelArg(
            kernel, 0,
            sizeof(renderCtxBuffer), &renderCtxBuffer
        );
        error |= clEnqueueNDRangeKernel(
            cmdQueue, kernel,
            1, nullptr, &globalSize, &ctx->kickoffTileLocalSize,
            eventWaitListSize, eventWaitList,
            event
        );

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    void
    RenderState::shotWaitKickoffEntry(
        RenderShotCtx::kickoff_entry_t * entry
    ) const
    {
        cl_int error = 0;

        error |= clWaitForEvents(1, &entry->bufferWriteDone);
        error |= clWaitForEvents(1, &entry->kickoffDone);
        error |= clReleaseEvent(entry->bufferWriteDone);
        error |= clReleaseEvent(entry->kickoffDone);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    void
    RenderState::shotFreeKickoffEntriesBuffer(RenderShotCtx const * ctx) const
    {
        for (size_t circularPassId = 0; circularPassId < kHostAheadCommandCount; circularPassId++)
        {
            for (size_t kickoffTileId = 0; kickoffTileId < ctx->kickoffTileCount(); kickoffTileId++)
            {
                auto const entry = ctx->kickoffEntry(circularPassId, kickoffTileId);

                cl_int error = clReleaseMemObject(entry->ctxBuffer);
                CS499R_ASSERT_NO_CL_ERROR(error);
            }
        }
    }


    // ------------------------------------------------------------------------- NASTY C++ WORK AROUNDS

    size_t const RenderState::kThreadsPerTilesTarget;
    size_t const RenderState::kWarpSizefactor;


}
