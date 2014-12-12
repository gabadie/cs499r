
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
            return mResolution.x;
        }

        inline
        size_t
        height() const
        {
            return mResolution.y;
        }

        inline
        size2_t const &
        resolution() const
        {
            return mResolution;
        }


        // --------------------------------------------------------------------- IDLE

        RenderTarget(RayTracer const * rayTracer, size_t width, size_t height);
        RenderTarget(RayTracer const * rayTracer, size_t squareWidth)
            : RenderTarget(rayTracer, squareWidth, squareWidth)
        { }

        ~RenderTarget();


    private:
        // --------------------------------------------------------------------- MEMBERS

        // the raytracer olding this target
        RayTracer const * const mRayTracer;

        // render target's dimensions
        size2_t const mResolution;

        // the render target's gpu memory space
        cl_mem mGpuBuffer;


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class RenderState;

    };

}

#endif // _H_CS499R_RENDERTARGET
