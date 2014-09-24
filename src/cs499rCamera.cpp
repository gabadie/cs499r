
#include "cs499rCamera.hpp"


namespace CS499R
{

    Camera::Camera()
    {
        mPosition = float3 { -1.0f, -1.0f, -1.0f };
        mFocusPosition = float3 { 0.0f, 0.0f, 0.0f };
        mViewField = kPI * 0.33f;
        mShotDiagonalLength = 0.02;
    }

}
