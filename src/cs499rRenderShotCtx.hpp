
#ifndef _H_CS499R_RENDERSHOTCTX
#define _H_CS499R_RENDERSHOTCTX

#include "cs499rCommonStruct.hpp"


namespace CS499R
{

    size_t const kHostAheadCommandCount = 10;

    /*
     * The render shot context is generate at each draw
     */
    class RenderShotCtx
    {
    public:
        // --------------------------------------------------------------------- MEMBERS

        size_t pixelBorderSubdivisions;
        size_t samplesPerSubdivisions;
        size_t recursionPerSample;

        size_t pixelSubdivisions;
        size_t pixelSampleCount;
        size_t pixelRayCount;

        size_t kickoffSampleIterationCount;
        size_t kickoffInvocationCount;

        size_t coherencyTileSize;
        size_t kickoffTileSize;
        size_t kickoffTileGlobalSize;
        size_t kickoffTileLocalSize;
        size2_t kickoffTileGrid;
        size_t kickoffTileCount;


    private:
        // --------------------------------------------------------------------- STRUCTS
        struct kickoff_events_t
        {
            cl_event bufferWriteDone;
            cl_event kickoffDone;
        };

        struct kickoff_ctx_manager_t
        {
            cl_mem buffers[kHostAheadCommandCount];
            kickoff_events_t events[kHostAheadCommandCount];
        };


        // --------------------------------------------------------------------- MEMBERS

        common_render_context_t * kickoffCtxCircularArray;
        kickoff_ctx_manager_t * kickoffCtxManagers;


        // --------------------------------------------------------------------- FRIENDSHIPS

        friend class RenderState;

    };


}

#endif // _H_CS499R_SCENE
