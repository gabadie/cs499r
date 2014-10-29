
#ifndef _H_CS499R_RENDERSTATE
#define _H_CS499R_RENDERSTATE

#include "cs499rCommonStruct.hpp"
#include "cs499rEnums.hpp"
#include "cs499rRenderShotCtx.hpp"


namespace CS499R
{

    /*
     * The render state is in charge of the render
     */
    class RenderState
    {
    public:
        // --------------------------------------------------------------------- MEMBERS

        // number of pixel subdivisions on the borders
        size_t mPixelBorderSubdivisions;

        // number of samples per pixel subdivisions
        size_t mSamplesPerSubdivisions;

        // number of samples ray recursion per sample > 0
        size_t mRecursionPerSample;

        // the ray tracer algorithm to use
        RayAlgorithm mRayAlgorithm;

        // the render target to render to
        RenderTarget * mRenderTarget;


        // --------------------------------------------------------------------- METHODES

        /*
         * Downscale to the current render target
         */
        void
        downscale(RenderTarget const * srcRenderTarget);

        /*
         * Shots the scene from the camera into the render target
         */
        void
        shotScene(SceneBuffer const * sceneBuffer, Camera const * camera, RenderAbstractTracker * renderTracker);


        // --------------------------------------------------------------------- CONSTS

        static size_t const kDefaultPixelBorderSubdivisions = 4;
        static size_t const kDefaultSamplesPerSubdivisions = 32;
        static size_t const kDefaultRecursionPerSample = 9;


        // --------------------------------------------------------------------- IDLE

        RenderState();
        ~RenderState();


    private:
        // --------------------------------------------------------------------- CONSTANTS

        static size_t const kThreadsPerTilesTarget = 2048 * 8;
        static size_t const kWarpSizefactor = 2;


        // --------------------------------------------------------------------- CONSTANTS

        struct ShotIteration
        {
            size_t invocationId;
            size_t kickoffTileId;
            size2_t subPixel;
        };


        // --------------------------------------------------------------------- METHODES

        /*
         * Validates members
         */
        bool
        validateParams() const;

        /*
         * Render targets operations
         */
        void
        clearRenderTarget();

        /*
         * Downscale to the current render target
         */
        void
        downscale(
            RenderTarget * destRenderTarget,
            RenderTarget const * srcRenderTarget,
            size2_t srcPos,
            size2_t srcSize,
            size2_t destPos,
            size2_t destSize,
            float32_t multiplyFactor = 1.0f,
            cl_uint eventWaitListSize = 0,
            cl_event const * eventWaitList = nullptr,
            cl_event * event = nullptr
        );

        /*
         * Multiplies the current render target
         */
        void
        multiplyRenderTarget(float32_t multiplyFactor);


        // --------------------------------------------------------------------- SHOT METHODES
        /*
         * Shots steps
         */
        void
        shotInit(RenderShotCtx * ctx) const;

        void
        shotInitTemplateCtx(
            RenderShotCtx const * ctx,
            SceneBuffer const * sceneBuffer,
            Camera const * camera,
            common_render_context_t * templateCtx
        ) const;

        void
        shotInitKickoffEntries(
            RenderShotCtx const * ctx,
            common_render_context_t const * templateCtx
        ) const;

        void
        shotAllocKickoffEntriesBuffer(
            RenderShotCtx const * ctx
        ) const;

        void
        shotKickoff(
            RenderShotCtx const * ctx,
            SceneBuffer const * sceneBuffer,
            RenderAbstractTracker * renderTracker
        );

        void
        shotUpdateKickoffRenderCtx(
            RenderShotCtx const * ctx,
            ShotIteration const * shotIteration,
            common_render_context_t * renderCtx,
            cl_command_queue cmdQueue,
            cl_mem renderCtxBuffer,
            cl_uint eventWaitListSize,
            cl_event const * eventWaitList ,
            cl_event * event
        ) const;

        void
        shotKickoffRenderCtx(
            RenderShotCtx const * ctx,
            cl_command_queue cmdQueue,
            cl_kernel kernel,
            cl_mem renderCtxBuffer,
            cl_uint eventWaitListSize,
            cl_event const * eventWaitList ,
            cl_event * event
        ) const;

        void
        shotWaitKickoffEntry(
            RenderShotCtx::kickoff_entry_t * entry
        ) const;

        void
        shotFreeKickoffEntriesBuffer(RenderShotCtx const * ctx) const;

    };

}

#endif // _H_CS499R_RENDERCONFIG
