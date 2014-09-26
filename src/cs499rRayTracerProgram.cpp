
#include <string.h>
#include "cs499rRayTracer.hpp"
#include "cs499rCommonStruct.hpp"


namespace
{

    char const * const kCodeLibTypedef = CS499R_CODE(

        typedef int int32_t;
        typedef int2 int32x2_t;
        typedef int3 int32x3_t;
        typedef int4 int32x4_t;

        typedef uint uint32_t;
        typedef uint2 uint32x2_t;
        typedef uint3 uint32x3_t;
        typedef uint4 uint32x4_t;

        typedef float float32_t;
        typedef float2 float32x2_t;
        typedef float3 float32x3_t;
        typedef float4 float32x4_t;

    );

    char const * const kCodeLibConsts = CS499R_CODE(

        __constant float32_t kEPSILONE = 0.00001f;
        __constant float32_t kPi = 3.14159265359f;

        __constant float32_t kRecursionCount = 4;
        __constant float32_t kRandomPerRecursion = 2;

    );

    char const * const kCodeLibStructs = CS499R_CODE(

        typedef __private struct sample_context_s
        {
            // ray's origin in the mesh space
            float32x3_t rayOrigin;

            // normalized ray's direction in the mesh space
            float32x3_t rayDirection;

            // ray's minimal distance found for depth test
            float32_t rayInterDistance;

            // ray's computed colors
            float32x3_t rayInterColorMultiply;
            float32x3_t rayInterColorAdd;

            // the ray's unnormalized intersection normal
            float32x3_t rayInterNormal;

            // the sample's random seed
            uint32_t randomSeed;

        } sample_context_t;

        float32_t
        random(sample_context_t * sampleCx)
        {
            uint32_t seed = sampleCx->randomSeed;

            seed++;

            sampleCx->randomSeed = seed;

            float32_t s = sin((float32_t)seed * 12.9898f) * 43758.5453f;

            return s - floor(s);
        }

    );

    char const * const kCodeLibEssentials = CS499R_CODE(

        void
        generate_basis(float32x3_t n, float32x3_t * u, float32x3_t * v)
        {
            if (fabs(n.x) > fabs(n.y))
            {
                float32_t invLen = 1.0 / (n.x * n.x + n.z * n.z);
                *u = invLen * (float32x3_t)(-n.z, 0.0f, n.x);
            }
            else
            {
                float32_t invLen = 1.0 / (n.y * n.y + n.z * n.z);
                *u = invLen * (float32x3_t)(0.0f, -n.z, n.y);
            }

            *v = cross(n, *u);
        }

        void
        intersect_triangles(sample_context_t * sampleCx, __global common_triangle_t const * triangle)
        {
            float32x3_t AB = triangle->v1 - triangle->v0;
            float32x3_t AC = triangle->v2 - triangle->v0;
            float32x3_t normal = cross(AB, AC);

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
             *      Point `O` is the origin of the ray (sampleCx->rayOrigin)
             *      Vector `d` is direction of the ray (sampleCx->rayDirection)
             *      Point `A` is the first vertex of the current triangle (triangle->v0)
             *      Point `I` is the ray intersection with the triangle
             *      Distance `OI` is the intersection distance (rayInterDistance)
             *
             *
             *
             *
             */

            float32_t OH = dot(sampleCx->rayOrigin - triangle->v0, normal);
            float32_t normalDotRay = dot(sampleCx->rayDirection, normal);
            float32_t rayInterDistance = - OH / normalDotRay;

            if (isless(rayInterDistance, kEPSILONE) || isgreaterequal(rayInterDistance, sampleCx->rayInterDistance))
            {
                // cull this triangle
                return;
            }

            float32x3_t rayIntersectionVector = sampleCx->rayOrigin + sampleCx->rayDirection * rayInterDistance - triangle->v0;
            float32_t basis_dot = dot(AB, AC);
            float32x2_t invSquareLenght = 1.0f / (float32x2_t)(dot(AB, AB), dot(AC, AC));
            float32x2_t invSquareLenghtBasisDot = basis_dot * invSquareLenght;
            float32x2_t h = (float32x2_t)(dot(rayIntersectionVector, AB), dot(rayIntersectionVector, AC)) * invSquareLenght;
            float32_t invDet = 1.0f / (1.0f - invSquareLenghtBasisDot.x * invSquareLenghtBasisDot.y);
            float32x2_t coord = invDet * (h - h.yx * invSquareLenghtBasisDot);

            if ((coord.x < 0.0f) || (coord.y < 0.0f) || ((coord.x + coord.y) > 1.0f))
            {
                // the intersection would be outside the triangle
                return;
            }

            // there is an intersection, so we update the context

            sampleCx->rayInterDistance = rayInterDistance;
            sampleCx->rayInterColorMultiply = triangle->diffuseColor;
            sampleCx->rayInterColorAdd = triangle->emitColor;
            sampleCx->rayInterNormal = normalize(normal);
        }

        void
        intersect_scene(
            sample_context_t * sampleCx,
            __global common_shot_context_t const * shotCx,
            __global common_triangle_t const * triangles
        )
        {
            sampleCx->rayInterColorMultiply = (float3)(0.0f);
            sampleCx->rayInterColorAdd = (float3)(0.0f);
            sampleCx->rayInterDistance = INFINITY;

            for (uint i = 0; i < shotCx->triangleCount; i++)
            {
                intersect_triangles(sampleCx, triangles + i);
            }
        }

    );

