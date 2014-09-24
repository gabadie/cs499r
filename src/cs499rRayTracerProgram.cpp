
#include "cs499rRayTracer.hpp"
#include "cs499rCommonStruct.hpp"

namespace
{

    char const * kKernelDispatchKernel = CS499R_CODE(
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
            kCodeStructTriangle,
            kKernelDispatchKernel
        };

        { // initialize program
            mProgram = clCreateProgramWithSource(mContext, CS499R_ARRAY_SIZE(programCode), programCode, NULL, &error);

            CS499R_ASSERT(error == 0);

            clBuildProgram(mProgram, 0, NULL, NULL, NULL, &error);

            CS499R_ASSERT(error == 0);
        }

        {
            mKernel.dispatch = clCreateKernel(mProgram, "dispatch_main", &error);

            CS499R_ASSERT(error == 0);
        }
    }


}
