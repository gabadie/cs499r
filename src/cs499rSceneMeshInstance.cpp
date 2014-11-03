
#include "cs499rSceneMesh.hpp"
#include "cs499rSceneMeshInstance.hpp"


namespace CS499R
{

    void
    SceneMeshInstance::computeBoundingBox(float32x3_t * outLowerBound, float32x3_t * outUpperBound) const
    {
        float32x3_t lowerBound = +INFINITY;
        float32x3_t upperBound = -INFINITY;

        for (size_t primId = 0; primId < mSceneMesh->mPrimitiveCount; primId++)
        {
            auto const * const prim = mSceneMesh->mPrimitiveArray + primId;

            float32x3_t const v0 = (
                mMeshSceneMatrix.x * prim->v0.x +
                mMeshSceneMatrix.y * prim->v0.y +
                mMeshSceneMatrix.z * prim->v0.z +
                mScenePosition - dot(mMeshSceneMatrix, mSceneMesh->mCenterPosition)
            );

            float32x3_t const v1 = (
                mMeshSceneMatrix.x * prim->e0.x +
                mMeshSceneMatrix.y * prim->e0.y +
                mMeshSceneMatrix.z * prim->e0.z +
                v0
            );

            float32x3_t const v2 = (
                mMeshSceneMatrix.x * prim->e1.x +
                mMeshSceneMatrix.y * prim->e1.y +
                mMeshSceneMatrix.z * prim->e1.z +
                v0
            );

            lowerBound = min(lowerBound, v0);
            lowerBound = min(lowerBound, v1);
            lowerBound = min(lowerBound, v2);

            upperBound = max(upperBound, v0);
            upperBound = max(upperBound, v1);
            upperBound = max(upperBound, v2);
        }

        *outLowerBound = lowerBound;
        *outUpperBound = upperBound;
    }

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
