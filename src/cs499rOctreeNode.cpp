
#include "cs499rOctreeNode.hpp"

namespace
{

    size_t const kOctreeMinPrimitivePerLeaf = 2;

}

namespace CS499R
{

    void
    OctreeNode::exportToCommonOctreeNode(
        size_t * outPrimNewIds,
        common_mesh_octree_node_t * outOctreeCommonNodes,
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

            subNode->exportToCommonOctreeNode(
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
    OctreeNode::isLeaf() const
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
    OctreeNode::nodeCount() const
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
    OctreeNode::optimize()
    {
        size_t prunedNodeCount = 0;

        for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
        {
            if (mChildren[i] == nullptr)
            {
                continue;
            }

            prunedNodeCount += mChildren[i]->optimize();

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


}
