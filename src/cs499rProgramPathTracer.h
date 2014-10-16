
#ifndef _CLH_CS499R_PROGRAM_PATH_TRACER
#define _CLH_CS499R_PROGRAM_PATH_TRACER

#include "cs499rProgramConsts.h"
#include "cs499rProgramSampleContext.h"
#include "cs499rProgramUtils.h"


// ----------------------------------------------------------------------------- CONSTANTES

__constant float32_t kRecursionCount = 9;
__constant float32_t kRandomPerRecursion = 2;


// ----------------------------------------------------------------------------- FUNCTIONS

inline
void
path_tracer_rebound(
    sample_context_t * const sampleCx
)
{
    sampleCx->raySceneOrigin += sampleCx->raySceneDirection * sampleCx->rayInterDistance;

    __global common_mesh_instance_t const * const meshInstance = sampleCx->rayInterMeshInstance;

    float32x3_t const meshNormal = sampleCx->rayInterMeshNormal;
    float32x3_t const n = (
        meshInstance->meshSceneMatrix.x * meshNormal.x +
        meshInstance->meshSceneMatrix.y * meshNormal.y +
        meshInstance->meshSceneMatrix.z * meshNormal.z
    );

    float32x3_t u;

    if (fabs(n.x) > fabs(n.y))
    {
        u = (float32x3_t)(-n.z, 0.0f, n.x);
    }
    else
    {
        u = (float32x3_t)(0.0f, -n.z, n.y);
    }

    float32x3_t const u2 = u * u;
    float32_t const r2 = random(sampleCx);
    float32_t const r2s = sqrt(r2) * rsqrt(u2.x + u2.y + u2.z);

    float32x3_t const v = cross(n, u);
    float32_t const r1 = 2.0f * kPi * random(sampleCx);

    sampleCx->raySceneDirection = (
        u * (cos(r1) * r2s) +
        v * (sin(r1) * r2s) +
        n * sqrt(1.0f - r2)
    );
}


#endif // _CLH_CS499R_PROGRAM_PATH_TRACER
