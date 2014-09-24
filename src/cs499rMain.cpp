
#include "cs499r.hpp"

void
buildDummyScene(CS499R::Scene & scene)
{
    float const radius = 10.0f;
    float const lightSize = 1.0f;

    float3 v000 { -radius, -radius, 0.0f };
    float3 v100 { +radius, -radius, 0.0f };
    float3 v010 { -radius, +radius, 0.0f };
    float3 v110 { +radius, +radius, 0.0f };
    float3 v001 { -radius, -radius, +radius };
    float3 v101 { +radius, -radius, +radius };
    float3 v011 { -radius, +radius, +radius };
    float3 v111 { +radius, +radius, +radius };

    float3 red { 0.75f, 0.25f, 0.25f };
    float3 blue { 0.75f, 0.25f, 0.25f };
    float3 white { 0.75f, 0.75f, 0.75f };
    float3 black { 0.0f, 0.0f, 0.0f };

    // X-
    scene.addTriangle(v000, v010, v001, red, black);
    scene.addTriangle(v001, v010, v011, red, black);

    // X+
    scene.addTriangle(v100, v101, v110, blue, black);
    scene.addTriangle(v101, v111, v110, blue, black);

    // Y-
    scene.addTriangle(v000, v100, v001, white, black);
    scene.addTriangle(v001, v100, v101, white, black);

    // Y+
    scene.addTriangle(v010, v011, v110, white, black);
    scene.addTriangle(v011, v111, v110, white, black);

    // Z-
    scene.addTriangle(v000, v100, v010, white, black);
    scene.addTriangle(v010, v100, v110, white, black);

    // Z+
    scene.addTriangle(v001, v011, v101, white, black);
    scene.addTriangle(v011, v111, v101, white, black);

    // light
    float3 vl0 { radius - lightSize, radius, radius };
    float3 vl1 { radius, radius - lightSize, radius };
    float3 vl2 { radius, radius, radius - lightSize };
    float3 light { 10.0f, 10.0f, 10.0f };

    scene.addTriangle(vl0, vl1, vl2, black, light);
}

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
    CS499R::Scene scene;

    buildDummyScene(scene);

    CS499R::SceneBuffer sceneBuffer(&scene, &rayTracer);

    return 0;
}
