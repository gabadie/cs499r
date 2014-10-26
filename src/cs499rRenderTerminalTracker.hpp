
#ifndef _H_CS499R_RENDERTERMINALTRACKER
#define _H_CS499R_RENDERTERMINALTRACKER

#include "cs499rBenchmark.hpp"
#include "cs499rRenderAbstractTracker.hpp"


namespace CS499R
{

    /*
     * The render terminal tracker is tracking the rendering into the console
     */
    class RenderTerminalTracker : public RenderAbstractTracker
    {
    protected:
        // --------------------------------------------------------------------- MEMBERS

        // flag at current == 0 and current == total
        timestamp_t mRenderingStartTime;
        timestamp_t mRenderingEndTime;

        // number of ray shot
        size_t mRayCount;


        // --------------------------------------------------------------------- ENTRIES

        void
        eventShotStart(RenderShotCtx const * ctx) override;

        void
        eventShotEnd() override;

        void
        eventShotProgress(size_t current, size_t total) override;

    };


}

#endif // _H_CS499R_RENDERTERMINALTRACKER
