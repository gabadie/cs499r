
#ifndef _H_CS499R_OCTREE
#define _H_CS499R_OCTREE

#include <list>

#include "cs499rCommonStruct.hpp"


namespace CS499R
{

    size_t const kOctreeMinPrimitivePerLeaf = 2;


    /*
     * Octree recursively subdivides space in 8 boxes, each containing a list of
     * primitive indexes (could be index for a mesh or a mesh's triangle).
     */
    template <typename PRIM>
    class Octree
    {
    public:
        // --------------------------------------------------------------------- METHODS

        /*
         * Inserts a primitive id at the correct octree node with the given
         * geometric information parameters
         */
        void
        insert(PRIM primId, float32x3_t primCenter, float32_t primBiggestDimSize)
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

                auto const subNodeId = subNodeIdFromCoord(subNodeCoord);

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

        /*
         * Optimizes the octree
         */
        size_t
        optimize()
        {
#if CS499R_CONFIG_ENABLE_OCTREE_OPTIMISATION_STAGE
            return mRoot->optimize();
#else
            return 0;
#endif
        }

        /*
         * Exports the octree to common nodes, and returns the ordered
         * primitives list
         */
        void
        exportToCommonOctreeNodeArray(
            PRIM * outPrimOrderedList,
            common_octree_node_t * outOctreeCommonNodes,
            size_t firstPrimId = 0
        ) const
        {
            ExportCtx exportCtx;
            exportCtx.nodeIdCounter = 0;
            exportCtx.primOffsetCounter = 0;
            exportCtx.primFirstId = firstPrimId;

            auto const rootNodeId = mRoot->selfExportToCommonOctreeNode(
                outPrimOrderedList,
                outOctreeCommonNodes,
                exportCtx
            );

            CS499R_ASSERT(rootNodeId == 0);

            mRoot->exportToCommonOctreeNode(
                outPrimOrderedList,
                outOctreeCommonNodes,
                exportCtx,
                rootNodeId
            );
        }

        size_t
        boxSize() const
        {
            return float32_t(pow(2.0f, mBoxSizeLog));
        }

        size_t
        primCount() const
        {
            return mPrimCount;
        }

        size_t
        nodeCount() const
        {
            return mRoot->nodeCount();
        }


        // --------------------------------------------------------------------- IDLE

        Octree(
            float32x3_t const & lowerBound,
            float32x3_t const & upperBound
        )
        {
            mPrimCount = 0;
            mBoxSizeLog = int32_t(ceil(log2(max(upperBound - lowerBound))));

            mRoot = new OctreeNode(boxSize() * 0.5f + lowerBound, mBoxSizeLog);
        }

        ~Octree()
        {
            delete mRoot;
        }


    private:
        // --------------------------------------------------------------------- PRIVATE STRUCTS

        /*
         * Octree export counters
         */
        struct ExportCtx
        {
            size_t nodeIdCounter;
            size_t primOffsetCounter;
            size_t primFirstId;
        };

        /*
         * An octree node
         */
        class OctreeNode
        {
        public:
            // ------------------------------------------------------------- METHODS

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
             * Exports this node to common_mesh_octree_node_t
             */
            size_t
            selfExportToCommonOctreeNode(
                PRIM * outPrimOrderedList,
                common_octree_node_t * outOctreeCommonNodes,
                ExportCtx & exportCtx
            ) const
            {
                auto const nodeId = exportCtx.nodeIdCounter;
                auto const primOffset = exportCtx.primOffsetCounter;
                auto const commonNode = outOctreeCommonNodes + nodeId;

                commonNode->primFirst = exportCtx.primFirstId + primOffset;
                commonNode->primCount = mPrimitiveIds.size();

                exportCtx.nodeIdCounter += 1;
                exportCtx.primOffsetCounter += mPrimitiveIds.size();

                { // export primitive ids list
                    auto primNewId = primOffset;

                    for (auto primId : mPrimitiveIds)
                    {
                        outPrimOrderedList[primNewId] = primId;
                        primNewId++;
                    }
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

                    commonNode->subNodeCount = accessCount;
                    commonNode->subNodeAccessLists[directionId] = accessList;
                }

#endif //CS499R_CONFIG_ENABLE_OCTREE_ACCESS_LISTS

                return nodeId;
            }

            /*
             * Exports to common_mesh_octree_node_t
             */
            void
            exportToCommonOctreeNode(
                PRIM * outPrimOrderedList,
                common_octree_node_t * outOctreeCommonNodes,
                ExportCtx & exportCtx,
                size_t nodeId
            ) const
            {
                auto const commonNode = outOctreeCommonNodes + nodeId;

                // export sub offsets
                for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
                {
                    auto const subNode = mChildren[i];

                    if (subNode == nullptr)
                    {
                        commonNode->subNodeOffsets[i] = 0;
                        continue;
                    }

                    commonNode->subNodeOffsets[i] = subNode->selfExportToCommonOctreeNode(
                        outPrimOrderedList,
                        outOctreeCommonNodes,
                        exportCtx
                    );

#if CS499R_CONFIG_ENABLE_OCTREE_CONSECUTIVE_SUBNODES
                }

                for (size_t i = 0; i < CS499R_ARRAY_SIZE(mChildren); i++)
                {
                    auto const subNode = mChildren[i];

                    if (subNode == nullptr)
                    {
                        continue;
                    }
#endif

                    subNode->exportToCommonOctreeNode(
                        outPrimOrderedList,
                        outOctreeCommonNodes,
                        exportCtx,
                        commonNode->subNodeOffsets[i]
                    );
                }
            }

            /*
             * Returns if this is a leaf node (no children)
             */
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

            /*
             * Returns the total number of children + himself
             */
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

            /*
             * Optimizes node's children
             */
            size_t
            optimize()
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


            // ------------------------------------------------------------- IDLE

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


            // ------------------------------------------------------------- MEMBERS

            // children octree nodes (x + 2 * y + 4 * z)
            OctreeNode * mChildren[8];

            // the node's center in the mesh's basis
            float32x3_t const mCenter;

            // the node's logarithmic size
            int32_t const mSizeLog;

            // primitive id list in that node
            std::list<PRIM> mPrimitiveIds;

        };


        // --------------------------------------------------------------------- FUNCTIONS


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


        // --------------------------------------------------------------------- MEMBERS

        // the octree's root
        OctreeNode * mRoot;

        // the octree's root log size
        int32_t mBoxSizeLog;

        // the octree's primitives count
        size_t mPrimCount;

    };

}

#endif // _H_CS499R_OCTREE
