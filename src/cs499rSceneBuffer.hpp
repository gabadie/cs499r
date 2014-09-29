
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
            cl_mem primitives;
            cl_mem meshInstances;
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


        // --------------------------------------------------------------------- FRIENDSHIP
        friend class RenderState;

    };

}

#endif // _H_CS499R_SCENEBUFFER
