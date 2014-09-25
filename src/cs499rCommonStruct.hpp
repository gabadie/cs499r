
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

        typedef struct mat4_s
        {
            float32x4_t c[4];
        } mat4_t;

        typedef struct mesh_s
        {
            uint32_t firstPrime; // first triangle ID
            uint32_t primeCount; // triangles count
        } mesh_t;

        typedef struct mesh_instance_s
        {
            mat4_t meshToSpace;
            mat4_t spaceToMesh;
            uint32_t meshId;
        } mesh_instance_t;

        typedef struct common_triangle_s
        {
            __attribute__((aligned(16))) float32x3_t v0;
            __attribute__((aligned(16))) float32x3_t v1;
            __attribute__((aligned(16))) float32x3_t v2;
            __attribute__((aligned(16))) float32x3_t diffuseColor;
            __attribute__((aligned(16))) float32x3_t emitColor;
        } common_triangle_t;

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
            //      w is the image's samples computed per threads
            __attribute__((aligned(16))) uint32x4_t render;

            // the number of triangles in the scene
            __attribute__((aligned(16))) uint32_t triangleCount;

        } common_shot_context_t;

    )

}


#endif // _H_CS499R_COMMON_STRUCTS
