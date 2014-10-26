
#ifndef _H_CS499R_OCTREENODE
#define _H_CS499R_OCTREENODE

#include <list>

#include "cs499rCommonStruct.hpp"


namespace CS499R
{

    /*
     * An octree node
     */
    class OctreeNode
    {
    private:
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
         * Exports to common_mesh_octree_node_t
         */
        void
        exportToCommonOctreeNode(
            size_t * outPrimNewIds,
            common_mesh_octree_node_t * outOctreeCommonNodes,
            size_t cursors[2]
        ) const;

        /*
         * Returns if this is a leaf node (no children)
         */
        bool
        isLeaf() const;

        /*
         * Returns the total number of children + himself
         */
        size_t
        nodeCount() const;

        /*
         * Optimizes node's children
         */
        size_t
        optimize();


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


        // --------------------------------------------------------------------- MEMBERS

        // children octree nodes (x + 2 * y + 4 * z)
        OctreeNode * mChildren[8];

        // the node's center in the mesh's basis
        float32x3_t const mCenter;

        // the node's logarithmic size
        int32_t const mSizeLog;

        // primitive id list in that node
        std::list<size_t> mPrimitiveIds;


        // --------------------------------------------------------------------- FRIENDSHIPS

        friend class Octree;

    };

}

#endif // _H_CS499R_OCTREENODE
