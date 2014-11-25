
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
    size_t const imageHeight = (imageWidth * 9) / 16;

    CS499R::RayTracer rayTracer(device);
    CS499R::RenderState renderState;
    CS499R::Scene scene;
    CS499R::Camera camera;

    { // sets up the camera
        camera.mShotPosition = float32x3_t(-18.0f, -8.0f, 8.0f);
        camera.mFocusPosition = float32x3_t(0.0f, 0.0f, 2.0f);
        camera.mShotDiagonalLength = 0.02f;
    }

    { // sets up the render state
        renderState.mPixelBorderSubdivisions = 4;
        renderState.mSamplesPerSubdivisions = 32;
        renderState.mRayAlgorithm = CS499R::kRayAlgorithmPathTracer;
        //renderState.mRayAlgorithm = CS499R::kRayAlgorithmDebugNormal;
        //renderState.mRayAlgorithm = CS499R::kRayAlgorithmRayStats;
    }

    CS499R::Image image(imageWidth, imageHeight, CS499R::RenderTarget::kChanelCount);
    CS499R::RenderTerminalTracker terminalTracker;

    App::buildSceneMeshes(scene);

    {
        CS499R::RenderTarget renderTarget(&rayTracer, imageWidth, imageHeight);
        CS499R::CompiledScene sceneBuffer(&scene, &rayTracer);

        renderState.mRenderTarget = &renderTarget;
        renderState.shotScene(&sceneBuffer, &camera, &terminalTracker);

        renderTarget.download(&image);
    }

    image.saveToFile("render.gitignore.png");

    return 0;
}
