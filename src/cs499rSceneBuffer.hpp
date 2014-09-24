
#ifndef _H_CS499R_SCENEBUFFER
#define _H_CS499R_SCENEBUFFER

#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * The scene buffer old all the scene information in buffers on the GPU side
     */
    class SceneBuffer
    {
    public:
        // --------------------------------------------------------------------- METHODES

        /*
         * Renders the scene into the given render target with a given camera
         */
        void
        render(RenderTarget * target, Camera const * camera) const;


        // --------------------------------------------------------------------- IDLE

        SceneBuffer(Scene const * scene, RayTracer const * rayTracer);
        ~SceneBuffer();


    private:
        // --------------------------------------------------------------------- MEMBERS

        // the scene source
        Scene const * const mScene;

        // the binded ray tracer
        RayTracer const *  const mRayTracer;

        // the buffers
        struct
        {
            cl_mem triangles;
        } mBuffer;


        // --------------------------------------------------------------------- METHODES

        /*
         * Creates GPU side buffers
         */
        void
        createBuffers();

        /*
         * Releases GPU side buffers
         */
        void
        releaseBuffers();

    };

}

#endif // _H_CS499R_SCENEBUFFER
