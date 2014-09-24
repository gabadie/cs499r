
#include "cs499r.hpp"

void
buildDummyScene(CS499R::Scene & scene)
{
    float const radius = 10.0f;
    float const lightSize = 1.0f;

    float32x3_t v000 { -radius, -radius, 0.0f };
    float32x3_t v100 { +radius, -radius, 0.0f };
    float32x3_t v010 { -radius, +radius, 0.0f };
    float32x3_t v110 { +radius, +radius, 0.0f };
    float32x3_t v001 { -radius, -radius, +radius };
    float32x3_t v101 { +radius, -radius, +radius };
    float32x3_t v011 { -radius, +radius, +radius };
    float32x3_t v111 { +radius, +radius, +radius };

    float32x3_t red { 0.75f, 0.25f, 0.25f };
    float32x3_t blue { 0.75f, 0.25f, 0.25f };
    float32x3_t white { 0.75f, 0.75f, 0.75f };
    float32x3_t black { 0.0f, 0.0f, 0.0f };

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
    float32x3_t vl0 { radius - lightSize, radius, radius };
    float32x3_t vl1 { radius, radius - lightSize, radius };
    float32x3_t vl2 { radius, radius, radius - lightSize };
    float32x3_t light { 10.0f, 10.0f, 10.0f };

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

    size_t const imageWidth = 32;
    size_t const imageHeight = 32;

    CS499R::RayTracer rayTracer(device);
    CS499R::Scene scene;
    CS499R::Camera camera;
    CS499R::Image image(imageWidth, imageHeight, CS499R::RenderTarget::kChanelCount);

    buildDummyScene(scene);

    {
        CS499R::RenderTarget target(&rayTracer, imageWidth, imageHeight);
        CS499R::SceneBuffer sceneBuffer(&scene, &rayTracer);

        sceneBuffer.render(&target, &camera);

        target.download(&image);
    }

    image.saveToFile("render.gitignore.png");

    return 0;
}
