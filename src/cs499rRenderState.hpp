
#ifndef _H_CS499R_RENDERSTATE
#define _H_CS499R_RENDERSTATE

#include "cs499rCommonStruct.hpp"
#include "cs499rEnums.hpp"


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
         * Shots the scene from the camera into the render target
         */
        void
        shotScene(SceneBuffer const * sceneBuffer, Camera const * camera, RenderProfiling * outProfiling = nullptr);


        // --------------------------------------------------------------------- CONSTS

        static size_t const kDefaultPixelBorderSubdivisions = 4;
        static size_t const kDefaultSamplesPerSubdivisions = 32;
        static size_t const kDefaultRecursionPerSample = 9;


        // --------------------------------------------------------------------- IDLE

        RenderState();
        ~RenderState();


    private:
        // --------------------------------------------------------------------- CONSTANTS

        static size_t const kHostAheadCommandCount = 10;
        static size_t const kMaxKickoffSampleIteration = 32;
        static size_t const kPathTracerRandomPerRay = 2;
        static size_t const kThreadsPerTilesTarget = 2048 * 8;


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

        void
        multiplyRenderTarget(float32_t multiplyFactor);


        // --------------------------------------------------------------------- SHOT STRUCT

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

        struct shot_ctx_t
        {
            size_t warpSize = 1;

            size_t pixelBorderSubdivisions;
            size_t samplesPerSubdivisions;
            size_t recursionPerSample;

            size_t pixelSubdivisions;
            size_t pixelSampleCount;
            size_t pixelRayCount;

            size_t kickoffSampleIterationCount;
            size_t kickoffInvocationCount;

            size_t coherencyTileSize;
            size_t kickoffTileSize;
            size_t kickoffTileGlobalSize;
            size_t kickoffTileLocalSize;
            size2_t kickoffTileGrid;
            size_t kickoffTileCount;

            common_render_context_t * kickoffCtxCircularArray;
            kickoff_ctx_manager_t * kickoffCtxManagers;
        };


        // --------------------------------------------------------------------- SHOT METHODES
        /*
         * Shots steps
         */
        void
        shotInit(shot_ctx_t * ctx) const;

        void
        shotInitTemplateCtx(
            shot_ctx_t const * ctx,
            SceneBuffer const * sceneBuffer,
            Camera const * camera,
            common_render_context_t * templateCtx
        ) const;

        void
        shotInitCircularCtx(
            shot_ctx_t const * ctx,
            common_render_context_t const * templateCtx
        ) const;

        void
        shotKickoff(
            shot_ctx_t const * ctx,
            SceneBuffer const * sceneBuffer,
            RenderProfiling * outProfiling
        );

        void
        shotFree(shot_ctx_t const * ctx) const;

    };

}

#endif // _H_CS499R_RENDERCONFIG
