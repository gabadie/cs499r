
#include "cs499rOctree.hpp"
#include "cs499rOctreeNode.hpp"


namespace CS499R
{

    void
    Octree::insert(size_t primId, float32x3_t primCenter, float32_t primBiggestDimSize)
    {
        auto currentOctreeNode = mRoot;

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

            CS499R_ASSERT(primCenter.x >= currentOctreeNode->mCenter.x - currentNodeSize * 0.5f);
            CS499R_ASSERT(primCenter.y >= currentOctreeNode->mCenter.y - currentNodeSize * 0.5f);
            CS499R_ASSERT(primCenter.z >= currentOctreeNode->mCenter.z - currentNodeSize * 0.5f);
            CS499R_ASSERT(primCenter.x <= currentOctreeNode->mCenter.x + currentNodeSize * 0.5f);
            CS499R_ASSERT(primCenter.y <= currentOctreeNode->mCenter.y + currentNodeSize * 0.5f);
            CS499R_ASSERT(primCenter.z <= currentOctreeNode->mCenter.z + currentNodeSize * 0.5f);
        }

        currentOctreeNode->mPrimitiveIds.push_back(primId);

        mPrimCount++;
    }

    size_t
    Octree::optimize()
    {
        return mRoot->optimize();
    }

    void
    Octree::exportToCommonOctreeNodeArray(
        size_t * outPrimNewIds,
        common_octree_node_t * outOctreeCommonNodes
    ) const
    {
        size_t cursors[] = { 0, 0 };

        mRoot->exportToCommonOctreeNode(
            outPrimNewIds,
            outOctreeCommonNodes,
            cursors
        );
    }

    size_t
    Octree::nodeCount() const
    {
        return mRoot->nodeCount();
    }

    Octree::Octree(
        float32x3_t const & lowerBound,
        float32x3_t const & upperBound
    )
    {
        mPrimCount = 0;
        mBoxSizeLog = int32_t(ceil(log2(max(upperBound - lowerBound))));

        mRoot = new OctreeNode(boxSize() * 0.5f + lowerBound, mBoxSizeLog);
    }

    Octree::~Octree()
    {
        delete mRoot;
    }

}
