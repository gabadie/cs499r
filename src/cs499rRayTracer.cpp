
#include "cs499rRayTracer.hpp"


namespace CS499R
{

    RayTracer::RayTracer(cl_device_id const & device)
    {
        cl_int error = 0;

        mContext = clCreateContext(0, 1, &device, 0, 0, &error);

        CS499R_ASSERT(error == 0);

        mCmdQueue = clCreateCommandQueue(mContext, device, 0, &error);

        CS499R_ASSERT(error == 0);
    }

    RayTracer::~RayTracer()
    {
        cl_int error = 0;

        error |= clReleaseCommandQueue(mCmdQueue);
        error |= clReleaseContext(mContext);

        CS499R_ASSERT(error == 0);
    }

}
