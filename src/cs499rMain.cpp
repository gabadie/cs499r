
#include "cs499r.hpp"

int
main()
{
    cl_platform_id platform;
    cl_device_id device;
    cl_int error = 0;

    {
        error |= clGetPlatformIDs(1, &platform, 0);
        error |= clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, 0);

        CS499R_ASSERT(error == 0);
    }

    CS499R::RayTracer rayTracer(device);

    return 0;
}
