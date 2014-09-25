
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


        // --------------------------------------------------------------------- IDLE

        RenderState();
        ~RenderState();

    };

}

#endif // _H_CS499R_RENDERCONFIG
