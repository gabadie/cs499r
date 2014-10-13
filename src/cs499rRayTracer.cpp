
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

}
