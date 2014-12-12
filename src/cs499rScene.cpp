
#include "cs499rScene.hpp"

namespace CS499R
{

    void
    Scene::computeBoundingBox(float32x3_t * outLowerBound, float32x3_t * outUpperBound) const
    {
        CS499R_ASSERT(mObjectsMap.meshInstances.size() != 0);

        *outLowerBound = +INFINITY;
        *outUpperBound = -INFINITY;

        for (auto it : mObjectsMap.meshInstances)
        {
            auto const meshInstance = it.second;

            float32x3_t meshInstanceLowerBound;
            float32x3_t meshInstanceUpperBound;

            meshInstance->computeBoundingBox(
                &meshInstanceLowerBound,
                &meshInstanceUpperBound
            );

            *outLowerBound = min(*outLowerBound, meshInstanceLowerBound);
            *outUpperBound = max(*outUpperBound, meshInstanceUpperBound);
        }
    }

    Scene::Scene()
    {

    }

    Scene::~Scene()
    {
        auto map = reinterpret_cast<Map<SceneObject> *>(&mObjectsMap);

        for (size_t i = 0; i < sizeof(mObjectsMap) / sizeof(map[0]); i++)
        {
            destroyMap(map[i]);
        }
    }

    void
    Scene::destroyMap(Map<SceneObject> & map)
    {
        for (auto it : map)
        {
            delete it.second;
        }

        map.clear();
    }


}
