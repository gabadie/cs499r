
#ifndef _CLH_CS499R_PROGRAM_SAMPLE_CONTEXT
#define _CLH_CS499R_PROGRAM_SAMPLE_CONTEXT

#include "cs499rProgramPrefix.h"


// ----------------------------------------------------------------------------- STRUCTS

typedef
__private struct sample_context_s
{
    // ray's origin in the mesh space
    float32x3_t raySceneOrigin;

    // normalized ray's direction in the mesh space
    float32x3_t raySceneDirection;

    // ray's origin in the mesh space
    float32x3_t rayMeshOrigin;

    // normalized ray's direction in the mesh space
    float32x3_t rayMeshDirection;

    // normalized ray's direction inverted
    float32x3_t rayMeshDirectionInverted;

    // ray's minimal distance found for depth test
    float32_t rayInterDistance;

    // the ray's normalized intersection normal in the mesh space
    float32x3_t rayInterMeshNormal;

    // the currently bound mesh instance
    __global common_mesh_instance_t const * rayInterMeshInstance;

    // the sample's random seed
    uint32_t randomSeed;

    // the currently bound mesh instance
    __global common_mesh_instance_t const * boundMeshInstance;

#ifdef _CL_DEBUG
    float32x3_t pixelColor;
#endif

} sample_context_t;


#endif // _CLH_CS499R_PROGRAM_SAMPLE_CONTEXT
