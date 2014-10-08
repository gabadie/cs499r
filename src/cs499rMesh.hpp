
#ifndef _H_CS499R_MESH
#define _H_CS499R_MESH

#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * Meshs old all information about the 3D objects
     */
    class Mesh
    {
    public:
        // --------------------------------------------------------------------- METHODES

        /*
         * Returns the mesh's bounding box corners
         */
        void
        computeBoundingBox(float32x3_t * outLowerBound, float32x3_t * outUpperBound) const;



        // --------------------------------------------------------------------- CONSTANTS

        // number of vertices per primitives
        static size_t const kPrimitiveVertexCount = 3;


        // --------------------------------------------------------------------- IDLE

        Mesh();
        Mesh(char const * filePath);
        Mesh(size_t primitiveCount, float32x3_t const * primitiveArray);
        ~Mesh();


    private:
        // --------------------------------------------------------------------- STRUCTS

        /*
         * Mesh's primitive struct
         */
        struct Primitive
        {
            float32x3_t vertex[3];
        };


        // --------------------------------------------------------------------- MEMBERS

        // the number of primitives
        size_t mPrimitiveCount;

        // the primitive array
        Primitive * mPrimitiveArray;


        // --------------------------------------------------------------------- METHODES

        /*
         * Loads an OBJ file
         */
        void
        loadObjFile(char const * filePath);


        // --------------------------------------------------------------------- FRIENDSHIP
        friend class SceneMesh;

    };

}


#endif // _H_CS499R_MESH
