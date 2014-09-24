
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
            float4 c[4];
        } mat4_t;

        typedef struct mesh_s
        {
            unsigned int firstPrime; // first triangle ID
            unsigned int primeCount; // triangles count
        } mesh_t;

        typedef struct mesh_instance_s
        {
            mat4_t meshToSpace;
            mat4_t spaceToMesh;
            unsigned int meshId;
        } mesh_instance_t;

        typedef struct triangle_s
        {
            float3 vertex[3];
            float3 diffuseColor;
            float3 emitColor;
        } triangle_t;

    )

}


#endif // _H_CS499R_COMMON_STRUCTS
