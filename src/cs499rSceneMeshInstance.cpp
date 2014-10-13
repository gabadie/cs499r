
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

    void
    SceneMeshInstance::exportToCommonMeshInstance(
        SceneMesh::SceneBufferCtx const & ctx,
        common_mesh_instance_t * outMeshInstance
    ) const
    {
        outMeshInstance->meshSceneMatrix.x = mMeshSceneMatrix.x;
        outMeshInstance->meshSceneMatrix.y = mMeshSceneMatrix.y;
        outMeshInstance->meshSceneMatrix.z = mMeshSceneMatrix.z;
        outMeshInstance->meshSceneMatrix.w = (
            mScenePosition -
            dot(mMeshSceneMatrix, mSceneMesh->mCenterPosition)
        );

        auto sceneMeshMatrix = transpose(mMeshSceneMatrix);

        outMeshInstance->sceneMeshMatrix.x = sceneMeshMatrix.x;
        outMeshInstance->sceneMeshMatrix.y = sceneMeshMatrix.y;
        outMeshInstance->sceneMeshMatrix.z = sceneMeshMatrix.z;
        outMeshInstance->sceneMeshMatrix.w = (
            mSceneMesh->mCenterPosition -
            dot(sceneMeshMatrix, mScenePosition)
        );

        outMeshInstance->diffuseColor = mColorDiffuse;
        outMeshInstance->emitColor = mColorEmit;

        mSceneMesh->exportToCommonMesh(ctx, &outMeshInstance->mesh);
    }


}
