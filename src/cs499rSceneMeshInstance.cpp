
#include "cs499rSceneMesh.hpp"
#include "cs499rSceneMeshInstance.hpp"


namespace CS499R
{

    SceneMeshInstance::SceneMeshInstance(
        Scene * const scene,
        std::string const & objectName,
        SceneMesh * sceneMesh
    )
        : SceneObject(scene, objectName)
        , mMeshSceneMatrix(identity<float32x3x3_t>())
        , mScenePosition(0.0f)
        , mSceneMesh(sceneMesh)
        , mColorDiffuse(0.75f)
        , mColorEmit(0.0f)
    { }

    SceneMeshInstance::~SceneMeshInstance()
    { }

}
