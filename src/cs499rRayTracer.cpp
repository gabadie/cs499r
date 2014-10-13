
#include <string.h>

#include "cs499rCommonStruct.hpp"
#include "cs499rRayTracer.hpp"
#include "cs499rProgram.hpp"


namespace CS499R
{

    RayTracer::RayTracer(cl_device_id const & device)
    {
        cl_int error = 0;

        mDeviceId = device;
        mContext = clCreateContext(0, 1, &device, 0, 0, &error);

        CS499R_ASSERT_NO_CL_ERROR(error);

        mCmdQueue = clCreateCommandQueue(mContext, device, 0, &error);

        CS499R_ASSERT_NO_CL_ERROR(error);

        buildPrograms();
    }

    RayTracer::~RayTracer()
    {
        cl_int error = 0;

        for (size_t i = 0; i < kRayAlgorithmCount; i++)
        {
            error = clReleaseKernel(mProgram[i].kernel);
            CS499R_ASSERT_NO_CL_ERROR(error);

            error = clReleaseProgram(mProgram[i].program);
            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        error |= clReleaseCommandQueue(mCmdQueue);
        error |= clReleaseContext(mContext);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

    void
    RayTracer::buildPrograms()
    {
        cl_int error = 0;

        char const * programCodes[kRayAlgorithmCount] = {
            kCS499RProgramPathTracer,
            kCS499RProgramDebugNormal
        };

        char const * const programKernelNameArray[kRayAlgorithmCount] = {
            "kernel_path_tracer_main",
            "kernel_debug_normal",
        };

        for (size_t programId = 0; programId < kRayAlgorithmCount; programId++)
        { // initialize program
            mProgram[programId].program = clCreateProgramWithSource(
                mContext,
                1, programCodes + programId,
                NULL, &error
            );

            CS499R_ASSERT_NO_CL_ERROR(error);

            error |= clBuildProgram(mProgram[programId].program, 0, NULL, NULL, NULL, NULL);

            if (error)
            {
                size_t bufferSize;
                clGetProgramBuildInfo(
                    mProgram[programId].program, mDeviceId,
                    CL_PROGRAM_BUILD_LOG,
                    0, nullptr, &bufferSize
                );

                char * buffer = new char [bufferSize + 1];
                memset(buffer, 0, bufferSize + 1);

                clGetProgramBuildInfo(
                    mProgram[programId].program, mDeviceId,
                    CL_PROGRAM_BUILD_LOG,
                    bufferSize + 1, buffer, &bufferSize
                );

                fprintf(stderr, "OPENCL COMPILATION (%s) FAILED\n", programKernelNameArray[programId]);
                fprintf(stderr, "%s\n", buffer);

                delete [] buffer;

                CS499R_ASSERT_NO_CL_ERROR(error);
                CS499R_CRASH();
            }

            mProgram[programId].kernel = clCreateKernel(
                mProgram[programId].program,
                programKernelNameArray[programId],
                &error
            );

            CS499R_ASSERT_NO_CL_ERROR(error);
        }
    }

}
