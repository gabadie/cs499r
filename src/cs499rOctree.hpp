
#ifndef _H_CS499R_OCTREE
#define _H_CS499R_OCTREE

#include <list>

#include "cs499rCommonStruct.hpp"


namespace CS499R
{

    /*
     * Octree recursively subdivides space in 8 boxes, each containing a list of
     * primitive indexes (could be index for a mesh or a mesh's triangle).
     */
    class Octree
    {
    public:
        // --------------------------------------------------------------------- METHODS

        /*
         * Inserts a primitive id at the correct octree node with the given
         * geometric information parameters
         */
        void
        insert(size_t primId, float32x3_t primCenter, float32_t primBiggestDimSize);

        /*
         * Optimizes the octree
         */
        size_t
        optimize();

        /*
         * Exports the octree to common nodes, and returns the primitives'
         * new ids
         */
        void
        exportToCommonOctreeNodeArray(
            size_t * outPrimNewIds,
            common_mesh_octree_node_t * outOctreeCommonNodes
        ) const;

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
        nodeCount() const;


        // --------------------------------------------------------------------- IDLE

        Octree(
            float32x3_t const & lowerBound,
            float32x3_t const & upperBound
        );
        ~Octree();


    private:
        // --------------------------------------------------------------------- METHODS


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