    char const * const kKernelDispatchKernel = CS499R_CODE(
        /*
         * This trace the ray throught the scene
         */
        __kernel void
        dispatch_main(
            __global common_shot_context_t const * shotCx,
            __global common_triangle_t const * triangles,
            __global float32_t * renderTarget
        )
        {
            uint32x2_t const pixelCoord = ((uint32x2_t)(get_global_id(0) / shotCx->render.z, get_global_id(1) / shotCx->render.z));

            if (pixelCoord.x >= shotCx->render.x || pixelCoord.y >= shotCx->render.y)
            {
                return;
            }

            uint32x2_t const subpixelCoord = (get_local_id(0), get_local_id(1));

            uint32_t const pixelId = pixelCoord.x + pixelCoord.y * shotCx->render.x;
            uint32_t const subpixelId = subpixelCoord.x + subpixelCoord.y * shotCx->render.z;

            float32x2_t const sampleCoord = (float32x2_t)(pixelCoord.x, pixelCoord.y) +
                    (1.0f + 2.0f * (float32x2_t)(subpixelCoord.x, subpixelCoord.y)) *
                    (0.5f / (float32_t) shotCx->render.z);

            float32x2_t areaCoord;
            areaCoord.x = sampleCoord.x / ((float32_t)shotCx->render.x);
            areaCoord.y = sampleCoord.y / ((float32_t)shotCx->render.y);
            areaCoord = areaCoord * 2.0f - 1.0f;

            sample_context_t sampleCx;

            { // set up random seed
                uint32_t const subpixelCountPerPixel = shotCx->render.z * shotCx->render.z;
                uint32_t const sampleCountPerSubPixel = get_local_size(2);

                sampleCx.randomSeed = kRecursionCount * kRandomPerRecursion * (
                    (pixelId * subpixelCountPerPixel + subpixelId) *
                    sampleCountPerSubPixel + get_local_id(2)
                );

                sampleCx.randomSeed = sampleCx.randomSeed % 1436283;
            }

            { // set up first ray
                float32x3_t rayFocusPoint = shotCx->camera.focusPosition +
                    shotCx->camera.focusBasisU * areaCoord.x +
                    shotCx->camera.focusBasisV * areaCoord.y;

                sampleCx.rayOrigin = shotCx->camera.shotPosition +
                    shotCx->camera.shotBasisU * areaCoord.x +
                    shotCx->camera.shotBasisV * areaCoord.y;

                sampleCx.rayDirection = normalize(rayFocusPoint - sampleCx.rayOrigin);
            }

            intersect_scene(&sampleCx, shotCx, triangles);

            float32x3_t sampleColor = sampleCx.rayInterColorAdd;
            float32x3_t sampleColorMultiply = sampleCx.rayInterColorMultiply;

            for (uint32_t i = 0; i < kRecursionCount; i++)
            {
                { // generate a new difuse ray
                    float32x3_t u;
                    float32x3_t v;

                    generate_basis(sampleCx.rayInterNormal, &u, &v);

                    float32_t const r1 = 2.0f * kPi * random(&sampleCx);
                    float32_t const r2 = random(&sampleCx);
                    float32_t const r2s = sqrt(r2);

                    sampleCx.rayOrigin += sampleCx.rayDirection * sampleCx.rayInterDistance;
                    sampleCx.rayDirection = normalize(
                        u * (cos(r1) * r2s) +
                        v * (sin(r1) * r2s) +
                        sampleCx.rayInterNormal * sqrt(1.0f - r2)
                    );
                }

                intersect_scene(&sampleCx, shotCx, triangles);

                sampleColor += sampleCx.rayInterColorAdd * sampleColorMultiply;
                sampleColorMultiply *= sampleCx.rayInterColorMultiply;
            }

            __local float32x3_t sampleColors[1024];
            uint32_t const sampleId = subpixelId * get_local_size(2) + get_local_id(2);

            sampleColors[sampleId] = sampleColor;

            if ((get_local_id(0) + get_local_id(1) + get_local_id(2)) == 0)
            {
                float32x3_t pixelColor = 0.0f;

                uint32_t const sampleCount = shotCx->render.z * shotCx->render.z * get_local_size(2);

                for (uint32_t i = 0; i < sampleCount; i++)
                {
                    pixelColor += sampleColors[i];
                }

                pixelColor *= (1.0f / (float32_t) sampleCount);

                renderTarget[pixelId * 3 + 0] = pixelColor.x;
                renderTarget[pixelId * 3 + 1] = pixelColor.y;
                renderTarget[pixelId * 3 + 2] = pixelColor.z;
            }
        }
    );

}

namespace CS499R
{

    void RayTracer::buildProgram()
    {
        cl_int error = 0;

        char const * programCode[] = {
            kCodeLibTypedef,
            kCodeLibConsts,
            kCodeStructs,
            kCodeLibStructs,
            kCodeLibEssentials,
            kKernelDispatchKernel
        };

        { // initialize program
            mProgram = clCreateProgramWithSource(
                mContext,
                CS499R_ARRAY_SIZE(programCode), programCode,
                NULL, &error
            );

            CS499R_ASSERT(error == 0);

            error |= clBuildProgram(mProgram, 0, NULL, NULL, NULL, NULL);

            if (error)
            {
                size_t bufferSize;
                clGetProgramBuildInfo(mProgram, mDeviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &bufferSize);

                char * buffer = new char [bufferSize + 1];
                memset(buffer, 0, bufferSize + 1);

                clGetProgramBuildInfo(mProgram, mDeviceId, CL_PROGRAM_BUILD_LOG, bufferSize + 1, buffer, &bufferSize);

                fprintf(stderr, "%s\n", buffer);

                delete [] buffer;

                CS499R_CRASH();
            }
        }

        {
            mKernel.dispatch = clCreateKernel(mProgram, "dispatch_main", &error);

            CS499R_ASSERT(error == 0);
        }
    }


}
