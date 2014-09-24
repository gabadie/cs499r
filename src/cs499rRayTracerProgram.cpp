
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

        __constant float kEPSILONE = 0.00001f;

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

        } sample_context_t;

    );

    char const * const kCodeLibEssentials = CS499R_CODE(

        void
        intersect_triangles(sample_context_t * cx, __global common_triangle_t const * triangle)
        {
            float3 AB = triangle->vertex[1] - triangle->vertex[0];
            float3 AC = triangle->vertex[2] - triangle->vertex[0];
            float3 normal = cross(AB, AC);

            float originFromPlan = dot(cx->rayOrigin - triangle->vertex[0], normal);
            float normalDotRay = dot(cx->rayDirection, normal);
            float rayDistance = - originFromPlan / normalDotRay;

            if (isless(rayDistance, kEPSILONE) || isgreaterequal(rayDistance, cx->rayInterDistance))
            {
                // cull this triangle
                return;
            }

            float3 intersection = cx->rayOrigin + cx->rayDirection * rayDistance;
            float basis_dot = dot(AB, AC);
            float inv_squared_length_AB = 1.0 / dot(AB, AB);
            float inv_squared_length_AC = 1.0 / dot(AC, AC);
            float uh2 = basis_dot * inv_squared_length_AB;
            float vh1 = basis_dot * inv_squared_length_AC;
            float inv_det = 1.0 / (1.0 - uh2 * vh1);
            float h1 = dot(intersection, AB) * inv_squared_length_AB;
            float h2 = dot(intersection, AC) * inv_squared_length_AC;
            float u = (h1 - h2 * uh2) * inv_det;

            if(u < 0.0)
            {
                return;
            }

            float v = (h2 - vh1 * h1) * inv_det;

            if((v < 0.0) || ((u + v) > 1.0))
            {
                return;
            }

            // there is an intersection, so we update the context

            cx->rayInterDistance = rayDistance;
            cx->rayInterColorMultiply = triangle->diffuseColor;
            cx->rayInterColorAdd = triangle->emitColor;
            cx->rayInterNormal = normal;
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
            __global float * renderTarget
        )
        {
            uint2 texelCoord = (uint2)(get_global_id(0), get_global_id(1));

            uint texelId = texelCoord.x + texelCoord.y * shotCx->renderWidth;

            if (texelCoord.x >= shotCx->renderWidth || texelCoord.y >= shotCx->renderHeight)
            {
                return;
            }

            float2 shotAreaCoord;
            shotAreaCoord.x = ((float)texelCoord.x) / (((float)shotCx->renderWidth) - 1.0f);
            shotAreaCoord.y = ((float)texelCoord.y) / (((float)shotCx->renderHeight) - 1.0f);

            sample_context_t sampleCx;

            intersect_scene(&sampleCx, shotCx, triangles);

            renderTarget[texelId * 3 + 0] = 1.0;
            renderTarget[texelId * 3 + 1] = 0.0;
            renderTarget[texelId * 3 + 2] = 0.0;
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
