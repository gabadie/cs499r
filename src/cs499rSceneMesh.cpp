
#include "cs499rMesh.hpp"
#include "cs499rOctree.hpp"
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

        free(mOctreeNodeArray);
        free(mPrimitiveArray);
    }

    void
    SceneMesh::buildFromMesh(Mesh const & mesh)
    {
        CS499R_ASSERT(mesh.mPrimitiveCount != 0);

        float32x3_t lowerBound;
        float32x3_t upperBound;

        mesh.computeBoundingBox(&lowerBound, &upperBound);

        Octree<size_t> octree(lowerBound, upperBound);

        { // build up mesh's octree
            /*
             * We loop over all primitive to insert them one by one.
             */

            for (size_t primId = 0; primId < mesh.mPrimitiveCount; primId++)
            {
                auto const prim = mesh.mPrimitiveArray + primId;
                auto const primLowerBound = min(prim->vertex[0], min(prim->vertex[1], prim->vertex[2]));
                auto const primUpperBound = max(prim->vertex[0], max(prim->vertex[1], prim->vertex[2]));
                auto const primCenter = (primLowerBound + primUpperBound) * 0.5f;
                auto const primBiggestDimSize = max(primUpperBound - primLowerBound);

                octree.insert(primId, primCenter, primBiggestDimSize);
            }
        }

        { // optimize the octree for more performance
            octree.optimize();
        }

        { // inits scene mesh's members
            mPrimitiveCount = octree.primCount();
            mPrimitiveArray = alloc<common_primitive_t>(mPrimitiveCount);
            mOctreeNodeCount = octree.nodeCount();
            mOctreeNodeArray = alloc<common_octree_node_t>(mOctreeNodeCount);
            mCenterPosition = -lowerBound;
            mVertexUpperBound = upperBound - lowerBound;
            mOctreeRootHalfSize = octree.boxSize() * 0.5f;
        }

        auto const primNewOrder = new size_t[mPrimitiveCount];

        { // export new prim's id and octree node array
            octree.exportToCommonOctreeNodeArray(
                primNewOrder,
                mOctreeNodeArray
            );
        }

        // export primitives
        for (size_t primId = 0; primId < mPrimitiveCount; primId++)
        {
            CS499R_ASSERT(primNewOrder[primId] < mPrimitiveCount);

            mesh.mPrimitiveArray[primNewOrder[primId]].exportToCommonPrimitive(
                mPrimitiveArray + primId,
                mCenterPosition
            );
        }

        delete [] primNewOrder;
    }

    void
    SceneMesh::exportToCommonMesh(
        SceneMesh::SceneBufferCtx const & ctx,
        common_mesh_t * outMesh
    ) const
    {
        CS499R_STATIC_ASSERT(sizeof(common_mesh_t) == sizeof(uint32_t) * 8);

        auto const sceneMeshPrimFirst = ctx.meshPrimitivesGlobalOffsets.find(this)->second;
        auto const sceneMeshOctreeRootId = ctx.meshOctreeRootGlobalId.find(this)->second;

        outMesh->primFirst = sceneMeshPrimFirst;
        outMesh->primCount = mPrimitiveCount;
        outMesh->octreeRootGlobalId = sceneMeshOctreeRootId;
        outMesh->octreeNodeCount = mOctreeNodeCount;
        outMesh->vertexUpperBound.x = mVertexUpperBound.x;
        outMesh->vertexUpperBound.y = mVertexUpperBound.y;
        outMesh->vertexUpperBound.z = mVertexUpperBound.z;
        outMesh->vertexUpperBound.w = mOctreeRootHalfSize;
    }

}
