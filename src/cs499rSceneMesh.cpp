
#include <list>

#include "cs499rMesh.hpp"
#include "cs499rSceneMesh.hpp"

namespace
{

    size_t const kOctreeMinPrimitivePerLeaf = 2;

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
         * Compute the node's size
         */
        inline
        float32_t
        size() const
        {
            return pow(2.0f, mSizeLog);
        }

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
#if CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
                commonNode->subNodeCount = 0;
#endif
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

                commonNode->subNodeOffsets[i] = subNodeId;
#if CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
                commonNode->subNodeCount ++;
                CS499R_ASSERT(commonNode->subNodeCount <= CS499R_ARRAY_SIZE(mChildren));
#endif

                subNode->generateCommonOctree(
                    outPrimNewIds,
                    outOctreeCommonNodes,
                    cursors
                );
            }

#if CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
            /*
             * generate common node' access lists depending on the direction
             */
            for (size_t directionId = 0; directionId < CS499R_ARRAY_SIZE(mChildren); directionId++)
            {
                uint32_t accessCount = 0;
                uint32_t accessList = 0x0;

                for (size_t nodeAccessId = 0; nodeAccessId < CS499R_ARRAY_SIZE(mChildren); nodeAccessId++)
                {
                    uint32_t nodeId = directionId ^ nodeAccessId;
                    CS499R_ASSERT(nodeId < CS499R_ARRAY_SIZE(mChildren));

                    if (mChildren[nodeId] == nullptr)
                    {
                        continue;
                    }

                    accessList |= nodeId << (4 * accessCount);
                    accessCount++;
                }

                CS499R_ASSERT(accessCount == commonNode->subNodeCount);
                commonNode->subNodeAccessLists[directionId] = accessList;
            }

#endif //CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS
        }

        bool
        isLeaf() const
        {
            for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
            {
                if (mChildren[i] != nullptr)
                {
                    return false;
                }
            }

            return true;
        }

        size_t
        nodeCount() const
        {
            size_t nodeCount = 1;

            for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
            {
                if (mChildren[i] != nullptr)
                {
                    nodeCount += mChildren[i]->nodeCount();
                }
            }

            return nodeCount;
        }

        size_t
        prune()
        {
            size_t prunedNodeCount = 0;

            for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
            {
                if (mChildren[i] == nullptr)
                {
                    continue;
                }

                prunedNodeCount += mChildren[i]->prune();

                if (!mChildren[i]->isLeaf())
                {
                    continue;
                }

                if (mChildren[i]->mPrimitiveIds.size() > kOctreeMinPrimitivePerLeaf)
                {
                    continue;
                }

                for (auto primId : mChildren[i]->mPrimitiveIds)
                {
                    mPrimitiveIds.push_back(primId);
                }

                delete mChildren[i];

                mChildren[i] = nullptr;

                prunedNodeCount++;
            }

            return prunedNodeCount;
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

        auto const octreeBoxSizeLog = int32_t(ceil(log2(max(upperBound - lowerBound))));
        auto const octreeBoxSize = float32_t(pow(2.0f, octreeBoxSizeLog));

        OctreeNode * const octreeRoot = new OctreeNode(octreeBoxSize * 0.5f + lowerBound, octreeBoxSizeLog);
        size_t primCount = 0;

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
                auto currentOctreeNode = octreeRoot;

                // seek the correct node who should contain this primitive
                for (;;)
                {
                    auto const currentNodeSize = currentOctreeNode->size();

                    if (currentNodeSize * 0.5f < primBiggestDimSize)
                    {
                        break;
                    }

                    CS499R_ASSERT(abs(primCenter.x - currentOctreeNode->mCenter.x) <= currentNodeSize);
                    CS499R_ASSERT(abs(primCenter.y - currentOctreeNode->mCenter.y) <= currentNodeSize);
                    CS499R_ASSERT(abs(primCenter.z - currentOctreeNode->mCenter.z) <= currentNodeSize);

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
                    }

                    currentOctreeNode = currentOctreeNode->mChildren[subNodeId];
                }

                {
                    auto const currentNodeSize = currentOctreeNode->size();

                    CS499R_ASSERT(primBiggestDimSize >= currentNodeSize * 0.5f);
                    CS499R_ASSERT(primBiggestDimSize <= currentNodeSize);

                    CS499R_ASSERT(primLowerBound.x >= currentOctreeNode->mCenter.x - currentNodeSize);
                    CS499R_ASSERT(primLowerBound.y >= currentOctreeNode->mCenter.y - currentNodeSize);
                    CS499R_ASSERT(primLowerBound.z >= currentOctreeNode->mCenter.z - currentNodeSize);
                    CS499R_ASSERT(primUpperBound.x <= currentOctreeNode->mCenter.x + currentNodeSize);
                    CS499R_ASSERT(primUpperBound.y <= currentOctreeNode->mCenter.y + currentNodeSize);
                    CS499R_ASSERT(primUpperBound.z <= currentOctreeNode->mCenter.z + currentNodeSize);
                }

                currentOctreeNode->mPrimitiveIds.push_back(primId);
                primCount++;
            }
        }

        { // prunes leaf that are to small
            octreeRoot->prune();
        }

        { // inits scene mesh's members
            mPrimitiveCount = primCount;
            mPrimitiveArray = alloc<common_primitive_t>(mPrimitiveCount);
            mOctreeNodeCount = octreeRoot->nodeCount();
            mOctreeNodeArray = alloc<common_mesh_octree_node_t>(mOctreeNodeCount);
            mCenterPosition = -lowerBound;
            mVertexUpperBound = upperBound - lowerBound;
            mOctreeRootHalfSize = octreeBoxSize * 0.5f;
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
            CS499R_ASSERT(primNewIds[primId] < mPrimitiveCount);

            auto const originalPrimitive = mesh.mPrimitiveArray + primId;
            auto const commonPrimitive = mPrimitiveArray + primNewIds[primId];

            float32x3_t const v0 = mCenterPosition + originalPrimitive->vertex[0];
            float32x3_t const e0 = originalPrimitive->vertex[1] - originalPrimitive->vertex[0];
            float32x3_t const e1 = originalPrimitive->vertex[2] - originalPrimitive->vertex[0];
            float32x3_t const normal = normalize(cross(e0, e1));

            float32_t const basisDot = dot(e0, e1);
            float32x2_t const invSquareLenght = 1.0f / float32x2_t(dot(e0, e0), dot(e1, e1));
            float32x2_t const invSquareLenghtBasisDot = basisDot * invSquareLenght;
            float32_t const invDet = 1.0f / (1.0f - invSquareLenghtBasisDot.x * invSquareLenghtBasisDot.y);
            float32x2_t const factorAIdot = invDet * invSquareLenght;

            commonPrimitive->v0 = float32x4_t(v0.x, v0.y, v0.z, normal.x);
            commonPrimitive->e0 = float32x4_t(e0.x, e0.y, e0.z, normal.y);
            commonPrimitive->e1 = float32x4_t(e1.x, e1.y, e1.z, normal.z);
            commonPrimitive->uvMatrix.x.x = factorAIdot.x;
            commonPrimitive->uvMatrix.y.x = - factorAIdot.y * invSquareLenghtBasisDot.x;
            commonPrimitive->uvMatrix.x.y = - factorAIdot.x * invSquareLenghtBasisDot.y;
            commonPrimitive->uvMatrix.y.y = factorAIdot.y;
        }

        delete octreeRoot;
        delete [] primNewIds;
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
