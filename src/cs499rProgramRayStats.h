
#ifndef _CLH_CS499R_PROGRAM_RAY_STATS
#define _CLH_CS499R_PROGRAM_RAY_STATS
#ifdef _CL_PROGRAM_RAY_STATS

#include "cs499rProgramPrefix.h"



/*
 * The stat output's factor
 */
#define CS499R_CONFIG_RAY_STATS_FACTOR (0.02f, 0.0004f, 0.001f)

/*
 * Disables a stats output
 */
#define CS499R_DISABLE_STATS 4

/*
 * Stats on primitive_intersection() calls
 */
#define CS499R_STATS_PRIM_INTERSECTION 1

/*
 * Stats on loops in octree_tmplt_intersection() or
 * in scene_octree_one_loop_intersection()
 */
#define CS499R_STATS_OCTREE_LOOPS CS499R_DISABLE_STATS

/*
 * Stats on mesh browsed in mesh_instance_intersection() or
 * in scene_octree_one_loop_intersection()
 */
#define CS499R_STATS_OCTREE_MESH_BROWSING CS499R_DISABLE_STATS

/*
 * Stats on octree node browsed in octree_tmplt_intersection() or
 * in scene_octree_one_loop_intersection()
 */
#define CS499R_STATS_OCTREE_NODE_BROWSING 0

#define sample_stats_name(sampleCtx,name,op) \
    if (CS499R_STATS_##name < CS499R_DISABLE_STATS) \
    {\
        sampleCtx->stats[CS499R_STATS_##name] op;\
    }

#define sample_stats_id(sampleCtx,id,op) \
    (sampleCtx->stats[id] op)

#else

#define sample_stats_name(sampleCtx,name,op)
#define sample_stats_id(sampleCtx,id,op)

#endif //_CL_PROGRAM_RAY_STATS
#endif //_CLH_CS499R_PROGRAM_RAY_STATS
