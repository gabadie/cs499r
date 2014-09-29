
#include "cs499rRayTracer.hpp"
#include "cs499rCommonStruct.hpp"


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

        buildProgram();
    }

    RayTracer::~RayTracer()
    {
        cl_int error = 0;

        for (size_t i = 0; i < kRayAlgorithmCount; i++)
        {
            error = clReleaseKernel(mKernelArray[i]);

            CS499R_ASSERT_NO_CL_ERROR(error);
        }

        error |= clReleaseProgram(mProgram);
        error |= clReleaseCommandQueue(mCmdQueue);
        error |= clReleaseContext(mContext);

        CS499R_ASSERT_NO_CL_ERROR(error);
    }

}
