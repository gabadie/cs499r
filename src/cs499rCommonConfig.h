
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
 * Enables the octree node's children mask to quickly detect leaf nodes
 */
#ifndef CS499R_CONFIG_ENABLE_OCTREE_CHILDREN_MASK
# define CS499R_CONFIG_ENABLE_OCTREE_CHILDREN_MASK 1
#endif

/*
 * Enables the octree node's children access order
 */
#ifndef CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING
# define CS499R_CONFIG_ENABLE_OCTREE_SUBNODE_REORDERING 1
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
