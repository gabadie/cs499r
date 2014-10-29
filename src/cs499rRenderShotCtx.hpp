
#ifndef _H_CS499R_RENDERSHOTCTX
#define _H_CS499R_RENDERSHOTCTX

#include "cs499rCommonStruct.hpp"


namespace CS499R
{
    /*
     * Super tile: this a tile that is going be to downscaled at once
     *  contains kMaxKickoffTilePerSuperTileBorder^2 kickoff tiles
     *
     * Kickoff tile: the tile that is going be computed at once
     */

    size_t const kHostAheadCommandCount = 10;
    size_t const kMaxKickoffSampleIteration = 32;

    /*
     * Maximum number of super samples per border
     */
    size_t const kMaxVirtualPixelPerPixelBorder = 4;

    /*
     * Maximum number of kickoff-tile per super-tile border
     *  -> kMaxKickoffTilePerSuperTile = pow(kMaxKickoffTilePerSuperTileBorder, 2)
     */
    size_t const kMaxKickoffTilePerSuperTileBorder = 4;

    CS499R_STATIC_ASSERT(isPow2(kMaxKickoffTilePerSuperTileBorder));
    CS499R_STATIC_ASSERT((kMaxKickoffTilePerSuperTileBorder % kMaxVirtualPixelPerPixelBorder) == 0);

    /*
     * The render shot context is generate at each draw
     */
    class RenderShotCtx
    {
    public:
        // --------------------------------------------------------------------- MEMBERS

        // ----------------------------------------------- RAW MEMBERS
        /*
         * The final render target's resolution in pixels
         */
        size2_t renderTargetResolution;

        /*
         * The final render target's MSA per pixel
         */
        size_t pixelBorderSubdivisions;

        /*
         * The number of samples per MSA subdivions
         */
        size_t samplesPerSubdivisions;

        /*
         * The number of recursions per sample (== number of ray per sample)
         */
        size_t recursionPerSample;

        // ----------------------------------------------- KICKOFF MEMBERS
        /*
         * The kickoff tile's border size in virtual pixels
         */
        size_t kickoffTileSize;

        /*
         * The kickoff tile's OpenCL local size
         */
        size_t kickoffTileLocalSize;

        /*
         * The number of kickoff tile on the x and y axes in the virtual target
         */
        size2_t kickoffTileGrid;


        // --------------------------------------------------------------------- METHODS

        // ----------------------------------------------- RAW METHODS
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


        // ----------------------------------------------- SUPER SAMPLING
        /*
         * The number of virtual pixel per pixel border
         */
        inline
        size_t
        virtualPixelPerPixelBorder() const
        {
#if CS499R_CONFIG_ENABLE_SUPERSAMPLING
            return min(pixelBorderSubdivisions, kMaxVirtualPixelPerPixelBorder);
#else
            return 1;
#endif
        }

        /*
         * The resolution of the virtual target
         */
        inline
        size2_t
        virtualTargetResolution() const
        {
            return renderTargetResolution * virtualPixelPerPixelBorder();
        }

        /*
         * The virtual pixel's MSA per borders
         */
        inline
        size_t
        virtualPixelBorderSubdivisions() const
        {
            return pixelBorderSubdivisions / virtualPixelPerPixelBorder();
        }

        /*
         * The number of virtual pixel per super tile border
         */
        inline
        size_t
        virtualPixelPerSuperTileBorder() const
        {
            return kickoffTileSize * kMaxKickoffTilePerSuperTileBorder;
        }

        /*
         * The number of pixel per super tile border
         */
        inline
        size_t
        pixelPerSuperTileBorder() const
        {
            return virtualPixelPerSuperTileBorder() / virtualPixelPerPixelBorder();
        }

        /*
         * The number of super tile on the x and y axes in the final target
         */
        inline
        size2_t
        superTileGrid() const
        {
            return size2_t(
                (kickoffTileGrid.x + kMaxKickoffTilePerSuperTileBorder - 1) / kMaxKickoffTilePerSuperTileBorder,
                (kickoffTileGrid.y + kMaxKickoffTilePerSuperTileBorder - 1) / kMaxKickoffTilePerSuperTileBorder
            );
        }


        // ----------------------------------------------- KICKOFFSAMPLE
        /*
         * The number of sample per kickoff sample
         */
        inline
        size_t
        kickoffSampleIterationCount() const
        {
            return min(samplesPerSubdivisions, kMaxKickoffSampleIteration);
        }

        /*
         * The number of invocation of kickoff tile for a given MSA pos
         */
        inline
        size_t
        kickoffInvocationCount() const
        {
            CS499R_ASSERT((samplesPerSubdivisions % kickoffSampleIterationCount()) == 0)
            return samplesPerSubdivisions / kickoffSampleIterationCount();
        }

        /*
         * The kickoff tile's OpenCL global size
         */
        inline
        size_t
        kickoffTileGlobalSize() const
        {
            return kickoffTileSize * kickoffTileSize;
        }

        /*
         * The total number of kickoff tiles
         */
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
