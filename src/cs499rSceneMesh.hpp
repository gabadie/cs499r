
#ifndef _H_CS499R_SCENEMESH
#define _H_CS499R_SCENEMESH

#include "cs499rMesh.hpp"
#include "cs499rCommonStruct.hpp"
#include "cs499rSceneObject.hpp"


namespace CS499R
{

    /*
     * Define a mesh used in a scene
     */
    class SceneMesh : public SceneObject
    {
    public:
        // --------------------------------------------------------------------- OPERATORS

        SceneMesh &
        operator = (SceneMesh const &) = delete;


        // --------------------------------------------------------------------- IDLE

        SceneMesh(
            Scene * const scene,
            std::string const & objectName,
            Mesh const & mesh
        );
        SceneMesh(SceneMesh const &) = delete;
        ~SceneMesh();


    private:
        // --------------------------------------------------------------------- STRUCTS

        /*
         * Scene mesh's primitives are ready to be copied into GPU scene's
         * buffers.
         */
        using Primitive = common_primitive_t;


        // --------------------------------------------------------------------- MEMBERS

        // the number of primitives
        size_t mPrimitiveCount;

        // the primitive array
        Primitive * mPrimitiveArray;


        // --------------------------------------------------------------------- METHODES

        /*
         * Builds the scene mesh from a given raw mesh
         */
        void
        buildFromMesh(Mesh const & mesh);


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class SceneBuffer;

    };

}

#endif // _H_CS499R_SCENEMESH
