
#include <iomanip>
#include <sstream>

#include "app.hpp"


namespace
{
    float32_t const kRoomWidth = 20.0f;
    float32_t const kRoomHeight = 10.0f;

    float32x3_t const kColorRed(0.75f, 0.25f, 0.25f);
    float32x3_t const kColorBlue(0.25f, 0.25f, 0.75f);
    float32x3_t const kColorWhite(0.75f, 0.75f, 0.75f);

    void
    buildSceneRoom(CS499R::Scene & scene)
    {
        float32_t const radius = kRoomWidth;
        float32_t const height = kRoomHeight;

        { // Room's walls
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
                auto sceneMeshInstance = scene.addMeshInstance("room/wall/x-", sceneMesh);

                sceneMeshInstance->mColorDiffuse = kColorRed;
                sceneMeshInstance->mScenePosition = float32x3_t(-radius, 0.0f, 0.0f);
                sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                    float32x3_t(+1.0f, 0.0f, 0.0f),
                    float32x3_t(0.0f, +1.0f, 0.0f),
                    float32x3_t(0.0f, 0.0f, +1.0f)
                );
            }

            {
                auto sceneMeshInstance = scene.addMeshInstance("room/wall/y-", sceneMesh);

                sceneMeshInstance->mColorDiffuse = kColorWhite;
                sceneMeshInstance->mScenePosition = float32x3_t(0.0f, -radius, 0.0f);
                sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                    float32x3_t(0.0f, +1.0f, 0.0f),
                    float32x3_t(-1.0f, 0.0f, 0.0f),
                    float32x3_t(0.0f, 0.0f, +1.0f)
                );
            }

            {
                auto sceneMeshInstance = scene.addMeshInstance("room/wall/x+", sceneMesh);

                sceneMeshInstance->mColorDiffuse = kColorBlue;
                sceneMeshInstance->mScenePosition = float32x3_t(+radius, 0.0f, 0.0f);
                sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                    float32x3_t(-1.0f, 0.0f, 0.0f),
                    float32x3_t(0.0f, -1.0f, 0.0f),
                    float32x3_t(0.0f, 0.0f, +1.0f)
                );
            }

            {
                auto sceneMeshInstance = scene.addMeshInstance("room/wall/y+", sceneMesh);

                sceneMeshInstance->mColorDiffuse = kColorWhite;
                sceneMeshInstance->mScenePosition = float32x3_t(0.0f, +radius, 0.0f);
                sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                    float32x3_t(0.0f, -1.0f, 0.0f),
                    float32x3_t(+1.0f, 0.0f, 0.0f),
                    float32x3_t(0.0f, 0.0f, +1.0f)
                );
            }
        }

        { // Z-
            float32x3_t meshVertices[] = {
                float32x3_t(-radius, -radius, 0.0f),
                float32x3_t(+radius, -radius, 0.0f),
                float32x3_t(-radius, +radius, 0.0f),
                float32x3_t(-radius, +radius, 0.0f),
                float32x3_t(+radius, -radius, 0.0f),
                float32x3_t(+radius, +radius, 0.0f),
            };

            CS499R::Mesh mesh(CS499R_ARRAY_SIZE(meshVertices), meshVertices);

            auto sceneMesh = scene.addMesh("room/floor", mesh);
            scene.addMeshInstance("room/floor", sceneMesh);
        }

        { // Z+
            float32x3_t meshVertices[] = {
                float32x3_t(-radius, -radius, +height),
                float32x3_t(-radius, +radius, +height),
                float32x3_t(+radius, -radius, +height),
                float32x3_t(+radius, -radius, +height),
                float32x3_t(-radius, +radius, +height),
                float32x3_t(+radius, +radius, +height),
            };

            CS499R::Mesh mesh(CS499R_ARRAY_SIZE(meshVertices), meshVertices);

            auto sceneMesh = scene.addMesh("room/roof", mesh);
            scene.addMeshInstance("room/roof", sceneMesh);
        }

    }

    void
    buildSceneLights(CS499R::Scene & scene)
    {
        float32_t const kLightSize = 9.0f;
        float32x3_t const kLightColor = 5.0f;

        float32x3_t const vl0 { kRoomWidth - kLightSize, kRoomWidth, kRoomHeight };
        float32x3_t const vl1 { kRoomWidth, kRoomWidth, kRoomHeight - kLightSize };
        float32x3_t const vl2 { kRoomWidth, kRoomWidth - kLightSize, kRoomHeight };

        float32x3_t const meshVertices[] = {
            vl0,
            vl1,
            vl2,
        };

        CS499R::Mesh mesh(CS499R_ARRAY_SIZE(meshVertices), meshVertices);

        auto sceneMesh = scene.addMesh("room/light", mesh);

        {
            auto sceneMeshInstance = scene.addMeshInstance("room/light/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = 0.0f;
            sceneMeshInstance->mColorEmit = kLightColor;
        }

        {
            auto sceneMeshInstance = scene.addMeshInstance("room/light/002", sceneMesh);

            sceneMeshInstance->mColorDiffuse = 0.0f;
            sceneMeshInstance->mColorEmit = kLightColor * 2.0f;

            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(0.0f, -1.0f, 0.0f),
                float32x3_t(+1.0f, 0.0f, 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }

        {
            auto sceneMeshInstance = scene.addMeshInstance("room/light/003", sceneMesh);

            sceneMeshInstance->mColorDiffuse = 0.0f;
            sceneMeshInstance->mColorEmit = kLightColor;

            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(-1.0f, 0.0f, 0.0f),
                float32x3_t(0.0f, -1.0f, 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }

        {
            auto sceneMeshInstance = scene.addMeshInstance("room/light/004", sceneMesh);

            sceneMeshInstance->mColorDiffuse = 0.0f;
            sceneMeshInstance->mColorEmit = kLightColor;

            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(0.0f, +1.0f, 0.0f),
                float32x3_t(-1.0f, 0.0f, 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }
    }

    void
    buildSceneSparsedContent(CS499R::Scene & scene)
    {
        { // Monkey
            CS499R::Mesh mesh("models/monkey.obj");

            auto sceneMesh = scene.addMesh("monkey", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("monkey/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(3.0f, 1.0f, 0.0f);
            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(0.0f, +1.0f, 0.0f),
                float32x3_t(-1.0f, 0.0f, 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }

        { // Torus
            CS499R::Mesh mesh("models/torus.obj");

            auto sceneMesh = scene.addMesh("torus", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("torus/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(8.0f, -4.0f, 0.25f);
        }

        { // Sphere
            CS499R::Mesh mesh("models/sphere.obj");

            auto sceneMesh = scene.addMesh("sphere", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("sphere/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(10.0f, 8.0f, 1.0f);
        }

        { // Cone
            CS499R::Mesh mesh("models/cone.obj");

            auto sceneMesh = scene.addMesh("cone", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("cone/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(5.0f, 12.0f, 1.0f);
        }

        { // Cube
            float32_t const angle = 0.85f * kPi;
            CS499R::Mesh mesh("models/cube_sharp.obj");

            auto sceneMesh = scene.addMesh("cube", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("cube/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(15.0f, 1.0f, 1.0f);
            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(cos(angle), sin(angle), 0.0f),
                float32x3_t(-sin(angle), cos(angle), 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }

        { // Bunny
            CS499R::Mesh mesh("models/stanford_bunny.obj");

            auto sceneMesh = scene.addMesh("bunny", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("bunny/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(-1.5f, -4.0f, 1.0f);
            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(-1.0f, 0.0f, 0.0f),
                float32x3_t(0.0f, -1.0f, 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }

        { // Teapot
            float32_t const angle = 0.6f * kPi;
            CS499R::Mesh mesh("models/utah_teapot.obj");

            auto sceneMesh = scene.addMesh("teapot", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("teapot/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(-3.0f, 3.0f, 0.0f);
            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(cos(angle), sin(angle), 0.0f),
                float32x3_t(-sin(angle), cos(angle), 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }
    }

    void
    buildScenePyramidContent(CS499R::Scene & scene, char const * const meshPath)
    {
        CS499R::Mesh mesh(meshPath);

        float32_t const sphereRadius = 1.0f;
        float32_t const pyramidAngle = 0.6f;
        size_t const pyramidSize = 3;
        auto const sceneMesh = scene.addMesh("sphere", mesh);

        size_t i = 1;
        for (size_t layerId = 0; layerId < pyramidSize; layerId++)
        {
            for (size_t y = 0; y <= layerId; y++)
            {
                for (size_t x = 0; x <= y; x++)
                {
                    std::ostringstream ss;
                    ss << "sphere/" << std::right << std::setw(3);
                    ss << i++;

                    auto const sceneMeshInstance = scene.addMeshInstance(ss.str(), sceneMesh);

                    float32x3_t const instancePosition (sphereRadius * float32x3_t(
                        (
                            2.0f * float32_t(x) -
                            (float32_t(pyramidSize) - 1.0f) -
                            float32_t(y)
                        ),
                        sqrt(3.0f) * (
                            float32_t(y) -
                            (2.0f / 3.0f) * float32_t(layerId)
                        ),
                        1.0f + 2.0f * sqrt(3.0f / 4.0f - 1.0f / 9.0f) * (pyramidSize - layerId - 1)
                    ));

                    sceneMeshInstance->mColorDiffuse = kColorWhite;
                    sceneMeshInstance->mScenePosition = float32x3_t(
                        instancePosition.x * cos(pyramidAngle) - instancePosition.y * sin(pyramidAngle) + 2.0f,
                        instancePosition.x * sin(pyramidAngle) + instancePosition.y * cos(pyramidAngle) + 2.0f,
                        instancePosition.z
                    );
                }
            }
        }
    }

}

namespace App
{

    void
    buildSceneMeshes(CS499R::Scene & scene)
    {
        buildSceneRoom(scene);
        buildSceneLights(scene);
        //buildSceneSparsedContent(scene);
        buildScenePyramidContent(scene, "models/sphere.obj");
    }

}
