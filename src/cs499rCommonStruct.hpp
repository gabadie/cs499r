
#ifndef _H_CS499R_COMMON_STRUCTS
#define _H_CS499R_COMMON_STRUCTS

#include "cs499rMath.hpp"


#define CS499R_CL_STRUCTS(structs_code) \
    CS499R_CODE(structs_code); \
    structs_code

namespace CS499R
{

    static char const * const kCodeStructs = CS499R_CL_STRUCTS
    (

        typedef
        struct __attribute__((aligned(16)))
        common_mat3x4_s
        {
            __attribute__((aligned(16))) float32x3_t x;
            __attribute__((aligned(16))) float32x3_t y;
            __attribute__((aligned(16))) float32x3_t z;
            __attribute__((aligned(16))) float32x3_t w;
        } common_mat3x4_t;

        typedef
        struct __attribute__((aligned(16), packed))
        common_mesh_octree_node_s
        {
            // mesh's first primitive
            uint32_t subNodeOffsets[8];

            // node's first primitive within the mesh's primitives' array
            uint32_t primFirst;

            // node's primitives count
            uint32_t primCount;
        } common_mesh_octree_node_t;

        typedef
        struct __attribute__((aligned(16), packed))
        common_mesh_s
        {
            // mesh's first primitive
            uint32_t primFirst;

            // mesh's triangles count
            uint32_t primCount;
        } common_mesh_t;

        typedef
        struct __attribute__((aligned(16)))
        common_mesh_instance_s
        {
            // matrix from the mesh space to the scene space
            common_mat3x4_t meshSceneMatrix;

            // matrix from the scene space to the mesh space
            common_mat3x4_t sceneMeshMatrix;

            // the mesh instance's colors
            __attribute__((aligned(16))) float32x3_t diffuseColor;
            __attribute__((aligned(16))) float32x3_t emitColor;

            // the mesh's informations
            common_mesh_t mesh;
        } common_mesh_instance_t;

        typedef struct common_primitive_s
        {
            __attribute__((aligned(16))) float32x3_t v0;
            __attribute__((aligned(16))) float32x3_t v1;
            __attribute__((aligned(16))) float32x3_t v2;
        } common_primitive_t;

        typedef struct common_camera_s
        {
            __attribute__((aligned(16))) float32x3_t shotPosition;
            __attribute__((aligned(16))) float32x3_t shotBasisU;
            __attribute__((aligned(16))) float32x3_t shotBasisV;
            __attribute__((aligned(16))) float32x3_t focusPosition;
            __attribute__((aligned(16))) float32x3_t focusBasisU;
            __attribute__((aligned(16))) float32x3_t focusBasisV;
        } common_camera_t;

        typedef struct common_shot_context_s
        {
            // the shot's camera informations
            __attribute__((aligned(16))) common_camera_t camera;

            // render dimensions
            //      x is the image's width in pixels
            //      y is the image's height in pixels
            //      z is the image's subdivisions per pixels border
            __attribute__((aligned(16))) uint32x3_t render;

            // the max id of mesh instances in the scene
            __attribute__((aligned(16))) uint32_t meshInstanceMaxId;

        } common_shot_context_t;

    )

}


#endif // _H_CS499R_COMMON_STRUCTS
