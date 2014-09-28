
#ifndef _H_CS499R_RENDERTARGET
#define _H_CS499R_RENDERTARGET

#include <vector>
#include "cs499rCommonStruct.hpp"


namespace CS499R
{

    /*
     * The render target old newly computed texels
     */
    class RenderTarget
    {
    public:
        // --------------------------------------------------------------------- CONSTANTS

        // number of chanels in a render target
        static size_t const kChanelCount = 3;


        // --------------------------------------------------------------------- METHOD

        /*
         * Downloads render target into an image
         */
        void
        download(Image * outImage, float colorFactor = 1.0f) const;

        /*
         * Getters
         */
        inline
        size_t
        width() const
        {
            return mWidth;
        }

        inline
        size_t
        height() const
        {
            return mHeight;
        }


        // --------------------------------------------------------------------- IDLE

        RenderTarget(RayTracer const * rayTracer, size_t width, size_t height);
        ~RenderTarget();


    private:
        // --------------------------------------------------------------------- MEMBERS

        // the raytracer olding this target
        RayTracer const * const mRayTracer;

        // render target's dimensions
        size_t const mWidth;
        size_t const mHeight;

        // the render target's gpu memory space
        cl_mem mGpuBuffer;


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class RenderState;

    };

}

#endif // _H_CS499R_RENDERTARGET
