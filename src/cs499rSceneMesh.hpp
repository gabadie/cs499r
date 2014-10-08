
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


    private:
        // --------------------------------------------------------------------- IDLE

        SceneMesh(
            Scene * const scene,
            std::string const & objectName,
            Mesh const & mesh
        );
        SceneMesh(SceneMesh const &) = delete;
        ~SceneMesh();


        // --------------------------------------------------------------------- STRUCTS

        /*
         * Scene mesh's primitives are ready to be copied into GPU scene's
         * buffer
         */
        using Primitive = common_primitive_t;


        // --------------------------------------------------------------------- MEMBERS

        // the number of primitives
        size_t mPrimitiveCount;

        // the primitive array
        Primitive * mPrimitiveArray;

        // the number of octree's nodes
        size_t mOctreeNodeCount;

        // the octree's node array
        common_mesh_octree_node_t * mOctreeNodeArray;

        // the mesh's center pos within the octree basis
        float32x3_t mCenterPosition;


        // --------------------------------------------------------------------- METHODES

        /*
         * Builds the scene mesh from a given raw mesh
         */
        void
        buildFromMesh(Mesh const & mesh);


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class Scene;
        friend class SceneBuffer;

    };

}

#endif // _H_CS499R_SCENEMESH
