
#ifndef _H_CS499R_COMMON_STRUCTS
#define _H_CS499R_COMMON_STRUCTS

#include "cs499rMath.hpp"


#define CS499R_CL_STRUCT(struct_code) \
    CS499R_CODE(struct_code); \
    struct_code

/*
 * Defines a triangle
 */
static char const * kCodeStructTriangle = CS499R_CL_STRUCT
(
    typedef struct triangle_s
    {
        float3 vertex[3];
        float3 difuseColor;
        float3 emitColor;
    } triangle_t;
)


#endif // _H_CS499R_COMMON_STRUCTS
