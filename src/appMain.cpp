
#include <iostream>
#include "cs499r.hpp"


static
void
buildSceneMeshes(CS499R::Scene & scene)
{
    float32_t const radius = 20.0f;
    float32_t const height = 10.0f;

    {
        float32x3_t meshVertices[] = {
            float32x3_t(0.0f, -radius, 0.0f),
            float32x3_t(0.0f, +radius, 0.0f),
            float32x3_t(0.0f, -radius, +height),
            float32x3_t(0.0f, -radius, +height),
            float32x3_t(0.0f, +radius, 0.0f),
            float32x3_t(0.0f, +radius, +height),
        };

        CS499R::Mesh mesh(CS499R_ARRAY_SIZE(meshVertices), meshVertices);

        auto sceneMesh = scene.addMesh("room/wall", mesh);

        {
            auto instance = scene.addMeshInstance("room/wall/x+", sceneMesh);
            instance->mScenePosition = float32x3_t(10.0f, 0.0f, 0.0f);
        }
    }

    {
        float32x3_t meshVertices[] = {
            float32x3_t(-radius, -radius, 0.0f),
            float32x3_t(+radius, -radius, 0.0f),
            float32x3_t(-radius, +radius, 0.0f),
            float32x3_t(-radius, +radius, 0.0f),
            float32x3_t(+radius, -radius, 0.0f),
            float32x3_t(+radius, +radius, 0.0f),
        };

        CS499R::Mesh mesh(CS499R_ARRAY_SIZE(meshVertices), meshVertices);

        scene.addMesh("roomFloor", mesh);
    }
}

static
void
buildDummyScene(CS499R::Scene & scene)
{
    buildSceneMeshes(scene);

    float const radius = 20.0f;
    float const height = 10.0f;
    float const lightSize = 9.0f;

    float32x3_t v000 { -radius, -radius, 0.0f };
    float32x3_t v100 { +radius, -radius, 0.0f };
    float32x3_t v010 { -radius, +radius, 0.0f };
    float32x3_t v110 { +radius, +radius, 0.0f };
    float32x3_t v001 { -radius, -radius, height };
    float32x3_t v101 { +radius, -radius, height };
    float32x3_t v011 { -radius, +radius, height };
    float32x3_t v111 { +radius, +radius, height };

    float32x3_t red { 0.75f, 0.25f, 0.25f };
    float32x3_t blue { 0.25f, 0.25f, 0.75f };
    float32x3_t white { 0.75f, 0.75f, 0.75f };
    float32x3_t black { 0.0f, 0.0f, 0.0f };

    // X-
    scene.addTriangle(v000, v010, v001, red, black);
    scene.addTriangle(v001, v010, v011, red, black);

    // X+
    scene.addTriangle(v100, v101, v110, blue, black);
    scene.addTriangle(v101, v111, v110, blue, black);

    // Y-
    scene.addTriangle(v000, v001, v100, white, black);
    scene.addTriangle(v001, v101, v100, white, black);

    // Y+
    scene.addTriangle(v010, v110, v011, white, black);
    scene.addTriangle(v011, v110, v111, white, black);

    // Z-
    scene.addTriangle(v000, v100, v010, white, black);
    scene.addTriangle(v010, v100, v110, white, black);

    // Z+
    scene.addTriangle(v001, v011, v101, white, black);
    scene.addTriangle(v011, v111, v101, white, black);

    // light
    float32x3_t vl0 { radius - lightSize, radius, height };
    float32x3_t vl1 { radius, radius, height - lightSize };
    float32x3_t vl2 { radius, radius - lightSize, height };
    float32x3_t light(5.0f);

    scene.addTriangle(vl0, vl1, vl2, black, light);
}

static
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

int
main(int argc, char const * const * argv)
{
    cl_device_id device;

    if (chooseDevice(argc == 2 ? argv[1] : nullptr, &device))
    {
        return 1;
    }

    size_t const imageWidth = 512;
    size_t const imageHeight = 512;

    CS499R::RayTracer rayTracer(device);
    CS499R::RenderState renderState;
    CS499R::Scene scene;
    CS499R::Camera camera;

    { // sets up the camera
        camera.mShotPosition = float32x3_t(-10.0f, -15.0f, 9.0f);
        camera.mShotPosition = float32x3_t(-10.0f, -10.0f, 2.0f);
        camera.mFocusPosition = float32x3_t(0.0f, 0.0f, 2.0f);
        camera.mShotDiagonalLength = 0.02f;
    }

    { // sets up the render state
        renderState.mPixelBorderSubdivisions = 4;
        renderState.mSamplesPerSubdivisions = 32;
    }

    CS499R::Image image(imageWidth, imageHeight, CS499R::RenderTarget::kChanelCount);
    CS499R::RenderProfiling renderProfiling;

    buildDummyScene(scene);

    {
        CS499R::RenderTarget renderTarget(&rayTracer, imageWidth, imageHeight);
        CS499R::SceneBuffer sceneBuffer(&scene, &rayTracer);

        renderState.mRenderTarget = &renderTarget;
        renderState.shotScene(&sceneBuffer, &camera, &renderProfiling);

        renderTarget.download(&image);
    }

    image.saveToFile("render.gitignore.png");

    std::cout << "Input:" << std::endl;
    std::cout << "    Render width:          " << imageWidth << " px" << std::endl;
    std::cout << "    Render height:         " << imageHeight << " px" << std::endl;
    std::cout << "    Sub-pixels:            " << pow(renderState.mPixelBorderSubdivisions, 2) << std::endl;
    std::cout << "    Sample per sub-pixels: " << renderState.mSamplesPerSubdivisions << std::endl;
    std::cout << std::endl;

    std::cout << "Output:" << std::endl;
    std::cout << "    Rays shot:             " << renderProfiling.mRays << std::endl;
    std::cout << "    CPU duration:          " << renderProfiling.mCPUDuration << " us" << std::endl;
    std::cout << "    CPU duration per rays: " <<
        double(renderProfiling.mCPUDuration) / double(renderProfiling.mRays) << " us" << std::endl;
    std::cout << std::endl;

    return 0;
}
