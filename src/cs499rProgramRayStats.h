
#ifndef _CLH_CS499R_PROGRAM_RAY_STATS
#define _CLH_CS499R_PROGRAM_RAY_STATS
#ifdef _CL_PROGRAM_RAY_STATS

#include "cs499rProgramPrefix.h"



/*
 * The stat output's factor
 */
#define CS499R_CONFIG_RAY_STATS_FACTOR 0.01f

/*
 * Stats on primitive_intersection() calls
 */
//#define CS499R_STATS_PRIM_INTERSECTION

/*
 * Stats on loops in octree_tmplt_intersection() or
 * in scene_octree_one_loop_intersection()
 */
//#define CS499R_STATS_OCTREE_LOOPS

/*
 * Stats on mesh browsed in mesh_instance_intersection() or
 * in scene_octree_one_loop_intersection()
 */
//#define CS499R_STATS_OCTREE_MESH_BROWSING

/*
 * Stats on octree node browsed in octree_tmplt_intersection() or
 * in scene_octree_one_loop_intersection()
 */
#define CS499R_STATS_OCTREE_NODE_BROWSING

#endif //_CL_PROGRAM_RAY_STATS
#endif //_CLH_CS499R_PROGRAM_RAY_STATS
