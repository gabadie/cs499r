
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
    buildSceneContent(CS499R::Scene & scene)
    {
        { // Monkey
            CS499R::Mesh mesh("models/monkey.obj");

            auto sceneMesh = scene.addMesh("monkey", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("monkey/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = kColorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(-0.0f, -0.0f, 2.0f);
            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(0.0f, +1.0f, 0.0f),
                float32x3_t(-1.0f, 0.0f, 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
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
        buildSceneContent(scene);
    }

}
