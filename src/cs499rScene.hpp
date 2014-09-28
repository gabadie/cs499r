
#ifndef _H_CS499R_SCENE
#define _H_CS499R_SCENE

#include <map>
#include <vector>
#include <string>
#include "cs499rCommonStruct.hpp"
#include "cs499rSceneMesh.hpp"
#include "cs499rSceneMeshInstance.hpp"


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

        /*
         * Adds stuff into the scene
         */
        SceneMesh *
        addMesh(std::string const & meshName, Mesh const & mesh)
        {
            auto sceneMesh = new SceneMesh(this, meshName, mesh);

            mObjectsMap.meshes.insert({meshName, sceneMesh});

            return sceneMesh;
        }

        SceneMeshInstance *
        addMeshInstance(std::string const & meshInstanceName, SceneMesh * sceneMesh)
        {
            auto sceneMeshInstance = new SceneMeshInstance(this, meshInstanceName, sceneMesh);

            mObjectsMap.meshInstances.insert({meshInstanceName, sceneMeshInstance});

            return sceneMeshInstance;
        }

        /*
         * Finds stuff in the scene
         */
        SceneMesh *
        findMesh(std::string const & meshName)
        {
            auto result = mObjectsMap.meshes.find(meshName);

            CS499R_ASSERT(result != mObjectsMap.meshes.end());

            return result->second;
        }

        SceneMeshInstance *
        findMeshInstance(std::string const & meshName)
        {
            auto result = mObjectsMap.meshInstances.find(meshName);

            CS499R_ASSERT(result != mObjectsMap.meshInstances.end());

            return result->second;
        }


        // --------------------------------------------------------------------- IDLE

        Scene();
        ~Scene();


    private:
        // --------------------------------------------------------------------- STRUCTS

        template <typename T>
        using Map = std::map<std::string, T *>;


        // --------------------------------------------------------------------- MEMBERS

        // all scene's triangles
        std::vector<common_triangle_t> mTriangles;

        /*
         * scene's objects' maps
         *
         * Notes: they shall be sorted by order of destruction
         */
        struct
        {
            // all scene's mesh instances
            Map<SceneMeshInstance> meshInstances;

            // all scene's meshes
            Map<SceneMesh> meshes;
        } mObjectsMap;


        // --------------------------------------------------------------------- METHODES

        static
        void
        destroyMap(Map<SceneObject> & map);


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class RenderState;
        friend class SceneBuffer;

    };

}

#endif // _H_CS499R_SCENE
