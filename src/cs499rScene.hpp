
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
        addTriangle(float3 v0, float3 v1, float3 v2, float3 diffuse, float3 emit)
        {
            size_t i = mTriangles.size();

            mTriangles.resize(i + 1);
            mTriangles[i].vertex[0] = v0;
            mTriangles[i].vertex[1] = v1;
            mTriangles[i].vertex[2] = v2;
            mTriangles[i].diffuseColor = diffuse;
            mTriangles[i].emitColor = emit;
        }


        // --------------------------------------------------------------------- IDLE

        Scene();
        ~Scene();


    private:
        // --------------------------------------------------------------------- MEMBERS

        // all scene's triangles
        std::vector<triangle_t> mTriangles;


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class SceneBuffer;


    };

}

#endif // _H_CS499R_SCENE
