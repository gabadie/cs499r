
#include <iostream>

#include "app.hpp"


int
main(int argc, char const * const * argv)
{
    cl_device_id device;

    if (App::chooseDevice(argc == 2 ? argv[1] : nullptr, &device))
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
        renderState.mRayAlgorithm = CS499R::kRayAlgorithmPathTracer;
        //renderState.mRayAlgorithm = CS499R::kRayAlgorithmDebugNormal;
    }

    CS499R::Image image(imageWidth, imageHeight, CS499R::RenderTarget::kChanelCount);
    CS499R::RenderProfiling renderProfiling;

    App::buildSceneMeshes(scene);

    {
        CS499R::RenderTarget renderTarget(&rayTracer, imageWidth, imageHeight);
        CS499R::SceneBuffer sceneBuffer(&scene, &rayTracer);

        renderState.mRenderTarget = &renderTarget;
        renderState.shotScene(&sceneBuffer, &camera, &renderProfiling);

        renderTarget.download(&image);
    }

    image.saveToFile("render.gitignore.png");

    std::cout << "Input:" << std::endl;
    std::cout << "    Render width:             " << imageWidth << " px" << std::endl;
    std::cout << "    Render height:            " << imageHeight << " px" << std::endl;
    std::cout << "    Sub-pixels:               " << pow(renderState.mPixelBorderSubdivisions, 2) << std::endl;
    std::cout << "    Sample per sub-pixels:    " << renderState.mSamplesPerSubdivisions << std::endl;
    std::cout << std::endl;

    std::cout << "Output:" << std::endl;
    std::cout << "    Total Samples:            " << renderProfiling.mSamples << std::endl;
    std::cout << "    CPU duration:             " << renderProfiling.mCPUDuration << " us" << std::endl;
    std::cout << "    CPU duration per samples: " <<
        double(renderProfiling.mCPUDuration) / double(renderProfiling.mSamples) << " us" << std::endl;
    std::cout << std::endl;

    return 0;
}
