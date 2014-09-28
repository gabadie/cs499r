
#ifndef _H_CS499R_RENDERPROFILING
#define _H_CS499R_RENDERPROFILING

#include "cs499rBenchmark.hpp"


namespace CS499R
{

    /*
     * Render profiling contains all information about the render
     */
    struct RenderProfiling
    {
        // CPU duration
        timestamp_t mCPUDuration;

        // Ray shot
        uint64_t mRays;
    };

}


#endif // _H_CS499R_BENCHMARK
