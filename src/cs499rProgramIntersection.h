
#ifndef _CLH_CS499R_PROGRAM_INTERSECTION
#define _CLH_CS499R_PROGRAM_INTERSECTION

#include "cs499rProgramSampleContext.h"


// ----------------------------------------------------------------------------- FUNCTIONS

/*
 *      (triangle)
 *          |
 *    \     |
 *      \   |
 *        \ |             (normal)
 *          I--------------->
 *          | \
 *          |   \
 *          |     \
 *          |       \
 *          |         \
 *          A           \   (ray)
 *          |  \          \
 *          |     \         \
 *          |        \        \
 *          |           \       \       ->
 *          |              \      \ __  d
 *          |                 \    |\
 *          |                    \    \
 *          |                       \   \
 *          |                          \  \
 *          |                             \ \
 *          |                                \\
 *          H  -  -  -  -  -  -  -  -  -  -  -  O
 *          |
 *          |
 *
 *
 * Glossary:
 *      Point `O` is the origin of the ray (sampleCx->rayMeshOrigin)
 *      Vector `d` is direction of the ray (sampleCx->rayMeshDirection)
 *      Point `A` is the first vertex of the current triangle (primitive->v0)
 *      Point `I` is the ray intersection with the triangle
 *      Distance `OI` is the intersection distance (rayInterDistance)
 */
void
primitive_intersection(
    sample_context_t * sampleCx,
    __global common_primitive_t const * primitive
)
{
    float32x3_t const normal = (float32x3_t)(primitive->v0.w, primitive->e0.w, primitive->e1.w);
    float32x3_t const vAO = sampleCx->rayMeshOrigin - primitive->v0.xyz;
    float32_t const OH = dot(vAO, normal);
    float32_t const normalDotRay = dot(sampleCx->rayMeshDirection, normal);
    float32_t const rayInterDistance = - OH / normalDotRay;

    if (isless(rayInterDistance, kEPSILONE) || isgreaterequal(rayInterDistance, sampleCx->rayInterDistance))
    {
        // cull this triangle
        return;
    }

    float32x3_t const vAI = vAO + sampleCx->rayMeshDirection * rayInterDistance;
    float32x2_t const uv = (
        primitive->uvMatrix.x * dot(vAI, primitive->e0.xyz) +
        primitive->uvMatrix.y * dot(vAI, primitive->e1.xyz)
    );

    if ((uv.x < 0.0f) || (uv.y < 0.0f) || ((uv.x + uv.y) > 1.0f))
    {
        // the intersection would be outside the triangle
        return;
    }

    // there is an intersection, so we update the context

    sampleCx->rayInterDistance = rayInterDistance;
    sampleCx->rayInterMeshNormal = normal;
    sampleCx->rayInterMeshInstance = sampleCx->boundMeshInstance;
}

/*
 * Detects if there is an intersection between the box and the context's ray.
 *
 *                 \
 *                  \
 *                   \
 *                    \
 *          H---------------G
 *          |           \   |
 *          |            \  |
 *          |             \ |
 *          |              \|
 *          |               I
 *          |               |\
 *          |               | \
 *          E---------------F  \
 *                              \
 *                               \
 *                                \    ->
 *                                 \__ d
 *                                 /\
 *                                   \
 *                                    O
 *  y
 *  ^
 *  |
 *  |
 *  z----> x
 *
 *
 * Glossary:
 *      Point `O` is the origin of the ray (sampleCx->rayMeshOrigin)
 *      Vector `d` is direction of the ray (sampleCx->rayMeshDirection)
 *      Point `I` is the nearest ray intersection with the box
 */
inline
uint32_t
box_intersection(sample_context_t const * const sampleCx, float32x3_t const OE, float32x3_t const OG)
{
    float32x3_t const t0 = OE * sampleCx->rayMeshDirectionInverted;
    float32x3_t const t1 = OG * sampleCx->rayMeshDirectionInverted;
    float32x3_t const tMin = min(t0, t1);
    float32x3_t const tMax = max(t0, t1);

    return (
        (tMin.x < tMax.y) &&
        (tMin.y < tMax.x) &&
        (max(tMin.x, tMin.y) < tMax.z) &&
        (tMin.z < min(tMax.x, tMax.y)) &&
        (max(tMax.x, max(tMax.y, tMax.z)) > 0.0f) &&
        (min(tMin.x, min(tMin.y, tMin.z)) < sampleCx->rayInterDistance)
    );
}


#endif // _CLH_CS499R_PROGRAM_INTERSECTION