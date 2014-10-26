
#ifndef _CLH_CS499R_PROGRAM_PIXEL_POS
#define _CLH_CS499R_PROGRAM_PIXEL_POS

#include "cs499rProgramPathTracer.h"


/*
 * Coherent path tracing algorithm coherent tiles
 *
 *
 *      |    kickoff tile   |
 *           (0,1)
 *  --   -------------------   --  --
 *      |                   |
 *      |    kickoff tile   | kickoff tile
 *      |    (0,0)          | (1,0)
 *      |                   |
 *      |                   |
 *      |     coherency     |
 *      |---- tile (0, 0)   |
 *      |    |              |
 *      |    |              |
 *  --   -------------------   --  --
 *
 *      |                   |
 */

/*
 * Tiled Coherent Path Tracing
 */
inline
uint32x2_t
kernel_pixel_pos_cpt(
    __global common_render_context_t const * const coherencyCx,
    sample_context_t * const sampleCx
)
{
    // the coherency tile id in the kick of tile
    uint32_t const kickoffTileCoherencyTileId = get_group_id(0) >> coherencyCx->render.cpt.groupPerCoherencyTile.log;

    // the coherency tile pos in the kick of tile
    uint32x2_t const kickoffTileCoherencyTilePos = (uint32x2_t)(
        kickoffTileCoherencyTileId & (coherencyCx->render.cpt.coherencyTilePerKickoffTileBorder.value - 1),
        kickoffTileCoherencyTileId >> coherencyCx->render.cpt.coherencyTilePerKickoffTileBorder.log
    );

    // the thread id in the coherency tile
    uint32_t const coherencyTileGroupId = get_group_id(0) & (coherencyCx->render.cpt.groupPerCoherencyTile.value - 1);

    // the thread id in the coherency tile
    uint32_t const coherencyTileThreadId = get_local_id(0) + get_local_size(0) * coherencyTileGroupId;

    // the pixel id in the coherency tile
    uint32_t const coherencyTilePixelId = coherencyTileThreadId;

    // the pixel pos in the coherency tile
    uint32x2_t const coherencyTilePixelPos = (uint32x2_t)(
        coherencyTilePixelId & (coherencyCx->render.cpt.coherencyTileSize.value - 1),
        coherencyTilePixelId >> coherencyCx->render.cpt.coherencyTileSize.log
    );

    // the pixel pos in the kickoff tile
    uint32x2_t const kickoffTilePixelPos = (
        kickoffTileCoherencyTilePos * coherencyCx->render.cpt.coherencyTileSize.value +
        coherencyTilePixelPos
    );

    { // set up random seed
        sampleCx->randomSeed = (
            kPathTracerRandomPerRay *
            coherencyCx->render.kickoffSampleRecursionCount *
            coherencyCx->render.passId
        );
    }

    // the pixel pos
    return kickoffTilePixelPos + coherencyCx->render.kickoffTilePos;
}

/*
 * Tiled Interleaved Coherent Path Tracing
 */
inline
uint32x2_t
kernel_pixel_pos_icpt(
    __global common_render_context_t const * const coherencyCx,
    sample_context_t * const sampleCx
)
{
    // the coherency tile id in the kick of tile
    uint32_t const kickoffTileCoherencyTileId = get_group_id(0) >> coherencyCx->render.icpt.groupPerCoherencyTile.log;

    // the coherency tile pos in the kick of tile
    uint32x2_t const kickoffTileCoherencyTilePos = (uint32x2_t)(
        kickoffTileCoherencyTileId & (coherencyCx->render.icpt.coherencyTilePerKickoffTileBorder.value - 1),
        kickoffTileCoherencyTileId >> coherencyCx->render.icpt.coherencyTilePerKickoffTileBorder.log
    );

    // the group id in the coherency tile
    uint32_t const coherencyTileGroupId = get_group_id(0) & (coherencyCx->render.icpt.groupPerCoherencyTile.value - 1);

    // the thread id in the coherency tile
    uint32_t const coherencyTileThreadId = get_local_id(0) + get_local_size(0) * coherencyTileGroupId;

    // the pixel id in the coherency tile
    uint32_t const coherencyTilePixelId = coherencyTileThreadId;

    uint32_t const coherencyTilePixelPosY = latin_hypercube_x_pos(
        (2 * coherencyCx->render.passId + 0) & (kLatinHypercubeSize - 1),
        coherencyTilePixelId & (kLatinHypercubeSize - 1)
    );
    uint32_t const coherencyTileInterleaveId = latin_hypercube_x_pos(
        (2 * coherencyCx->render.passId + 1) & (kLatinHypercubeSize - 1),
        coherencyTilePixelId >> kLatinHypercubeSizeLog
    );

    // the pixel pos in the coherency tile
    uint32x2_t const coherencyTilePixelPos = (uint32x2_t)(
        latin_hypercube_x_pos(coherencyTileInterleaveId, coherencyTilePixelPosY),
        coherencyTilePixelPosY
    );

    // the pixel pos in the kickoff tile
    uint32x2_t const kickoffTilePixelPos = (
        kickoffTileCoherencyTilePos * kLatinHypercubeSize +
        coherencyTilePixelPos
    );

    { // set up random seed
        sampleCx->randomSeed = (
            kPathTracerRandomPerRay *
            coherencyCx->render.kickoffSampleRecursionCount *
            coherencyCx->render.passId *
            coherencyCx->render.icpt.groupPerCoherencyTile.value +
            coherencyTileGroupId
        );
    }

    // the pixel pos
    return kickoffTilePixelPos + coherencyCx->render.kickoffTilePos;
}

#endif // _CLH_CS499R_PROGRAM_PIXEL_POS
