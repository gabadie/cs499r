
#include <iostream>
#include <iomanip>

#include "cs499rRenderTerminalTracker.hpp"


namespace CS499R
{

    size_t const kProgressPrecision = 1;
    size_t const kDurationPrecision = 1;

    void
    RenderTerminalTracker::eventShotStart(RenderShotCtx const * ctx)
    {
        mRayCount = ctx->renderResolution.x * ctx->renderResolution.y * ctx->pixelRayCount();

        std::cout << "Input:" << std::endl;
        std::cout << "    Render width:             " << ctx->renderResolution.x << " px" << std::endl;
        std::cout << "    Render height:            " << ctx->renderResolution.y << " px" << std::endl;
        std::cout << "    Sub-pixels:               " << ctx->pixelSubdivisions() << std::endl;
        std::cout << "    Sample per sub-pixels:    " << ctx->samplesPerSubdivisions << std::endl;
        std::cout << "    Recursion per samples:    " << ctx->recursionPerSample << std::endl;
        std::cout << "    Total Rays:               " << mRayCount / 1000000 << " M" << std::endl;
        std::cout << std::endl;

        std::cout << "Rendering..." << std::endl;
    }

    void
    RenderTerminalTracker::eventShotEnd()
    {
        size_t const duration = mRenderingEndTime - mRenderingStartTime;

        // save std::cout's state
        auto const previousPrecision = std::cout.precision(5);

        std::cout << std::setprecision(kDurationPrecision);
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Output:" << std::endl;
        std::cout << "    CPU duration:             " << 0.000001f * duration << " s" << std::endl;
        std::cout << "    CPU duration per ray:     " <<
            1000.0 * double(duration) / double(mRayCount) << " ns" << std::endl;
        std::cout << std::endl;

        // restore std::cout
        std::cout << std::setprecision(previousPrecision);
    }

    void
    RenderTerminalTracker::eventShotProgress(size_t current, size_t total)
    {
        auto const currentTime = timestamp();

        if (current == 0)
        {
            mRenderingStartTime = currentTime;
        }
        else if (current == total)
        {
            mRenderingEndTime = currentTime;
        }

        // save std::cout's state
        auto const previousWidth = std::cout.width();
        auto const previousPrecision = std::cout.precision(5);

        // clean line
        std::cout << "\r    ";

        float32_t const progress = float32_t(current) / float32_t(total);

        { // show progress
            std::cout << std::setw(4 + kProgressPrecision);
            std::cout << std::setprecision(kProgressPrecision);
            std::cout << std::fixed;
            std::cout << std::right;
            std::cout << 100.0f * progress;
            std::cout << std::left;
            std::cout << std::setw(previousWidth);
            std::cout << "%";
        }

        size_t const duration = currentTime - mRenderingStartTime;
        float32_t const remainingDuration = (float32_t(duration) / progress) * (1.0f - progress);

        if (current != 0)
        {
            std::cout << "  (remaining: ~";
            std::cout << std::setprecision(kDurationPrecision);
            std::cout << std::fixed;
            std::cout << 0.000001f * remainingDuration;
            std::cout << " s)        \r";
        }

        // restore std::cout
        std::cout << std::setprecision(previousPrecision);
        std::cout << std::flush;
    }


}
