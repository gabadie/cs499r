
#include "cs499rMesh.hpp"


namespace CS499R
{

    float32x3_t
    Mesh::lowerBoundCoordinate() const
    {
        float32x3_t lowerBound (INFINITY);

        for (size_t primId = 0; primId < mPrimitiveCount; primId++)
        {
            for (size_t vertexId = 0; vertexId < kPrimitiveVertexCount; vertexId++)
            {
                lowerBound = min(lowerBound, mPrimitiveArray[primId].vertex[vertexId]);
            }
        }

        return lowerBound;
    }

    Mesh::Mesh()
    {
        mPrimitiveCount = 0;
    }

    Mesh::Mesh(size_t vertexCount, float32x3_t const * vertexArray)
    {
        CS499R_ASSERT(vertexCount % kPrimitiveVertexCount == 0);
        CS499R_ASSERT(vertexCount != 0);

        mPrimitiveCount = vertexCount / kPrimitiveVertexCount;
        mPrimitiveArray = new Mesh::Primitive[mPrimitiveCount];

        for (size_t primId = 0; primId < mPrimitiveCount; primId++)
        {
            for (size_t vertexId = 0; vertexId < kPrimitiveVertexCount; vertexId++)
            {
                mPrimitiveArray[primId].vertex[vertexId] =
                    vertexArray[primId * kPrimitiveVertexCount + vertexId];
            }
        }
    }

    Mesh::~Mesh()
    {
        if (mPrimitiveCount)
        {
            delete [] mPrimitiveArray;
        }
    }

}
