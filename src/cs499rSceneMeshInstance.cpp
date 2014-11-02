
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
        CompiledScene::CompilationCtx const & compilationCtx,
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

        mSceneMesh->exportToCommonMesh(compilationCtx, &outMeshInstance->mesh);
    }

    void
    SceneMeshInstance::exportAnonymousToCommonMeshInstance(
        common_mesh_instance_t * outAnonymousMesh
    )
    {
        /*
         * The anonymous mesh has completly null matrices in order to
         * have a grey background on normal debuging.
         */
        outAnonymousMesh->meshSceneMatrix.x = 0.0f;
        outAnonymousMesh->meshSceneMatrix.y = 0.0f;
        outAnonymousMesh->meshSceneMatrix.z = 0.0f;
        outAnonymousMesh->meshSceneMatrix.w = 0.0f;
        outAnonymousMesh->sceneMeshMatrix.x = 0.0f;
        outAnonymousMesh->sceneMeshMatrix.y = 0.0f;
        outAnonymousMesh->sceneMeshMatrix.z = 0.0f;
        outAnonymousMesh->sceneMeshMatrix.w = 0.0f;

        outAnonymousMesh->diffuseColor = 0.0f;
        outAnonymousMesh->emitColor = 0.0f;
        outAnonymousMesh->mesh.primFirst = 0;
        outAnonymousMesh->mesh.primCount = 0;
    }


}
