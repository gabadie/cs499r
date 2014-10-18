
#include <iostream>

#include "app.hpp"


namespace App
{

    int
    chooseDevice(char const * selectedDeviceName, cl_device_id * deviceId)
    {
        cl_int error = 0;
        cl_bool deviceFound = CL_FALSE;
        cl_uint deviceCount = 0;
        cl_platform_id platform;
        cl_device_id deviceIds[16];

        char tmpBuffer[512];
        uint64_t tmpU64;

        error = clGetPlatformIDs(1, &platform, 0);
        CS499R_ASSERT_NO_CL_ERROR(error);

        error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, CS499R_ARRAY_SIZE(deviceIds), deviceIds, &deviceCount);
        CS499R_ASSERT_NO_CL_ERROR(error);
        CS499R_ASSERT(deviceCount > 0);

        std::cout << "Available devices:" << std::endl;

        for (cl_uint i = 0; i < deviceCount; i++)
        {
            error = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, sizeof(tmpBuffer), tmpBuffer, 0);

            CS499R_ASSERT_NO_CL_ERROR(error);

            if (selectedDeviceName && strcmp(tmpBuffer, selectedDeviceName) == 0)
            {
                *deviceId = deviceIds[i];
                deviceFound = CL_TRUE;

                std::cout << "->  ";
            }
            else
            {
                std::cout << "    ";
            }

            std::cout << tmpBuffer << std::endl;
        }

        if (selectedDeviceName == nullptr)
        {
            std::cout << "Please choose a device" << std::endl;
            return 1;
        }

        if (!deviceFound)
        {
            std::cout << "Unknown device `" << selectedDeviceName << "`" << std::endl;
            return 1;
        }



        std::cout << std::endl;
        std::cout << "Platform:" << std::endl;

        clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(tmpBuffer), tmpBuffer, NULL);
        std::cout << "    CL_PLATFORM_NAME: " << tmpBuffer << std::endl;

        clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(tmpBuffer), tmpBuffer, NULL);
        std::cout << "    CL_PLATFORM_VENDOR: " << tmpBuffer << std::endl;

        clGetPlatformInfo(platform, CL_PLATFORM_PROFILE, sizeof(tmpBuffer), tmpBuffer, NULL);
        std::cout << "    CL_PLATFORM_PROFILE: " << tmpBuffer << std::endl;

        clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(tmpBuffer), tmpBuffer, NULL);
        std::cout << "    CL_PLATFORM_VERSION: " << tmpBuffer << std::endl;

        std::cout << std::endl;

        clGetDeviceInfo(*deviceId, CL_DEVICE_NAME, sizeof(tmpBuffer), tmpBuffer, NULL);
        std::cout << tmpBuffer << ":" << std::endl;

        clGetDeviceInfo(*deviceId, CL_DRIVER_VERSION, sizeof(tmpBuffer), tmpBuffer, NULL);
        std::cout << "    CL_DRIVER_VERSION: " << tmpBuffer << "" << std::endl;

        clGetDeviceInfo(*deviceId, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(tmpU64), &tmpU64, NULL);
        std::cout << "    CL_DEVICE_GLOBAL_MEM_SIZE: " << tmpU64 << "" << std::endl;

        clGetDeviceInfo(*deviceId, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(tmpU64), &tmpU64, NULL);
        std::cout << "    CL_DEVICE_GLOBAL_MEM_CACHE_SIZE: " << tmpU64 << "" << std::endl;

        clGetDeviceInfo(*deviceId, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(tmpU64), &tmpU64, NULL);
        std::cout << "    CL_DEVICE_LOCAL_MEM_SIZE: " << tmpU64 << "" << std::endl;

        clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(tmpU64), &tmpU64, NULL);
        std::cout << "    CL_DEVICE_MAX_CLOCK_FREQUENCY: " << tmpU64 << "" << std::endl;

        clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(tmpU64), &tmpU64, NULL);
        std::cout << "    CL_DEVICE_MAX_WORK_GROUP_SIZE: " << tmpU64 << "" << std::endl;

        clGetDeviceInfo(*deviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(tmpU64), &tmpU64, NULL);
        std::cout << "    CL_DEVICE_MAX_COMPUTE_UNITS: " << tmpU64 << "" << std::endl;

        std::cout << std::endl;

        return 0;
    }


}
