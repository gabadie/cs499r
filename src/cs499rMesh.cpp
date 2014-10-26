
#include <fstream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include "cs499rMesh.hpp"

namespace CS499R
{

    void
    Mesh::computeBoundingBox(float32x3_t * outLowerBound, float32x3_t * outUpperBound) const
    {
        float32x3_t lowerBound(INFINITY);
        float32x3_t upperBound(-INFINITY);

        for (size_t primId = 0; primId < mPrimitiveCount; primId++)
        {
            for (size_t vertexId = 0; vertexId < kPrimitiveVertexCount; vertexId++)
            {
                lowerBound = min(lowerBound, mPrimitiveArray[primId].vertex[vertexId]);
                upperBound = max(upperBound, mPrimitiveArray[primId].vertex[vertexId]);
            }
        }

        *outLowerBound = lowerBound;
        *outUpperBound = upperBound;
    }

    Mesh::Mesh()
    {
        mPrimitiveCount = 0;
    }

    Mesh::Mesh(char const * filePath)
    {
        loadObjFile(filePath);
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

    void
    Mesh::loadObjFile(char const * filePath)
    {
        std::ifstream infile(filePath);

        CS499R_ASSERT(infile.is_open());

        std::string line;
        std::list<float32x3_t> verticesList;
        std::list<uint32x3_t> faces;

        while (std::getline(infile, line))
        {
            std::istringstream iss(line);
            std::string instruction;

            iss >> instruction;

            if (instruction == "v")
            {
                float32x3_t vertex;

                iss >> vertex.x;
                iss >> vertex.y;
                CS499R_ASSERT(iss);
                iss >> vertex.z;

                verticesList.push_back(float32x3_t(vertex.x, vertex.y, vertex.z));
            }
            else if (instruction == "f")
            {
                uint32x4_t face;

                iss >> face.x;
                iss >> face.y;
                CS499R_ASSERT(iss);
                iss >> face.z;

                face = face - uint32_t(1);

                if (iss)
                {
                    iss >> face.w;

                    faces.push_back(uint32x3_t(face.z, face.w - 1, face.x));
                }

                faces.push_back(uint32x3_t(face.x, face.y, face.z));
            }
        }

        std::vector<float32x3_t> vertices(verticesList.begin(), verticesList.end());

        CS499R_ASSERT(faces.size() != 0);
        CS499R_ASSERT(vertices.size() != 0);

        mPrimitiveCount = faces.size();
        mPrimitiveArray = new Mesh::Primitive[mPrimitiveCount];

        auto it = mPrimitiveArray;

        for (auto face : faces)
        {
            CS499R_ASSERT(face.x < vertices.size());
            CS499R_ASSERT(face.y < vertices.size());
            CS499R_ASSERT(face.z < vertices.size());

            it->vertex[0] = vertices[face.x];
            it->vertex[1] = vertices[face.y];
            it->vertex[2] = vertices[face.z];

            auto n = cross(it->vertex[1] - it->vertex[0], it->vertex[2] - it->vertex[0]);

            if (dot(n, n) < 0.00000001f)
            {
                // we filter out too small triangle
                continue;
            }

            it++;
        }

        mPrimitiveCount = it - mPrimitiveArray;

        CS499R_ASSERT(mPrimitiveCount != 0);
    }

    void
    Mesh::Primitive::exportToCommonPrimitive(common_primitive_t * outCommonPrimitive, float32x3_t vertexOffset) const
    {
        float32x3_t const v0 = vertex[0] + vertexOffset;
        float32x3_t const e0 = vertex[1] - vertex[0];
        float32x3_t const e1 = vertex[2] - vertex[0];
        float32x3_t const normal = normalize(cross(e0, e1));

        float32_t const basisDot = dot(e0, e1);
        float32x2_t const invSquareLenght = 1.0f / float32x2_t(dot(e0, e0), dot(e1, e1));
        float32x2_t const invSquareLenghtBasisDot = basisDot * invSquareLenght;
        float32_t const invDet = 1.0f / (1.0f - invSquareLenghtBasisDot.x * invSquareLenghtBasisDot.y);
        float32x2_t const factorAIdot = invDet * invSquareLenght;

        outCommonPrimitive->v0 = float32x4_t(v0.x, v0.y, v0.z, normal.x);
        outCommonPrimitive->e0 = float32x4_t(e0.x, e0.y, e0.z, normal.y);
        outCommonPrimitive->e1 = float32x4_t(e1.x, e1.y, e1.z, normal.z);
        outCommonPrimitive->uvMatrix.x.x = factorAIdot.x;
        outCommonPrimitive->uvMatrix.y.x = - factorAIdot.y * invSquareLenghtBasisDot.x;
        outCommonPrimitive->uvMatrix.x.y = - factorAIdot.x * invSquareLenghtBasisDot.y;
        outCommonPrimitive->uvMatrix.y.y = factorAIdot.y;
    }

}
