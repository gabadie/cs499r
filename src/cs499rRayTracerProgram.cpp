
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
            float32x3_t raySceneOrigin;

            // normalized ray's direction in the mesh space
            float32x3_t raySceneDirection;

            // ray's origin in the mesh space
            float32x3_t rayMeshOrigin;

            // normalized ray's direction in the mesh space
            float32x3_t rayMeshDirection;

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

        } sample_context_t;

    );

    char const * const kCodeLibEssentials = CS499R_CODE(

        inline
        float32_t
        random(sample_context_t * sampleCx)
        {
            uint32_t seed = sampleCx->randomSeed;

            seed++;

            sampleCx->randomSeed = seed;

            float32_t s = sin((float32_t)seed * 12.9898f) * 43758.5453f;

            return s - floor(s);
        }

        void
        primitive_intersection(
            sample_context_t * sampleCx,
            __global common_primitive_t const * primitive
        )
        {
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
             *      Point `A` is the first vertex of the current triangle (primitive->v0)
             *      Point `I` is the ray intersection with the triangle
             *      Distance `OI` is the intersection distance (rayInterDistance)
             *
             *
             *
             *
             */

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

        inline
        void
        mesh_instance_intersection(
            sample_context_t * sampleCx,
            __global common_primitive_t const * primitives
        )
        {
            __global common_mesh_instance_t const * const meshInstance = sampleCx->boundMeshInstance;

            sampleCx->rayMeshOrigin = (
                meshInstance->sceneMeshMatrix.x * sampleCx->raySceneOrigin.x +
                meshInstance->sceneMeshMatrix.y * sampleCx->raySceneOrigin.y +
                meshInstance->sceneMeshMatrix.z * sampleCx->raySceneOrigin.z +
                meshInstance->sceneMeshMatrix.w
            );

            sampleCx->rayMeshDirection = (
                meshInstance->sceneMeshMatrix.x * sampleCx->raySceneDirection.x +
                meshInstance->sceneMeshMatrix.y * sampleCx->raySceneDirection.y +
                meshInstance->sceneMeshMatrix.z * sampleCx->raySceneDirection.z
            );

            __global common_primitive_t const * const meshPrimitives = primitives + meshInstance->mesh.primFirst;
            uint32_t const primCount = meshInstance->mesh.primCount;

            for (uint32_t primId = 0; primId < primCount; primId++)
            {
                primitive_intersection(sampleCx, meshPrimitives + primId);
            }
        }

        inline
        void
        scene_intersection(
            sample_context_t * sampleCx,
            __global common_shot_context_t const * shotCx,
            __global common_mesh_instance_t const * meshInstances,
            __global common_primitive_t const * primitives
        )
        {
            sampleCx->rayInterDistance = INFINITY;

            /*
             * We sets up the intersection mesh as the anonymous mesh first
             */
            sampleCx->rayInterMeshInstance = meshInstances;

            /*
             * i = 1 because we skip over the anonymous mesh instance
             */
            for (uint32_t i = 1; i < shotCx->meshInstanceMaxId; i++)
            {
                sampleCx->boundMeshInstance = meshInstances + i;

                mesh_instance_intersection(sampleCx, primitives);
            }
        }

        inline
        void
        camera_first_ray(
            sample_context_t * const sampleCx,
            __global common_shot_context_t const * shotCx,
            uint32x2_t const pixelCoord,
            uint32x2_t const pixelSubpixelCoord
        )
        {
            float32x2_t const subpixelCoord = (float32x2_t)(pixelCoord.x, pixelCoord.y) +
                    (1.0f + 2.0f * (float32x2_t)(pixelSubpixelCoord.x, pixelSubpixelCoord.y)) *
                    (0.5f / (float32_t) shotCx->render.z);

            float32x2_t areaCoord;
            areaCoord.x = subpixelCoord.x / ((float32_t)shotCx->render.x);
            areaCoord.y = subpixelCoord.y / ((float32_t)shotCx->render.y);
            areaCoord = areaCoord * 2.0f - 1.0f;

            float32x3_t rayFocusPoint = (
                shotCx->camera.focusBasisU * areaCoord.x +
                shotCx->camera.focusBasisV * areaCoord.y +
                shotCx->camera.focusPosition
            );

            sampleCx->raySceneOrigin = (
                shotCx->camera.shotBasisU * areaCoord.x +
                shotCx->camera.shotBasisV * areaCoord.y +
                shotCx->camera.shotPosition
            );

            sampleCx->raySceneDirection = normalize(rayFocusPoint - sampleCx->raySceneOrigin);
        }

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

    );

    /*
     * In some case, it might be more performant to recompute a variable within
     * a functions instead of storing it into the stack.
     */
    char const * const kKernelCommonDefines = "\n"
        "#define pixelBorderSubpixelCount shotCx->render.z\n"
        "#define subpixelSampleCount get_local_size(2)\n"
        "#define subpixelSampleId get_local_id(2)\n"
        "#define pixelSubpixelCount (pixelBorderSubpixelCount * pixelBorderSubpixelCount)\n"
        "#define pixelSubpixelId (pixelSubpixelCoord.x + pixelSubpixelCoord.y * pixelBorderSubpixelCount)\n"
        "#define pixelId (pixelCoord.x + pixelCoord.y * shotCx->render.x)\n"
    ;

    char const * const kKernelPathTracer = CS499R_CODE(
        /*
         * Path tracer's main entry point
         */
        __kernel void
        kernel_path_tracer_main(
            __global common_shot_context_t const * shotCx,
            __global common_mesh_instance_t const * meshInstances,
            __global common_primitive_t const * primitives,
            __global float32_t * renderTarget
        )
        {
            uint32x2_t const pixelCoord = (uint32x2_t)(
                get_global_id(0) / pixelBorderSubpixelCount,
                get_global_id(1) / pixelBorderSubpixelCount
            );

            if (pixelCoord.x >= shotCx->render.x || pixelCoord.y >= shotCx->render.y)
            {
                return;
            }

            uint32x2_t const pixelSubpixelCoord = (uint32x2_t)(get_local_id(0), get_local_id(1));

            sample_context_t sampleCx;

            { // set up random seed
                uint32_t const subpixelId = pixelId * pixelSubpixelCount + pixelSubpixelId;
                uint32_t const sampleId = subpixelId * subpixelSampleCount + subpixelSampleId;

                sampleCx.randomSeed = sampleId * (kRecursionCount * kRandomPerRecursion);
                sampleCx.randomSeed = sampleCx.randomSeed % 1436283;
            }

            camera_first_ray(&sampleCx, shotCx, pixelCoord, pixelSubpixelCoord);
            scene_intersection(&sampleCx, shotCx, meshInstances, primitives);

            float32x3_t sampleColor = sampleCx.rayInterMeshInstance->emitColor;
            float32x3_t sampleColorMultiply = sampleCx.rayInterMeshInstance->diffuseColor;

            for (uint32_t i = 0; i < kRecursionCount; i++)
            {
                path_tracer_rebound(&sampleCx);
                scene_intersection(&sampleCx, shotCx, meshInstances, primitives);

                sampleColor += sampleCx.rayInterMeshInstance->emitColor * sampleColorMultiply;
                sampleColorMultiply *= sampleCx.rayInterMeshInstance->diffuseColor;
            }

            { // logarithmic sum of pixelSampleColors[]
                /*
                 * This algorithm is dedicated to compute the average color
                 * value over pixelSampleColors[]. Here is an example of the
                 * optimization for a warp size of 2 threads and group size of
                 * a total of 8 threads.
                 *
                 *  warp id | local id | offset 4 | offset 2 | offset 1
                 *          |          |
                 *     0    |    0     | -+------- ---+------ ----+-----
                 *     0    |    1     | -|-+----- ---|-+---- ----/
                 *     1    |    2     | -|-|-+--- ---/ |  * warps 1 looping
                 *     1    |    3     | -|-|-|-+- -----/  * on the barrier()
                 *     2    |    4     | -/ | | |  *
                 *     2    |    5     | ---/ | |  * warps 2 & 3 looping
                 *     3    |    6     | -----/ |  * on the barrier()
                 *     3    |    7     | -------/  *
                 */
                uint32_t const pixelSampleId = pixelSubpixelId + subpixelSampleId * pixelSubpixelCount;
                uint32_t const pixelSampleCount = pixelSubpixelCount * subpixelSampleCount;

                float32x3_t pixelColor = sampleColor;

                __local float32x3_t pixelSampleColors[1024];
                __local float32x3_t * pixelSampleColor = pixelSampleColors + pixelSampleId;

                pixelSampleColor[0] = pixelColor;

                for (uint32_t sampleOffset = (pixelSampleCount >> 1); sampleOffset != 0; sampleOffset >>= 1)
                {
                    barrier(CLK_LOCAL_MEM_FENCE);

                    if (pixelSampleId < sampleOffset)
                    {
                        /*
                         * This condition here is very important, when the group
                         * is instantiated with severals warps... Indeed, this
                         * condition will then avoid most of the warps to do
                         * useless operations while other still have threads
                         * doing the additions to finish up the sum. Therefor
                         * this idle time because of the barrier() can releases
                         * GPU's ALUs usages.
                         */
                        pixelColor += pixelSampleColor[sampleOffset];

                        pixelSampleColor[0] = pixelColor;
                    }
                }

                if (pixelSampleId == 0)
                {
                    pixelColor *= (1.0f / (float32_t) pixelSampleCount);

                    __global float32_t * pixelTarget = renderTarget + pixelId * 3;

                    pixelTarget[0] = pixelColor.x;
                    pixelTarget[1] = pixelColor.y;
                    pixelTarget[2] = pixelColor.z;
                }
            }
        }
    );

    char const * const kKernelDebug = CS499R_CODE(
        __kernel
        void
        kernel_debug_normal(
            __global common_shot_context_t const * shotCx,
            __global common_mesh_instance_t const * meshInstances,
            __global common_primitive_t const * primitives,
            __global float32_t * renderTarget
        )
        {
            uint32x2_t const pixelCoord = (uint32x2_t)(
                get_global_id(0) / pixelBorderSubpixelCount,
                get_global_id(1) / pixelBorderSubpixelCount
            );

            if (pixelCoord.x >= shotCx->render.x || pixelCoord.y >= shotCx->render.y)
            {
                return;
            }

            uint32x2_t const pixelSubpixelCoord = (uint32x2_t)(get_local_id(0), get_local_id(1));

            sample_context_t sampleCx;

            camera_first_ray(&sampleCx, shotCx, pixelCoord, pixelSubpixelCoord);
            scene_intersection(&sampleCx, shotCx, meshInstances, primitives);

            __global common_mesh_instance_t const * const meshInstance = sampleCx.rayInterMeshInstance;

            float32x3_t const meshNormal = sampleCx.rayInterMeshNormal;
            float32x3_t const sceneNormal = (
                meshInstance->meshSceneMatrix.x * meshNormal.x +
                meshInstance->meshSceneMatrix.y * meshNormal.y +
                meshInstance->meshSceneMatrix.z * meshNormal.z
            );

            float32x3_t pixelColor = sceneNormal * 0.5f + 0.5f;

            __global float32_t * pixelTarget = renderTarget + pixelId * 3;

            pixelTarget[0] = pixelColor.x;
            pixelTarget[1] = pixelColor.y;
            pixelTarget[2] = pixelColor.z;
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
            kKernelCommonDefines,
            kKernelPathTracer,
            kKernelDebug,
        };

        char const * programKernelNameArray[] = {
            "kernel_path_tracer_main",
            "kernel_debug_normal",
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

        for (size_t i = 0; i < kRayAlgorithmCount; i++)
        {
            mKernelArray[i] = clCreateKernel(mProgram, programKernelNameArray[i], &error);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }
    }


}
