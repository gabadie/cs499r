
#ifndef _H_CS499R_RENDERSTATE
#define _H_CS499R_RENDERSTATE

#include "cs499rPrefix.hpp"


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


        // --------------------------------------------------------------------- IDLE

        RenderState();
        ~RenderState();


    private:
        // --------------------------------------------------------------------- METHODES

        /*
         * Validates members
         */
        bool
        validateParams() const;

    };

}

#endif // _H_CS499R_RENDERCONFIG
