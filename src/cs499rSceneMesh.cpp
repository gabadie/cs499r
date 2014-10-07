
#include <list>

#include "cs499rMesh.hpp"
#include "cs499rSceneMesh.hpp"

namespace
{

    /*
     * Temporary octree struct for SceneMesh::buildFromMesh()
     */
    struct OctreeNode
    {
        // --------------------------------------------------------------------- MEMBERS

        // children octree nodes (x + 2 * y + 4 * z)
        OctreeNode * mChildren[8];

        // the node's center in the mesh's basis
        float32x3_t const mCenter;

        // the node's logarithmic size
        int32_t const mSizeLog;

        // primitive id list in that node
        std::list<size_t> mPrimitiveIds;


        // --------------------------------------------------------------------- METHODS

        /*
         * Compute the sub node id from its sub coordinate
         */
        static
        inline
        size_t
        subNodeIdFromCoord(size3_t const & subNodeCoord)
        {
            return subNodeCoord.x + 2 * subNodeCoord.y + 4 * subNodeCoord.z;
        }

        /*
         *
         */
        void
        generateCommonOctree(
            size_t * outPrimNewIds,
            CS499R::common_mesh_octree_node_t * outOctreeCommonNodes,
            size_t cursors[2]
        ) const
        {
            auto const nodeId = cursors[0];
            auto const primOffset = cursors[1];
            auto const commonNode = outOctreeCommonNodes + nodeId;

            cursors[0] += 1;
            cursors[1] += mPrimitiveIds.size();

            { // export primitive ids list
                auto primNewId = primOffset;

                for (auto primId : mPrimitiveIds)
                {
                    outPrimNewIds[primId] = primNewId;
                    primNewId++;
                }
            }

            { // export
                commonNode->primFirst = primOffset;
                commonNode->primCount = mPrimitiveIds.size();
            }

            // export sub offsets
            for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
            {
                auto const subNode = mChildren[i];

                if (subNode == nullptr)
                {
                    commonNode->subNodeOffsets[i] = 0;
                    continue;
                }

                auto const subNodeId = cursors[0];

                commonNode->subNodeOffsets[i] = subNodeId - nodeId;

                subNode->generateCommonOctree(
                    outPrimNewIds,
                    outOctreeCommonNodes,
                    cursors
                );
            }
        }


        // --------------------------------------------------------------------- IDLE

        inline
        OctreeNode(float32x3_t const & center, int32_t sizeLog)
            : mCenter(center)
            , mSizeLog(sizeLog)
        {
            for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
            {
                mChildren[i] = nullptr;
            }
        }

        inline
        ~OctreeNode()
        {
            for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
            {
                if (mChildren[i] != nullptr)
                {
                    delete mChildren[i];
                }
            }
        }

    };

}


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

        auto const octreeBoxSizeLog = int32_t(ceil(log(max(upperBound - lowerBound))));
        auto const octreeBoxSize = float32_t(pow(2.0f, octreeBoxSizeLog));

        OctreeNode * const octreeRoot = new OctreeNode(octreeBoxSize * 0.5f + lowerBound, octreeBoxSizeLog);
        size_t octreeNodeCount = 1;

        { // build up mesh's octree
            /*
             * We loop over all primitive to insert them one by one.
             */

            for (size_t primId = 0; primId < mesh.mPrimitiveCount; primId++)
            {
                auto prim = mesh.mPrimitiveArray + primId;
                auto primCenter = (prim->vertex[0] + prim->vertex[1] + prim->vertex[2]) * 0.3333f;

                float32_t const primLongestEdge = max(float32x3_t(
                    length(prim->vertex[0] - prim->vertex[1]),
                    length(prim->vertex[1] - prim->vertex[2]),
                    length(prim->vertex[2] - prim->vertex[0])
                ));

                auto const primOctreeDepth = int32_t(ceil(log(primLongestEdge / sqrt(3.0f))));
                auto currentOctreeNode = octreeRoot;

                CS499R_ASSERT(primOctreeDepth <= currentOctreeNode->mSizeLog);

                // seek the correct node who should contain this primitive
                while (currentOctreeNode->mSizeLog != primOctreeDepth)
                {
                    auto const subNodeCoord = size3_t(
                        size_t(primCenter.x > currentOctreeNode->mCenter.x),
                        size_t(primCenter.y > currentOctreeNode->mCenter.y),
                        size_t(primCenter.z > currentOctreeNode->mCenter.z)
                    );

                    auto const subNodeId = OctreeNode::subNodeIdFromCoord(subNodeCoord);

                    if (currentOctreeNode->mChildren[subNodeId] == nullptr)
                    {
                        // this octree sub node was not existing yet, so we create it.

                        auto const subNodeSizeLog = currentOctreeNode->mSizeLog - 1;
                        float32_t const subNodeSize = pow(2.0f, subNodeSizeLog);
                        auto const subNodeCenterOffset = subNodeSize * 0.5f;
                        auto const subNodeCenter = currentOctreeNode->mCenter + float32x3_t(
                            subNodeCoord.x ? subNodeCenterOffset : -subNodeCenterOffset,
                            subNodeCoord.y ? subNodeCenterOffset : -subNodeCenterOffset,
                            subNodeCoord.z ? subNodeCenterOffset : -subNodeCenterOffset
                        );

                        currentOctreeNode->mChildren[subNodeId] = new OctreeNode(
                            subNodeCenter,
                            subNodeSizeLog
                        );

                        octreeNodeCount++;
                    }

                    currentOctreeNode = currentOctreeNode->mChildren[subNodeId];
                }

                currentOctreeNode->mPrimitiveIds.push_back(primId);
            }
        }

        { // inits scene mesh's members
            mPrimitiveCount = mesh.mPrimitiveCount;
            mPrimitiveArray = alloc<common_primitive_t>(mPrimitiveCount);
            mOctreeNodeCount = octreeNodeCount;
            mOctreeNodeArray = alloc<common_mesh_octree_node_t>(mOctreeNodeCount);
            mCenterPosition = -lowerBound;
        }

        auto const primNewIds = new size_t[mPrimitiveCount];

        { // export new prim's id and octree node array
            size_t cursors[] = { 0, 0 };

            octreeRoot->generateCommonOctree(
                primNewIds,
                mOctreeNodeArray,
                cursors
            );
        }

        // export primitives
        for (size_t primId = 0; primId < mPrimitiveCount; primId++)
        {
            auto const originalPrimitive = mesh.mPrimitiveArray + primId;
            auto const commonPrimitive = mPrimitiveArray + primNewIds[primId];

            commonPrimitive->v0 = mCenterPosition + originalPrimitive->vertex[0];
            commonPrimitive->v1 = mCenterPosition + originalPrimitive->vertex[1];
            commonPrimitive->v2 = mCenterPosition + originalPrimitive->vertex[2];
        }

        delete octreeRoot;
        delete [] primNewIds;
    }

}
