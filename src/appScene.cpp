
#include "app.hpp"


namespace App
{

    void
    buildSceneMeshes(CS499R::Scene & scene)
    {
        float32_t const radius = 20.0f;
        float32_t const height = 10.0f;;
        float32_t const lightSize = 9.0f;

        float32x3_t const colorRed(0.75f, 0.25f, 0.25f);
        float32x3_t const colorBlue(0.25f, 0.25f, 0.75f);
        float32x3_t const colorWhite(0.75f, 0.75f, 0.75f);
        float32x3_t const colorLight(5.0f);


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

                sceneMeshInstance->mColorDiffuse = colorRed;
                sceneMeshInstance->mScenePosition = float32x3_t(-radius, 0.0f, 0.0f);
                sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                    float32x3_t(+1.0f, 0.0f, 0.0f),
                    float32x3_t(0.0f, +1.0f, 0.0f),
                    float32x3_t(0.0f, 0.0f, +1.0f)
                );
            }

            {
                auto sceneMeshInstance = scene.addMeshInstance("room/wall/y-", sceneMesh);

                sceneMeshInstance->mColorDiffuse = colorWhite;
                sceneMeshInstance->mScenePosition = float32x3_t(0.0f, -radius, 0.0f);
                sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                    float32x3_t(0.0f, +1.0f, 0.0f),
                    float32x3_t(-1.0f, 0.0f, 0.0f),
                    float32x3_t(0.0f, 0.0f, +1.0f)
                );
            }

            {
                auto sceneMeshInstance = scene.addMeshInstance("room/wall/x+", sceneMesh);

                sceneMeshInstance->mColorDiffuse = colorBlue;
                sceneMeshInstance->mScenePosition = float32x3_t(+radius, 0.0f, 0.0f);
                sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                    float32x3_t(-1.0f, 0.0f, 0.0f),
                    float32x3_t(0.0f, -1.0f, 0.0f),
                    float32x3_t(0.0f, 0.0f, +1.0f)
                );
            }

            {
                auto sceneMeshInstance = scene.addMeshInstance("room/wall/y+", sceneMesh);

                sceneMeshInstance->mColorDiffuse = colorWhite;
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

        { // light
            float32x3_t vl0 { radius - lightSize, radius, height };
            float32x3_t vl1 { radius, radius, height - lightSize };
            float32x3_t vl2 { radius, radius - lightSize, height };

            float32x3_t meshVertices[] = {
                vl0,
                vl1,
                vl2,
            };

            CS499R::Mesh mesh(CS499R_ARRAY_SIZE(meshVertices), meshVertices);

            auto sceneMesh = scene.addMesh("room/light", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("room/light", sceneMesh);

            sceneMeshInstance->mColorDiffuse = colorLight;
            sceneMeshInstance->mColorEmit = colorLight;
        }

        { // Monkey
            CS499R::Mesh mesh("models/monkey.obj");

            auto sceneMesh = scene.addMesh("monkey", mesh);
            auto sceneMeshInstance = scene.addMeshInstance("monkey/001", sceneMesh);

            sceneMeshInstance->mColorDiffuse = colorWhite;
            sceneMeshInstance->mScenePosition = float32x3_t(-0.0f, -0.0f, 2.0f);
            sceneMeshInstance->mMeshSceneMatrix = float32x3x3_t(
                float32x3_t(0.0f, +1.0f, 0.0f),
                float32x3_t(-1.0f, 0.0f, 0.0f),
                float32x3_t(0.0f, 0.0f, +1.0f)
            );
        }
    }

}
