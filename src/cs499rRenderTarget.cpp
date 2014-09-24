
#include <iostream>
#include "cs499rImage.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rRenderTarget.hpp"


namespace CS499R
{

    void
    RenderTarget::download(Image * outImage, float colorFactor) const
    {
        CS499R_ASSERT(outImage);
        CS499R_ASSERT(outImage->width() == mWidth);
        CS499R_ASSERT(outImage->height() == mHeight);
        CS499R_ASSERT(outImage->chanels() == kChanelCount);
        CS499R_ASSERT(outImage->depth() == 8 || outImage->depth() == 16);

        size_t const texelComponents = mWidth * mHeight * kChanelCount;
        float * rawTexels = (float *) malloc(texelComponents * sizeof(float));

        cl_int error = clEnqueueReadBuffer(mRayTracer->mCmdQueue,
            mGpuBuffer, CL_TRUE,
            0, sizeof(cl_float) * texelComponents, rawTexels,
            0, NULL, NULL
        );

        CS499R_ASSERT(error == 0);

        if (outImage->depth() == 16)
        {
            uint16_t * destTexels = outImage->texels<uint16_t>();

            for (size_t i = 0; i < texelComponents; i++)
            {
                float value = rawTexels[i] * colorFactor;

                if (value < 0.0f)
                {
                    value = 0.0f;
                }
                else if (value > 1.0f)
                {
                    value = 1.0f;
                }

                uint64_t uValue = (value * 65535.0f);

                if (uValue > 65535)
                {
                    uValue = 65535;
                }

                destTexels[i] = uint16_t(uValue);
            }
        }
        else if (outImage->depth() == 8)
        {
            uint8_t * destTexels = outImage->texels<uint8_t>();

            for (size_t i = 0; i < texelComponents; i++)
            {
                float value = rawTexels[i] * colorFactor;

                if (value < 0.0f)
                {
                    value = 0.0f;
                }
                else if (value > 1.0f)
                {
                    value = 1.0f;
                }

                uint64_t uValue = (value * 255.0f);

                if (uValue > 255)
                {
                    uValue = 255;
                }

                destTexels[i] = uint8_t(uValue);
            }
        }

        free(rawTexels);
    }

    RenderTarget::RenderTarget(RayTracer const * rayTracer, size_t width, size_t height)
        : mRayTracer(rayTracer)
        , mWidth(width)
        , mHeight(height)
    {
        CS499R_ASSERT(width * height != 0);

        cl_int error = 0;

        mGpuBuffer = clCreateBuffer(
            mRayTracer->mContext,
            CL_MEM_WRITE_ONLY,
            sizeof(cl_float) * mWidth * mHeight * kChanelCount,
            NULL,
            &error
        );

        CS499R_ASSERT(error == 0);
    }

    RenderTarget::~RenderTarget()
    {
        CS499R_ASSERT(mGpuBuffer != nullptr);

        cl_int error = clReleaseMemObject(mGpuBuffer);

        CS499R_ASSERT(error == 0);
    }

}
