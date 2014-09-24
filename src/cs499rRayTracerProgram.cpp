
#include <string.h>
#include "cs499rRayTracer.hpp"
#include "cs499rCommonStruct.hpp"


namespace
{

    char const * const kCodeLibConsts = CS499R_CODE(

        __constant float kEPSILONE = 0.00001f;

    );

    char const * const kCodeLibStructs = CS499R_CODE(

        typedef __private struct private_context_s
        {
            // ray's origin in the mesh space
            float3 rayOrigin;

            // normalized ray's direction in the mesh space
            float3 rayDirection;

            // ray's minimal distance found for depth test
            float rayInterDistance;

            // ray's computed colors
            float3 rayInterColorMultiply;
            float3 rayInterColorAdd;

            // the ray's unnormalized intersection normal
            float3 rayInterNormal;

        } private_context_t;

    );

    char const * const kCodeLibEssentials = CS499R_CODE(

        void
        intersect_triangles(private_context_t * cx, __global triangle_t const * triangle)
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

    );

    char const * const kKernelDispatchKernel = CS499R_CODE(
        /*
         * This trace the ray throught the scene
         */
        __kernel void
        dispatch_main(__global float * input, __global float * output)
        {
            int i = get_global_id(0);

            output[i] = input[i] * input[i];
        }
    );

}

namespace CS499R
{

    void RayTracer::buildProgram()
    {
        cl_int error = 0;

        char const * programCode[] = {
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
