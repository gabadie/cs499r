
#ifndef _H_CS499R_SCENE
#define _H_CS499R_SCENE

#include <vector>
#include "cs499rCommonStruct.hpp"


namespace CS499R
{

    /*
     * The scene old all the scene information on the CPU side
     */
    class Scene
    {
    public:
        // --------------------------------------------------------------------- METHODES

        /*
         * Adds a triangle into the scene
         */
        void
        addTriangle(float32x3_t v0, float32x3_t v1, float32x3_t v2, float32x3_t diffuse, float32x3_t emit)
        {
            size_t i = mTriangles.size();

            mTriangles.resize(i + 1);
            mTriangles[i].v0 = v0;
            mTriangles[i].v1 = v1;
            mTriangles[i].v2 = v2;
            mTriangles[i].diffuseColor = diffuse;
            mTriangles[i].emitColor = emit;
        }


        // --------------------------------------------------------------------- IDLE

        Scene();
        ~Scene();


    private:
        // --------------------------------------------------------------------- MEMBERS

        // all scene's triangles
        std::vector<common_triangle_t> mTriangles;


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class RenderState;
        friend class SceneBuffer;


    };

}

#endif // _H_CS499R_SCENE
