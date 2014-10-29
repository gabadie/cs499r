
#ifndef _H_CS499R_RENDERSHOTCTX
#define _H_CS499R_RENDERSHOTCTX

#include "cs499rCommonStruct.hpp"


namespace CS499R
{

    size_t const kHostAheadCommandCount = 10;
    size_t const kMaxKickoffSampleIteration = 32;

    /*
     * The render shot context is generate at each draw
     */
    class RenderShotCtx
    {
    public:
        // --------------------------------------------------------------------- MEMBERS

        size2_t renderResolution;
        size_t pixelBorderSubdivisions;
        size_t samplesPerSubdivisions;
        size_t recursionPerSample;

        size_t kickoffTileSize;
        size_t kickoffTileLocalSize;
        size2_t kickoffTileGrid;


        // --------------------------------------------------------------------- FUNCTIONS

        inline
        size_t
        pixelSubdivisions() const
        {
            return pixelBorderSubdivisions * pixelBorderSubdivisions;
        }

        inline
        size_t
        pixelSampleCount() const
        {
            return pixelSubdivisions() * samplesPerSubdivisions;
        }

        inline
        size_t
        pixelRayCount() const
        {
            return pixelSampleCount() * recursionPerSample;
        }

        inline
        size_t
        kickoffSampleIterationCount() const
        {
            return min(samplesPerSubdivisions, kMaxKickoffSampleIteration);
        }

        inline
        size_t
        kickoffInvocationCount() const
        {
            CS499R_ASSERT((samplesPerSubdivisions % kickoffSampleIterationCount()) == 0)
            return samplesPerSubdivisions / kickoffSampleIterationCount();
        }

        inline
        size_t
        kickoffTileGlobalSize() const
        {
            return kickoffTileSize * kickoffTileSize;
        }

        inline
        size_t
        kickoffTileCount() const
        {
            return kickoffTileGrid.x * kickoffTileGrid.y;
        }


    private:
        // --------------------------------------------------------------------- STRUCTS
        struct kickoff_entry_t
        {
            common_render_context_t ctx;
            cl_mem ctxBuffer;
            cl_event bufferWriteDone;
            cl_event kickoffDone;
        };


        // --------------------------------------------------------------------- MEMBERS

        kickoff_entry_t * kickoffEntries;


        // --------------------------------------------------------------------- METHOD

        /*
         * Allocs kickoff entries
         */
        inline
        void
        allocKickoffEntries()
        {
            kickoffEntries = alloc<kickoff_entry_t>(kickoffTileCount() * kHostAheadCommandCount);
        }

        /*
         * Gets an entry from the stream pass ID and tile ID
         */
        inline
        kickoff_entry_t *
        kickoffEntry(size_t streamPassId, size_t tileId) const
        {
            CS499R_ASSERT(streamPassId <= kHostAheadCommandCount);
            CS499R_ASSERT(tileId <= kickoffTileCount());

            return kickoffEntries + streamPassId * kickoffTileCount() + tileId;
        }


        // --------------------------------------------------------------------- IDLE

        RenderShotCtx()
        {
            kickoffEntries = nullptr;
        }

        ~RenderShotCtx()
        {
            if (kickoffEntries != nullptr)
            {
                free(kickoffEntries);
            }
        }

        // --------------------------------------------------------------------- FRIENDSHIPS

        friend class RenderState;

    };


}

#endif // _H_CS499R_RENDERSHOTCTX
