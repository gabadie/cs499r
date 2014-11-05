
#ifndef _H_CS499R_COMMON_CONFIG
#define _H_CS499R_COMMON_CONFIG

/*
 * Caution: this file will be also included in OpenCL programs' preprocessor
 */


/*
 * Enables mesh's back face culling
 */
#ifndef CS499R_CONFIG_ENABLE_BACKFACE_CULLING
# define CS499R_CONFIG_ENABLE_BACKFACE_CULLING 1
#endif

/*
 * Enables mesh's bounding box checking
 */
#ifndef CS499R_CONFIG_ENABLE_MESH_BOUNDING_BOX
# define CS499R_CONFIG_ENABLE_MESH_BOUNDING_BOX 1
#endif

/*
 * Enables mesh's octree
 */
#ifndef CS499R_CONFIG_ENABLE_MESH_OCTREE
# define CS499R_CONFIG_ENABLE_MESH_OCTREE 1
#endif

/*
 * Enables octree node's children access lists
 *
 *  This optimisation list all available subnodes in a node. This has the
 *  ability to reduce the conditions serialisation caused by testing if a
 *  subnode i exists when browsing an octree.
 */
#ifndef CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
# define CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS 1
#endif

/*
 * Enables the octree node's children access order
 */
#ifndef CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
# define CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING 1
#endif

/*
 * Stores the octree node's children consecutively in the memory
 *
 *  This optimisation slightly reduce the cache's number of page fault by
 *  storing a node's direct children consecutively in the memory
 */
#ifndef CS499R_CONFIG_ENABLE_OCTREE_CONSECUTIVE_SUBNODES
# define CS499R_CONFIG_ENABLE_OCTREE_CONSECUTIVE_SUBNODES 1
#endif

 /*
  * Merge the scene octree and mesh octree intersection computation in one loop
  *
  *  This optimisation has the hability to de-synchronise the warp threads when
  *  iterating over differents meshes. This way, it reduces the GPU idle time
  *  caused by warp threads' execution mask set by conditions.
  */
#ifndef CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP
# define CS499R_CONFIG_ENABLE_OCTREE_ONE_LOOP 1
#endif

/*
 * Optimises the octree's leaves that to few primitives
 *
 *  This optimisation removes leaves that have to few primitives. This has the
 *  hability to compress the octree's memory size and not let the raytracer test
 *  many cube intersections that are not worth it.
 */
#ifndef CS499R_CONFIG_ENABLE_OCTREE_OPTIMISATION_STAGE
# define CS499R_CONFIG_ENABLE_OCTREE_OPTIMISATION_STAGE 1
#endif

/*
 * Enables the super-sampling rendering
 *
 *  This optimisation renders per tile, but with a bigger resolution, so that
 *  the CBT and ICBT coherency tiles' area is reduced. Then each tile are
 *  downscaled on the final render target. But it also decreases the number of
 *  diverging samples per final pixel.
 */
#ifndef CS499R_CONFIG_ENABLE_SUPERSAMPLING
# define CS499R_CONFIG_ENABLE_SUPERSAMPLING 0
#endif

/*
 * Enables scene's octree
 */
#ifndef CS499R_CONFIG_ENABLE_SCENE_OCTREE
# define CS499R_CONFIG_ENABLE_SCENE_OCTREE 1
#endif


/*
 * Configures the pixels gathering strategy per warp
 */
#define CS499R_CONFIG_PIXELPOS_DUMMY 0
#define CS499R_CONFIG_PIXELPOS_CPT 1
#define CS499R_CONFIG_PIXELPOS_ICPT 2

#ifndef CS499R_CONFIG_PIXELPOS
# define CS499R_CONFIG_PIXELPOS CS499R_CONFIG_PIXELPOS_ICPT
#endif


#endif // _H_CS499R_COMMON_CONFIG
