
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

        static size_t const kMaxKickoffSampleIteration = 32;
        static size_t const kThreadsPerTilesTarget = 2048 * 8;
        static size_t const kWarpSizefactor = 2;


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
        shotInitCircularCtx(
            RenderShotCtx const * ctx,
            common_render_context_t const * templateCtx
        ) const;

        void
        shotKickoff(
            RenderShotCtx const * ctx,
            SceneBuffer const * sceneBuffer,
            RenderProfiling * outProfiling
        );

        void
        shotFree(RenderShotCtx const * ctx) const;

    };

}

#endif // _H_CS499R_RENDERCONFIG
