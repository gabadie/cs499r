
#include "cs499rMesh.hpp"
#include "cs499rSceneMesh.hpp"


namespace CS499R
{

    SceneMesh::SceneMesh(
        Scene * const scene,
        std::string const & objectName,
        Mesh const & mesh
    )
        : SceneObject(scene, objectName)
    {
        buildFromMesh(mesh);

        CS499R_ASSERT(mPrimitiveCount != 0);
    }

    SceneMesh::~SceneMesh()
    {
        CS499R_ASSERT(mPrimitiveCount != 0);

        free(mPrimitiveArray);
    }

    void
    SceneMesh::buildFromMesh(Mesh const & mesh)
    {
        CS499R_ASSERT(mesh.mPrimitiveCount != 0);

        mPrimitiveCount = mesh.mPrimitiveCount;
        mPrimitiveArray = alloc<common_primitive_t>(mPrimitiveCount);

        for (size_t primId = 0; primId < mPrimitiveCount; primId++)
        {
            mPrimitiveArray[primId].v0 = mesh.mPrimitiveArray[primId].vertex[0];
            mPrimitiveArray[primId].v1 = mesh.mPrimitiveArray[primId].vertex[1];
            mPrimitiveArray[primId].v2 = mesh.mPrimitiveArray[primId].vertex[2];
        }
    }

}
